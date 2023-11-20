#pragma once

#include "proptest/stateful/stateful_class.hpp"
#include "proptest/gen.hpp"
#include "proptest/Random.hpp"
#include "proptest/Shrinkable.hpp"
#include "proptest/api.hpp"
#include "proptest/PropertyContext.hpp"
#include "proptest/GenBase.hpp"
#include "proptest/util/std.hpp"
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>

namespace proptest {
namespace concurrent {
namespace alt {

using std::atomic;
using std::atomic_bool;
using std::atomic_int;
using std::mutex;
using std::thread;

using proptest::stateful::alt::Action;
using proptest::stateful::alt::actionListGenOf;
using proptest::stateful::alt::SimpleAction;
using proptest::stateful::alt::EmptyModel;

template <typename ActionType>
class PROPTEST_API Concurrency {
public:
    using ObjectType = typename ActionType::ObjectType;
    using ObjectTypeGen = GenFunction<ObjectType>;
    using ModelType = typename ActionType::ModelType;
    using ModelTypeGen = function<ModelType(ObjectType&)>;
    using ActionList = list<shared_ptr<ActionType>>;
    using ActionListGen = GenFunction<ActionList>;

    static constexpr uint32_t defaultNumRuns = 200;

    Concurrency(shared_ptr<ObjectTypeGen> _initialGenPtr, shared_ptr<ActionListGen> _actionListGenPtr)
        : initialGenPtr(_initialGenPtr),
          actionListGenPtr(_actionListGenPtr),
          seed(util::getGlobalSeed()),
          numRuns(defaultNumRuns),
          maxDurationMs(0)
    {
    }

    Concurrency(shared_ptr<ObjectTypeGen> _initialGenPtr, shared_ptr<ModelTypeGen> _modelFactoryPtr,
                shared_ptr<ActionListGen> _actionListGenPtr)
        : initialGenPtr(_initialGenPtr),
          modelFactoryPtr(_modelFactoryPtr),
          actionListGenPtr(_actionListGenPtr),
          seed(util::getGlobalSeed()),
          numRuns(defaultNumRuns),
          maxDurationMs(0)
    {
    }

    bool go();
    bool invoke(Random& rand);
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

    Concurrency& setMaxDurationMs(uint32_t durationMs)
    {
        maxDurationMs = durationMs;
        return *this;
    }

    Concurrency& setOnStartup(function<void()> onStartup) {
        onStartupPtr = util::make_shared<function<void()>>(onStartup);
        return *this;
    }

    Concurrency& setOnCleanup(function<void()> onCleanup) {
        onCleanupPtr = util::make_shared<function<void()>>(onCleanup);
        return *this;
    }

    template <typename M = ModelType>
        requires(!is_same_v<M, EmptyModel>)
    Concurrency& setPostCheck(function<void(ObjectType&, ModelType&)> postCheck)  {
        postCheckPtr = util::make_shared<function<void(ObjectType&, ModelType&)>>(postCheck);
        return *this;
    }

