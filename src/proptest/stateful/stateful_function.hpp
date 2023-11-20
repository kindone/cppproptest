#pragma once

#include "proptest/util/std.hpp"
#include "proptest/util/function_traits.hpp"
#include "proptest/util/action.hpp"
#include "proptest/combinator/transform.hpp"
#include "proptest/combinator/oneof.hpp"
#include "proptest/gen.hpp"
#include "proptest/Shrinkable.hpp"
#include "proptest/Random.hpp"
#include "proptest/GenBase.hpp"
#include "proptest/combinator/just.hpp"

namespace proptest {

template <typename... ARGS>
class Property;
namespace stateful {

// template <typename ObjectType, typename ModelType>
// using ActionListGen = GenFunction<list<Action<ObjectType, ModelType>>>;
template <typename ObjectType>
using SimpleActionGen = Generator<SimpleAction<ObjectType>>;

template <typename ObjectType>
using SimpleActionGenFactory = function<Generator<SimpleAction<ObjectType>>(ObjectType&)>;

template <typename ObjectType, typename ModelType>
using ActionGen = Generator<Action<ObjectType, ModelType>>;

template <typename ObjectType, typename ModelType>
using ActionGenFactory = function<Generator<Action<ObjectType, ModelType>>(ObjectType&, ModelType&)>;


template <typename ObjectType, typename ModelType>
class StatefulProperty {
    using InitialGen = GenFunction<ObjectType>;
    using ModelFactoryFunction = function<ModelType(ObjectType&)>;
    using PropertyType = Property<ObjectType, list<Action<ObjectType, ModelType>>>;
    using Func = function<bool(ObjectType, list<Action<ObjectType, ModelType>>)>;

public:
    StatefulProperty(InitialGen&& initGen, ModelFactoryFunction mdlFactory, ActionGen<ObjectType, ModelType>& actGen)
        : seed(UINT64_MAX), numRuns(UINT32_MAX), maxDurationMs(UINT32_MAX), initialGen(initGen), modelFactory(mdlFactory), actionGen(actGen)
    {
    }

    StatefulProperty(InitialGen&& initGen, ModelFactoryFunction mdlFactory, ActionGenFactory<ObjectType, ModelType>& actGen)
        : seed(UINT64_MAX), numRuns(UINT32_MAX), maxDurationMs(UINT32_MAX), initialGen(initGen), modelFactory(mdlFactory), actionGen(actGen)
    {
    }

    StatefulProperty& setSeed(uint64_t s)
    {
        seed = s;
        return *this;
    }

    StatefulProperty& setNumRuns(uint32_t runs)
    {
        numRuns = runs;
        return *this;
    }

    StatefulProperty& setMaxDurationMs(uint32_t durationMs)
    {
        maxDurationMs = durationMs;
        return *this;
    }

    StatefulProperty& setOnStartup(function<void()> onStartup)
    {
        onStartupPtr = util::make_shared<function<void()>>(onStartup);
        return *this;
    }

    StatefulProperty& setOnCleanup(function<void()> onCleanup)
    {
        onCleanupPtr = util::make_shared<function<void()>>(onCleanup);
        return *this;
    }


    template <typename M = ModelType>
        requires(!is_same_v<M, EmptyModel>)
    StatefulProperty& setPostCheck(function<void(ObjectType&, ModelType&)> postCheck)
    {
        postCheckPtr = util::make_shared<function<void(ObjectType&, ModelType&)>>(postCheck);
        return *this;
    }

    template <typename M = ModelType>
        requires(is_same_v<M, EmptyModel>)
    StatefulProperty& setPostCheck(function<void(ObjectType&)> postCheck)
    {
        function<void(ObjectType&, ModelType&)> fullPostCheck = [postCheck](ObjectType& sys, ModelType&) {
            postCheck(sys);
        };
        postCheckPtr = util::make_shared(fullPostCheck);
        return *this;
    }

    bool go()
    {
        // TODO add interface to adjust list min max sizes
        auto actionListGen = Arbi<list<Action<ObjectType, ModelType>>>(actionGen);
        auto genTup = util::make_tuple(util::forward<InitialGen>(initialGen), actionListGen);
        shared_ptr<ModelFactoryFunction> modelFactoryPtr =
            util::make_shared<ModelFactoryFunction>(util::forward<ModelFactoryFunction>(modelFactory));

        auto func = [modelFactoryPtr, postCheckPtr = this->postCheckPtr](ObjectType obj,
                                                                         list<Action<ObjectType, ModelType>> actions) {
            auto model = (*modelFactoryPtr)(obj);
            for (auto action : actions) {
                action(obj, model);
            }
            if (postCheckPtr)
                (*postCheckPtr)(obj, model);
            return true;
        };

        auto prop = util::make_shared<PropertyType>(func, genTup);
        if (onStartupPtr)
            prop->setOnStartup(*onStartupPtr);
        if (onCleanupPtr)
            prop->setOnStartup(*onCleanupPtr);
        if (seed != UINT64_MAX)
            prop->setSeed(seed);
        if (numRuns != UINT32_MAX)
            prop->setNumRuns(numRuns);
        if (maxDurationMs != UINT32_MAX)
            prop->setMaxDurationMs(maxDurationMs);
        return prop->forAll();
    }

private:
    uint64_t seed;
    uint32_t numRuns;
    uint32_t maxDurationMs;
    InitialGen initialGen;
    ModelFactoryFunction modelFactory;
    ActionGen<ObjectType, ModelType> actionGen;

    shared_ptr<function<void(ObjectType&, ModelType&)>> postCheckPtr;
    shared_ptr<function<void()>> onStartupPtr;
    shared_ptr<function<void()>> onCleanupPtr;
};

template <typename ObjectType, typename InitialGen>
decltype(auto) statefulProperty(InitialGen&& initialGen, SimpleActionGen<ObjectType>& actionGen)
{
    static EmptyModel emptyModel;
    auto actionGen2 = actionGen.template map<Action<ObjectType, EmptyModel>>(
        [](SimpleAction<ObjectType>& simpleAction) { return Action<ObjectType, EmptyModel>(simpleAction); });

    auto modelFactory = +[](ObjectType&) { return emptyModel; };
    return StatefulProperty<ObjectType, EmptyModel>(util::forward<InitialGen>(initialGen), modelFactory, actionGen2);
}

template <typename ObjectType, typename ModelType, typename InitialGen>
decltype(auto) statefulProperty(InitialGen&& initialGen, function<ModelType(ObjectType&)> modelFactory,
                                ActionGen<ObjectType, ModelType>& actionGen)
{
    return StatefulProperty<ObjectType, ModelType>(util::forward<InitialGen>(initialGen), modelFactory, actionGen);
}

}  // namespace stateful
}  // namespace proptest
