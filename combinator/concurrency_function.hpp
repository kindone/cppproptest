#pragma once

#include "stateful_function.hpp"
#include "../gen.hpp"
#include "../Random.hpp"
#include "../Shrinkable.hpp"
#include "../api.hpp"
#include "../PropertyContext.hpp"
#include "../GenBase.hpp"
#include "../util/std.hpp"
#include <thread>
#include <atomic>

namespace proptest {

namespace concurrent {

using stateful::Action;
using stateful::EmptyModel;
using stateful::SimpleAction;
using stateful::SimpleActionGen;
using stateful::ActionGen;

using std::atomic;
using std::atomic_bool;
using std::atomic_int;
using std::thread;

template <typename ObjectType, typename ModelType>
class PROPTEST_API Concurrency {
public:
    using ActionType = Action<ObjectType,ModelType>;
    using ObjectTypeGen = GenFunction<ObjectType>;
    using ModelTypeGen = function<ModelType(ObjectType&)>;
    using ActionList = list<ActionType>;
    using ActionGen = GenFunction<ActionType>;

    static constexpr uint32_t defaultNumRuns = 200;
    static constexpr int defaultNumThreads = 2;

    Concurrency(shared_ptr<ObjectTypeGen> _initialGenPtr, shared_ptr<ActionGen> _actionGenPtr)
        : initialGenPtr(_initialGenPtr),
          actionGenPtr(_actionGenPtr),
          seed(getCurrentTime()),
          numRuns(defaultNumRuns),
          numThreads(defaultNumThreads)
    {
    }

    Concurrency(shared_ptr<ObjectTypeGen> _initialGenPtr, shared_ptr<ModelTypeGen> _modelFactoryPtr,
                shared_ptr<ActionGen> _actionGenPtr)
        : initialGenPtr(_initialGenPtr),
          modelFactoryPtr(_modelFactoryPtr),
          actionGenPtr(_actionGenPtr),
          seed(getCurrentTime()),
          numRuns(defaultNumRuns),
          numThreads(defaultNumThreads)
    {
    }

    Concurrency& setOnStartup(function<void()> onStartup) {
        onStartupPtr = util::make_shared<function<void()>>(onStartup);
        return *this;
    }

    Concurrency& setOnCleanup(function<void()> onCleanup) {
        onCleanupPtr = util::make_shared<function<void()>>(onCleanup);
        return *this;
    }

    Concurrency& setPostCheck(function<void(ObjectType&, ModelType&)> postCheck)  {
        postCheckPtr = util::make_shared<function<void(ObjectType&, ModelType&)>>(postCheck);
        return *this;
    }