    template <typename M = ModelType>
        requires(is_same_v<M, EmptyModel>)
    Concurrency&  setPostCheck(function<void(ObjectType&)> postCheck)  {
        function<void(ObjectType&,ModelType&)>  fullPostCheck = [postCheck](ObjectType& sys, ModelType&) { postCheck(sys); };
        postCheckPtr = util::make_shared(fullPostCheck);
        return *this;
    }

private:
    shared_ptr<ObjectTypeGen> initialGenPtr;
    shared_ptr<ModelTypeGen> modelFactoryPtr;
    shared_ptr<ActionListGen> actionListGenPtr;
    shared_ptr<function<void()>> onStartupPtr;
    shared_ptr<function<void()>> onCleanupPtr;
    shared_ptr<function<void(ObjectType&, ModelType&)>> postCheckPtr;
    uint64_t seed;
    uint32_t numRuns;
    uint32_t maxDurationMs;
};

template <typename ActionType>
bool Concurrency<ActionType>::go()
{
    Random rand(seed);
    Random savedRand(seed);
    cout << "random seed: " << seed << endl;
    PropertyContext ctx;
    size_t i = 0;
    auto startedTime = steady_clock::now();
    try {
        for (; i < numRuns; i++) {
            if(maxDurationMs != 0) {
                auto currentTime = steady_clock::now();
                if(duration_cast<util::milliseconds>(currentTime - startedTime).count() > maxDurationMs)
                {
                    cout << "Timed out after " << duration_cast<util::milliseconds>(currentTime - startedTime).count() << "ms , passed " << i << " tests" << endl;
                    return true;
                }
            }
            bool pass = true;
            do {
                pass = true;
                try {
                    savedRand = rand;
                    if(onStartupPtr)
                        (*onStartupPtr)();
                    if(invoke(rand) && onCleanupPtr)
                        (*onCleanupPtr)();
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
        cerr << "Falsifiable, after " << (i + 1) << " tests: " << e.what() << " (" << e.filename << ":" << e.lineno
             << ")" << endl;

        // shrink
        handleShrink(savedRand);
        return false;
    } catch (const exception& e) {
        cerr << "Falsifiable, after " << (i + 1) << " tests - exception occurred: " << e.what() << endl;
        cerr << "    seed: " << seed << endl;
        // shrink
        handleShrink(savedRand);
        return false;
    }

    cout << "OK, passed " << numRuns << " tests" << endl;

    return true;
}

template <typename ActionType>
struct RearRunner
{
    using ObjectType = typename ActionType::ObjectType;
    using ModelType = typename ActionType::ModelType;
    using ActionList = list<shared_ptr<ActionType>>;

    RearRunner(int _n, ObjectType& _obj, ModelType& _model, ActionList& _actions, atomic_bool& _thread_ready,
               atomic_bool& _sync_ready, vector<int>& _log, atomic_int& _counter)
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
            // cout << "rear2" << endl;
            log[counter++] = n;
        }
    }

    int n;
    ObjectType& obj;
    ModelType& model;
    ActionList& actions;
    atomic_bool& thread_ready;
    atomic_bool& sync_ready;
    vector<int>& log;
    atomic_int& counter;
};

template <typename ActionType>
bool Concurrency<ActionType>::invoke(Random& rand)
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
    thread spawner([&]() {
        atomic_bool thread1_ready(false);
        atomic_bool thread2_ready(false);
        atomic_bool sync_ready(false);
        atomic<int> counter{0};
        vector<int> log;
        log.resize(5000);

        thread rearRunner1(RearRunner<ActionType>(1, obj, model, rear1, thread1_ready, sync_ready, log, counter));
        thread rearRunner2(RearRunner<ActionType>(2, obj, model, rear2, thread2_ready, sync_ready, log, counter));
        while (!thread1_ready) {}
        while (!thread2_ready) {}

        sync_ready = true;

        rearRunner1.join();
        rearRunner2.join();

        cout << "count: " << counter << ", order: ";
        for (int i = 0; i < counter; i++) {
            cout << log[i];
        }
        cout << endl;
    });

    spawner.join();

    if(postCheckPtr)
            (*postCheckPtr)(obj, model);

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
    using ActionList = list<shared_ptr<ActionType>>;
    auto initialGenPtr = util::make_shared<ObjectTypeGen>(util::forward<InitialGen>(initialGen));
    auto actionListGenPtr = util::make_shared<GenFunction<ActionList>>(util::forward<ActionListGen>(actionListGen));
    return Concurrency<ActionType>(initialGenPtr, actionListGenPtr);
}

template <typename ActionType, typename InitialGen, typename ModelFactory, typename ActionListGen>
decltype(auto) concurrency(InitialGen&& initialGen, ModelFactory&& modelFactory, ActionListGen&& actionListGen)
{
    using ObjectType = typename ActionType::ObjectType;
    using ModelType = typename ActionType::ModelType;
    using ModelFactoryFunction = function<ModelType(ObjectType&)>;
    using ObjectTypeGen = GenFunction<ObjectType>;
    using ActionList = list<shared_ptr<ActionType>>;

    shared_ptr<ModelFactoryFunction> modelFactoryPtr =
        util::make_shared<ModelFactoryFunction>(util::forward<ModelFactory>(modelFactory));

    auto initialGenPtr = util::make_shared<ObjectTypeGen>(util::forward<InitialGen>(initialGen));
    auto actionListGenPtr = util::make_shared<GenFunction<ActionList>>(util::forward<ActionListGen>(actionListGen));
    return Concurrency<ActionType>(initialGenPtr, modelFactoryPtr, actionListGenPtr);
}

}  // namespace alt
}  // namespace concurrent

}  // namespace proptest
