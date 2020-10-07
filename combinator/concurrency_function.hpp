#pragma once

#include "stateful_function.hpp"
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

using stateful::Action;
using stateful::actionListGenOf;
using stateful::EmptyModel;
using stateful::SimpleAction;

template <typename ObjectType, typename ModelType>
class PROPTEST_API Concurrency {
public:
    using Action = std::function<bool(ObjectType&, ModelType&)>;
    using ObjectTypeGen = GenFunction<ObjectType>;
    using ModelTypeGen = typename std::function<ModelType(ObjectType&)>;
    using ActionList = std::list<Action>;
    using ActionListGen = GenFunction<ActionList>;

    static constexpr uint32_t defaultNumRuns = 200;

    Concurrency(std::shared_ptr<ObjectTypeGen> initialGenPtr, std::shared_ptr<ActionListGen> actionsGenPtr)
        : initialGenPtr(initialGenPtr), actionsGenPtr(actionsGenPtr), seed(getCurrentTime()), numRuns(defaultNumRuns)
    {
    }

    Concurrency(std::shared_ptr<ObjectTypeGen> initialGenPtr, std::shared_ptr<ModelTypeGen> modelFactoryPtr,
                std::shared_ptr<ActionListGen> actionsGenPtr)
        : initialGenPtr(initialGenPtr),
          modelFactoryPtr(modelFactoryPtr),
          actionsGenPtr(actionsGenPtr),
          seed(getCurrentTime()),
          numRuns(defaultNumRuns)
    {
    }

    bool check();
    bool check(std::function<void(ObjectType&, ModelType&)> postCheck);
    bool check(std::function<void(ObjectType&)> postCheck);
    bool invoke(Random& rand, std::function<void(ObjectType&, ModelType&)> postCheck);
    void handleShrink(Random& savedRand, const PropertyFailedBase& e);

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
    std::shared_ptr<ActionListGen> actionsGenPtr;
    uint64_t seed;
    int numRuns;
};

template <typename ObjectType, typename ModelType>
bool Concurrency<ObjectType, ModelType>::check()
{
    static auto emptyPostCheck = +[](ObjectType&, ModelType&) {};
    return check(emptyPostCheck);
}

template <typename ObjectType, typename ModelType>
bool Concurrency<ObjectType, ModelType>::check(std::function<void(ObjectType&)> postCheck)
{
    static auto fullPostCheck = [postCheck](ObjectType& sys, ModelType&) { postCheck(sys); };
    return check(fullPostCheck);
}

template <typename ObjectType, typename ModelType>
bool Concurrency<ObjectType, ModelType>::check(std::function<void(ObjectType&, ModelType&)> postCheck)
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
        handleShrink(savedRand, e);
        return false;
    } catch (const std::exception& e) {
        std::cerr << "Falsifiable, after " << (i + 1) << " tests - std::exception occurred: " << e.what() << std::endl;
        std::cerr << "    seed: " << seed << std::endl;
        return false;
    }

    std::cout << "OK, passed " << numRuns << " tests" << std::endl;

    return true;
}

template <typename ObjectType, typename ModelType>
struct RearRunner
{
    using Action = std::function<bool(ObjectType&, ModelType&)>;
    using ActionList = std::list<Action>;

    RearRunner(int n, ObjectType& obj, ModelType& model, ActionList& actions, std::atomic_bool& thread_ready,
               std::atomic_bool& sync_ready, std::vector<int>& log, std::atomic_int& counter)
        : n(n),
          obj(obj),
          model(model),
          actions(actions),
          thread_ready(thread_ready),
          sync_ready(sync_ready),
          log(log),
          counter(counter)
    {
    }

    void operator()()
    {
        thread_ready = true;
        while (!sync_ready) {}

        for (auto action : actions) {
            if (!action(obj, model))
                continue;
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

template <typename ObjectType, typename ModelType>
bool Concurrency<ObjectType, ModelType>::invoke(Random& rand, std::function<void(ObjectType&, ModelType&)> postCheck)
{
    Shrinkable<ObjectType> initialShr = (*initialGenPtr)(rand);
    ObjectType& obj = initialShr.getRef();
    ModelType model = modelFactoryPtr ? (*modelFactoryPtr)(obj) : ModelType();
    Shrinkable<ActionList> frontShr = (*actionsGenPtr)(rand);
    Shrinkable<ActionList> rear1Shr = (*actionsGenPtr)(rand);
    Shrinkable<ActionList> rear2Shr = (*actionsGenPtr)(rand);
    ActionList& front = frontShr.getRef();
    ActionList& rear1 = rear1Shr.getRef();
    ActionList& rear2 = rear2Shr.getRef();

    // front
    for (auto action : front) {
        action(obj, model);
    }

    // rear
    std::thread spawner([&]() {
        std::atomic_bool thread1_ready(false);
        std::atomic_bool thread2_ready(false);
        std::atomic_bool sync_ready(false);
        std::atomic<int> counter{0};
        std::vector<int> log;
        log.resize(5000);

        std::thread rearRunner1(
            RearRunner<ObjectType, ModelType>(1, obj, model, rear1, thread1_ready, sync_ready, log, counter));
        std::thread rearRunner2(
            RearRunner<ObjectType, ModelType>(2, obj, model, rear2, thread2_ready, sync_ready, log, counter));
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

template <typename ObjectType, typename ModelType>
void Concurrency<ObjectType, ModelType>::handleShrink(Random&, const PropertyFailedBase&)
{
    // TODO
}

/* without model */
template <typename ObjectType, typename InitialGen>
decltype(auto) concurrency(InitialGen&& initialGen, stateful::ActionListGen<ObjectType, EmptyModel>& actionsGen)
{
    using ObjectTypeGen = GenFunction<ObjectType>;
    using Action = std::function<bool(ObjectType&, EmptyModel&)>;
    using ActionList = std::list<Action>;
    auto initialGenPtr = std::make_shared<ObjectTypeGen>(std::forward<InitialGen>(initialGen));
    auto actionsGenPtr = std::make_shared<GenFunction<ActionList>>(actionsGen);
    return Concurrency<ObjectType, EmptyModel>(initialGenPtr, actionsGenPtr);
}

/* with model */
template <typename ObjectType, typename ModelType, typename InitialGen, typename ModelFactory, typename ActionsGen>
decltype(auto) concurrency(InitialGen&& initialGen, ModelFactory&& modelFactory, ActionsGen&& actionsGen)
{
    using ModelFactoryFunction = std::function<ModelType(ObjectType&)>;
    using ObjectTypeGen = GenFunction<ObjectType>;
    using Action = std::function<bool(ObjectType&, ModelType&)>;
    using ActionList = std::list<Action>;

    std::shared_ptr<ModelFactoryFunction> modelFactoryPtr =
        std::make_shared<ModelFactoryFunction>(std::forward<ModelFactory>(modelFactory));

    auto initialGenPtr = std::make_shared<ObjectTypeGen>(std::forward<InitialGen>(initialGen));
    auto actionsGenPtr = std::make_shared<GenFunction<ActionList>>(std::forward<ActionsGen>(actionsGen));
    return Concurrency<ObjectType, ModelType>(initialGenPtr, modelFactoryPtr, actionsGenPtr);
}

}  // namespace concurrent
}  // namespace proptest
