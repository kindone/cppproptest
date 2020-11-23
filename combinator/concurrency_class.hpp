#pragma once

#include "stateful_class.hpp"
#include "../gen.hpp"
#include "../Random.hpp"
#include "../Shrinkable.hpp"
#include "../api.hpp"
#include "../PropertyContext.hpp"
#include "../GenBase.hpp"
#include <memory>
#include <list>
#include <type_traits>
#include <iostream>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>

namespace proptest {
namespace concurrent {
namespace alt {

using proptest::stateful::alt::Action;
using proptest::stateful::alt::actionListGenOf;
using proptest::stateful::alt::SimpleAction;

template <typename ActionType>
class PROPTEST_API Concurrency {
public:
    using ObjectType = typename ActionType::ObjectType;
    using ObjectTypeGen = GenFunction<ObjectType>;
    using ModelType = typename ActionType::ModelType;
    using ModelTypeGen = typename std::function<ModelType(ObjectType&)>;
    using ActionList = std::list<std::shared_ptr<ActionType>>;
    using ActionListGen = GenFunction<ActionList>;

    static constexpr uint32_t defaultNumRuns = 200;

    Concurrency(std::shared_ptr<ObjectTypeGen> _initialGenPtr, std::shared_ptr<ActionListGen> _actionListGenPtr)
        : initialGenPtr(_initialGenPtr),
          actionListGenPtr(_actionListGenPtr),
          seed(getCurrentTime()),
          numRuns(defaultNumRuns)
    {
    }

    Concurrency(std::shared_ptr<ObjectTypeGen> _initialGenPtr, std::shared_ptr<ModelTypeGen> _modelFactoryPtr,
                std::shared_ptr<ActionListGen> _actionListGenPtr)
        : initialGenPtr(_initialGenPtr),
          modelFactoryPtr(_modelFactoryPtr),
          actionListGenPtr(_actionListGenPtr),
          seed(getCurrentTime()),
          numRuns(defaultNumRuns)
    {
    }

    bool go();
    bool go(std::function<void(ObjectType&, ModelType&)> postCheck);
    bool go(std::function<void(ObjectType&)> postCheck);
    bool invoke(Random& rand, std::function<void(ObjectType&, ModelType&)> postCheck);
    void handleShrink(Random& savedRand);

    Concurrency& setSeed(uint64_t s)
    {
        seed = s;
        return *this;
    }

    Concurrency& setNumRuns(uint32_t runs)
    {
        numRuns = runs;
        return *this;
    }

private:
    std::shared_ptr<ObjectTypeGen> initialGenPtr;
    std::shared_ptr<ModelTypeGen> modelFactoryPtr;
    std::shared_ptr<ActionListGen> actionListGenPtr;
    uint64_t seed;
    int numRuns;
};

template <typename ActionType>
bool Concurrency<ActionType>::go()
{
    static auto emptyPostCheck = +[](ObjectType&, ModelType&) {};
    return go(emptyPostCheck);
}

template <typename ActionType>
bool Concurrency<ActionType>::go(std::function<void(ObjectType&)> postCheck)
{
    auto fullPostCheck = [postCheck](ObjectType& sys, ModelType&) { postCheck(sys); };
    return go(fullPostCheck);
}

template <typename ActionType>
bool Concurrency<ActionType>::go(std::function<void(ObjectType&, ModelType&)> postCheck)
{
    Random rand(seed);
    Random savedRand(seed);
    std::cout << "random seed: " << seed << std::endl;
    int i = 0;
    try {
        for (; i < numRuns; i++) {
            bool pass = true;
            do {
                pass = true;
                try {
                    savedRand = rand;
                    invoke(rand, postCheck);
                    pass = true;
                } catch (const Success&) {
                    pass = true;
                } catch (const Discard&) {
                    // silently discard combination
                    pass = false;
                }
            } while (!pass);
        }
    } catch (const PropertyFailedBase& e) {
        std::cerr << "Falsifiable, after " << (i + 1) << " tests: " << e.what() << " (" << e.filename << ":" << e.lineno
                  << ")" << std::endl;

        // shrink
        handleShrink(savedRand);
        return false;
    } catch (const std::exception& e) {
        std::cerr << "Falsifiable, after " << (i + 1) << " tests - std::exception occurred: " << e.what() << std::endl;
        std::cerr << "    seed: " << seed << std::endl;
        // shrink
        handleShrink(savedRand);
        return false;
    }

    std::cout << "OK, passed " << numRuns << " tests" << std::endl;

    return true;
}

template <typename ActionType>
struct RearRunner
{
    using ObjectType = typename ActionType::ObjectType;
    using ModelType = typename ActionType::ModelType;
    using ActionList = std::list<std::shared_ptr<ActionType>>;

