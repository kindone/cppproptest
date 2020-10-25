#pragma once

#include <stdexcept>
#include <list>
#include <memory>
#include <functional>
#include "../util/function_traits.hpp"
#include "../util/action.hpp"
#include "../combinator/transform.hpp"
#include "../combinator/oneof.hpp"
#include "../gen.hpp"
#include "../Shrinkable.hpp"
#include "../Random.hpp"
#include "../GenBase.hpp"
#include "just.hpp"

namespace proptest {

template <typename... ARGS>
class Property;
namespace stateful {

// template <typename ObjectType, typename ModelType>
// using ActionListGen = GenFunction<std::list<Action<ObjectType, ModelType>>>;
template <typename ObjectType>
using SimpleActionGen = Generator<SimpleAction<ObjectType>>;

template <typename ObjectType, typename ModelType>
using ActionGen = Generator<Action<ObjectType, ModelType>>;

template <typename ObjectType, typename ModelType>
class StatefulProperty {
    using InitialGen = GenFunction<ObjectType>;
    using ModelFactoryFunction = std::function<ModelType(ObjectType&)>;
    using PropertyType = Property<ObjectType, std::list<Action<ObjectType,ModelType>>>;
    using Func = std::function<bool(ObjectType, std::list<Action<ObjectType,ModelType>>)>;

public:
    StatefulProperty(InitialGen&& initGen, ModelFactoryFunction mdlFactory, ActionGen<ObjectType, ModelType>& actGen)
         : seed(UINT64_MAX), numRuns(UINT32_MAX), initialGen(initGen), modelFactory(mdlFactory), actionGen(actGen)
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

    StatefulProperty& setOnStartup(std::function<void()> onStartup) {
        onStartupPtr = std::make_shared<std::function<void()>>(onStartup);
        return *this;
    }

    StatefulProperty& setOnCleanup(std::function<void()> onCleanup) {
        onCleanupPtr = std::make_shared<std::function<void()>>(onCleanup);
        return *this;
    }

    StatefulProperty& setPostCheck(std::function<void(ObjectType&, ModelType&)> postCheck)  {
        postCheckPtr = std::make_shared(postCheck);
        return *this;
    }

    StatefulProperty& setPostCheck(std::function<void(ObjectType&)> postCheck)  {
        std::function<void(ObjectType&,ModelType&)>  fullPostCheck = [postCheck](ObjectType& sys, ModelType&) { postCheck(sys); };
        postCheckPtr = std::make_shared(fullPostCheck);
        return *this;
    }

    bool go() {
        // TODO add interface to adjust list min max sizes
        auto actionListGen = Arbi<std::list<Action<ObjectType,ModelType>>>(actionGen);
        auto genTup = std::make_tuple(std::forward<InitialGen>(initialGen), actionListGen);
        std::shared_ptr<ModelFactoryFunction> modelFactoryPtr =
            std::make_shared<ModelFactoryFunction>(std::forward<ModelFactoryFunction>(modelFactory));

        auto func = [modelFactoryPtr](ObjectType obj, std::list<Action<ObjectType,ModelType>> actions) {
            auto model = (*modelFactoryPtr)(obj);
            for (auto action : actions) {
                action(obj, model);
            }
            return true;
        };

        auto prop = std::make_shared<PropertyType>(func, genTup);
        if(onStartupPtr)
            prop->setOnStartup(*onStartupPtr);
        if(onCleanupPtr)
            prop->setOnStartup(*onCleanupPtr);
        if(seed != UINT64_MAX)
            prop->setSeed(seed);
        if(numRuns != UINT32_MAX)
            prop->setNumRuns(numRuns);
        return prop->forAll();
    }

private:
    uint64_t seed;
    uint32_t numRuns;
    InitialGen initialGen;
    ModelFactoryFunction modelFactory;
    ActionGen<ObjectType, ModelType> actionGen;

    std::shared_ptr<std::function<void(ObjectType&, ModelType&)>> postCheckPtr;
    std::shared_ptr<std::function<void()>> onStartupPtr;
    std::shared_ptr<std::function<void()>> onCleanupPtr;
};

template <typename ObjectType, typename InitialGen>
decltype(auto) statefulProperty(InitialGen&& initialGen, SimpleActionGen<ObjectType>& actionGen)
{
    static EmptyModel emptyModel;
    auto actionGen2 = actionGen.template map<Action<ObjectType, EmptyModel>>(
        [](SimpleAction<ObjectType>& simpleAction) {
            return Action<ObjectType,EmptyModel>(simpleAction);
        });

    auto modelFactory = +[](ObjectType&) { return emptyModel; };
    return StatefulProperty<ObjectType, EmptyModel>(std::forward<InitialGen>(initialGen), modelFactory, actionGen2);
}

template <typename ObjectType, typename ModelType, typename InitialGen>
decltype(auto) statefulProperty(InitialGen&& initialGen, std::function<ModelType(ObjectType&)> modelFactory,
                                ActionGen<ObjectType, ModelType>& actionGen)
{
    return StatefulProperty<ObjectType, ModelType>(
        std::forward<InitialGen>(initialGen), modelFactory, actionGen);
}

}  // namespace stateful
}  // namespace proptest