    Concurrency& setPostCheck(function<void(ObjectType&)> postCheck)  {
        function<void(ObjectType&,ModelType&)>  fullPostCheck = [postCheck](ObjectType& sys, ModelType&) { postCheck(sys); };
        postCheckPtr = util::make_shared(fullPostCheck);
        return *this;
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

    Concurrency& setMaxConcurrency(uint32_t numThr)
    {
        numThreads = numThr;
        return *this;
    }

private:
    shared_ptr<ObjectTypeGen> initialGenPtr;
    shared_ptr<ModelTypeGen> modelFactoryPtr;
    shared_ptr<ActionGen> actionGenPtr;
    shared_ptr<function<void()>> onStartupPtr;
    shared_ptr<function<void()>> onCleanupPtr;
    shared_ptr<function<void(ObjectType&, ModelType&)>> postCheckPtr;
    uint64_t seed;
    int numRuns;
    int numThreads;
};

template <typename ObjectType, typename ModelType>
bool Concurrency<ObjectType, ModelType>::go()
{
    Random rand(seed);
    Random savedRand(seed);
    cout << "random seed: " << seed << endl;
    PropertyContext ctx;
    int i = 0;
    try {
        for (; i < numRuns; i++) {
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
        cerr << "    seed: " << seed << endl;
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

template <typename ObjectType, typename ModelType>
struct RearRunner
{
    using ActionType = Action<ObjectType,ModelType>;
    using ActionList = list<ActionType>;

    RearRunner(int _num, ObjectType& _obj, ModelType& _model, ActionList& _actions, atomic_bool& _thread_ready,
               atomic_bool& _sync_ready, vector<int>& _log, atomic_int& _counter)
        : num(_num),
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
            log[counter++] = num; // start
            action(obj, model);
            // cout << "rear2" << endl;
            log[counter++] = num; // end
        }
    }

    int num;
    ObjectType& obj;
    ModelType model;
    ActionList& actions;
    atomic_bool& thread_ready;
    atomic_bool& sync_ready;
    vector<int>& log;
    atomic_int& counter;
};

template <typename ObjectType, typename ModelType>
bool Concurrency<ObjectType, ModelType>::invoke(Random& rand)
{
    constexpr int UNINITIALIZED_THREAD_ID = -2;
    constexpr int FRONT_THREAD_ID = -1;
    Shrinkable<ObjectType> initialShr = (*initialGenPtr)(rand);

    auto actionListGen = Arbi<list<Action<ObjectType,ModelType>>>(*actionGenPtr);
    Shrinkable<ActionList> frontShr = actionListGen(rand);
    vector<Shrinkable<ActionList>> rearShrs;
    for (int i = 0; i < numThreads; i++) {
        rearShrs.push_back(actionListGen(rand));
    }

    ObjectType& obj = initialShr.getRef();
    ModelType model = modelFactoryPtr ? (*modelFactoryPtr)(obj) : ModelType();
    ActionList& front = frontShr.getRef();

    atomic<int> counter{0};
    vector<int> log;

    // run front
    for (auto action : front) {
        action(obj, model);
        log.push_back(FRONT_THREAD_ID);
        counter++;
    }

    // serial execution
    if (numThreads <= 1) {
        if(postCheckPtr)
            (*postCheckPtr)(obj, model);
        return true;
    }

    // run rear
    thread spawner([&]() {
        atomic_bool sync_ready(false);
        vector<shared_ptr<atomic_bool>> thread_ready;
        vector<thread> rearRunners;
        vector<ActionList> rears;

        for (int i = 0; i < numThreads; i++) {
            thread_ready.emplace_back(new atomic_bool(false));
            auto& rear = rearShrs[i].getRef();
            rears.emplace_back(rear);

            // logging start/end of action
            for(size_t j = 0; j < rear.size() * 2; j++)
                log.push_back(UNINITIALIZED_THREAD_ID);
        }

        // start threads
        for(int i = 0; i < numThreads; i++) {
            rearRunners.emplace_back(RearRunner<ObjectType, ModelType>(i, obj, model, rearShrs[i].getRef(),
                                                                       *thread_ready[i], sync_ready, log, counter));
        }

        for (int i = 0; i < numThreads; i++) {
            while (!*thread_ready[i]) {}
        }

        sync_ready = true;

        for (int i = 0; i < numThreads; i++) {
            rearRunners[i].join();
        }

        cout << "count: " << counter << ", order: ";
        auto frontItr = front.begin();
        vector<typename ActionList::iterator> rearItrs;
        vector<bool> rearStarted;
        for(int i = 0; i < numThreads; i++) {
            rearItrs.push_back(rears[i].begin());
            rearStarted.push_back(false);
        }

        for (int i = 0; i < counter; i++) {
            int threadId = log[i];
            // front
            if(threadId == FRONT_THREAD_ID) {
                cout << (*frontItr) << " -> ";
                ++frontItr;
            }
            // rear
            else {
                if(rearStarted[threadId]) {
                    cout << "thr" << threadId << " " << (*rearItrs[threadId]) << " end -> ";
                    ++rearItrs[threadId];
                }
                else {
                    cout << "thr" << threadId << " " << (*rearItrs[threadId]) << " start -> ";
                }
                rearStarted[threadId] = rearStarted[threadId] ? false : true;
            }
        }

        cout << "onCleanup" << endl;
    });

    spawner.join();

    if(postCheckPtr)
        (*postCheckPtr)(obj, model);
    return true;
}

template <typename ObjectType, typename ModelType>
void Concurrency<ObjectType, ModelType>::handleShrink(Random&)
{
}

/* without model */
template <typename ObjectType, typename InitialGen>
decltype(auto) concurrency(InitialGen&& initialGen, SimpleActionGen<ObjectType>& actionGen)
{
    using ObjectTypeGen = GenFunction<ObjectType>;
    using ActionType = Action<ObjectType, EmptyModel>;

    auto actionGen2 = actionGen.template map<Action<ObjectType, EmptyModel>>(
        [](SimpleAction<ObjectType>& simpleAction) {
            return Action<ObjectType,EmptyModel>(simpleAction);
        });

    auto initialGenPtr = util::make_shared<ObjectTypeGen>(util::forward<InitialGen>(initialGen));
    auto actionGenPtr = util::make_shared<GenFunction<ActionType>>(actionGen2);
    return Concurrency<ObjectType, EmptyModel>(initialGenPtr, actionGenPtr);
}

/* with model */
template <typename ObjectType, typename ModelType, typename InitialGen, typename ModelFactory>
decltype(auto) concurrency(InitialGen&& initialGen, ModelFactory&& modelFactory, ActionGen<ObjectType, ModelType>& actionGen)
{
    using ModelFactoryFunction = function<ModelType(ObjectType&)>;
    using ObjectTypeGen = GenFunction<ObjectType>;
    using ActionType = Action<ObjectType, ModelType>;

    shared_ptr<ModelFactoryFunction> modelFactoryPtr =
        util::make_shared<ModelFactoryFunction>(util::forward<ModelFactory>(modelFactory));

    auto initialGenPtr = util::make_shared<ObjectTypeGen>(util::forward<InitialGen>(initialGen));
    auto actionGenPtr = util::make_shared<GenFunction<ActionType>>(util::forward<ActionGen<ObjectType, ModelType>>(actionGen));
    return Concurrency<ObjectType, ModelType>(initialGenPtr, modelFactoryPtr, actionGenPtr);
}

}  // namespace concurrent
}  // namespace proptest