    RearRunner(int _n, ObjectType& _obj, ModelType& _model, ActionList& _actions, std::atomic_bool& _thread_ready,
               std::atomic_bool& _sync_ready, std::vector<int>& _log, std::atomic_int& _counter)
        : n(_n),
          obj(_obj),
          model(_model),
          actions(_actions),
          thread_ready(_thread_ready),
          sync_ready(_sync_ready),
          log(_log),
          counter(_counter)
    {
    }

    void operator()()
    {
        thread_ready = true;
        while (!sync_ready) {}

        for (auto action : actions) {
            if (!action->precondition(obj, model))
                continue;
            PROP_ASSERT(action->run(obj, model));
            // std::cout << "rear2" << std::endl;
            log[counter++] = n;
        }
    }

    int n;
    ObjectType& obj;
    ModelType& model;
    ActionList& actions;
    std::atomic_bool& thread_ready;
    std::atomic_bool& sync_ready;
    std::vector<int>& log;
    std::atomic_int& counter;
};

template <typename ActionType>
bool Concurrency<ActionType>::invoke(Random& rand, std::function<void(ObjectType&, ModelType&)> postCheck)
{
    Shrinkable<ObjectType> initialShr = (*initialGenPtr)(rand);
    ObjectType& obj = initialShr.getRef();
    ModelType model = modelFactoryPtr ? (*modelFactoryPtr)(obj) : ModelType();
    Shrinkable<ActionList> frontShr = (*actionListGenPtr)(rand);
    Shrinkable<ActionList> rear1Shr = (*actionListGenPtr)(rand);
    Shrinkable<ActionList> rear2Shr = (*actionListGenPtr)(rand);
    ActionList& front = frontShr.getRef();
    ActionList& rear1 = rear1Shr.getRef();
    ActionList& rear2 = rear2Shr.getRef();

    // front
    for (auto action : front) {
        if (action->precondition(obj, model))
            PROP_ASSERT(action->run(obj, model));
    }

    // rear
    std::thread spawner([&]() {
        std::atomic_bool thread1_ready(false);
        std::atomic_bool thread2_ready(false);
        std::atomic_bool sync_ready(false);
        std::atomic<int> counter{0};
        std::vector<int> log;
        log.resize(5000);

        std::thread rearRunner1(RearRunner<ActionType>(1, obj, model, rear1, thread1_ready, sync_ready, log, counter));
        std::thread rearRunner2(RearRunner<ActionType>(2, obj, model, rear2, thread2_ready, sync_ready, log, counter));
        while (!thread1_ready) {}
        while (!thread2_ready) {}

        sync_ready = true;

        rearRunner1.join();
        rearRunner2.join();

        std::cout << "count: " << counter << ", order: ";
        for (int i = 0; i < counter; i++) {
            std::cout << log[i];
        }
        std::cout << std::endl;
    });

    spawner.join();
    postCheck(obj, model);

    return true;
}

template <typename ActionType>
void Concurrency<ActionType>::handleShrink(Random&)
{
}

template <typename ActionType, typename InitialGen, typename ActionListGen>
decltype(auto) concurrency(InitialGen&& initialGen, ActionListGen&& actionListGen)
{
    using ObjectType = typename ActionType::ObjectType;
    using ObjectTypeGen = GenFunction<ObjectType>;
    using ActionList = std::list<std::shared_ptr<ActionType>>;
    auto initialGenPtr = std::make_shared<ObjectTypeGen>(std::forward<InitialGen>(initialGen));
    auto actionListGenPtr = std::make_shared<GenFunction<ActionList>>(std::forward<ActionListGen>(actionListGen));
    return Concurrency<ActionType>(initialGenPtr, actionListGenPtr);
}

template <typename ActionType, typename InitialGen, typename ModelFactory, typename ActionListGen>
decltype(auto) concurrency(InitialGen&& initialGen, ModelFactory&& modelFactory, ActionListGen&& actionListGen)
{
    using ObjectType = typename ActionType::ObjectType;
    using ModelType = typename ActionType::ModelType;
    using ModelFactoryFunction = std::function<ModelType(ObjectType&)>;
    using ObjectTypeGen = GenFunction<ObjectType>;
    using ActionList = std::list<std::shared_ptr<ActionType>>;

    std::shared_ptr<ModelFactoryFunction> modelFactoryPtr =
        std::make_shared<ModelFactoryFunction>(std::forward<ModelFactory>(modelFactory));

    auto initialGenPtr = std::make_shared<ObjectTypeGen>(std::forward<InitialGen>(initialGen));
    auto actionListGenPtr = std::make_shared<GenFunction<ActionList>>(std::forward<ActionListGen>(actionListGen));
    return Concurrency<ActionType>(initialGenPtr, modelFactoryPtr, actionListGenPtr);
}

}  // namespace alt
}  // namespace concurrent

}  // namespace proptest
