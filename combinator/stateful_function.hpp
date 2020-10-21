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

// template <typename ObjectType>
// using SimpleAction = std::function<bool(ObjectType&)>;
// template <typename ObjectType, typename ModelType>
// using Action = std::function<bool(ObjectType&, ModelType&)>;

template <typename ObjectType, typename ModelType>
using ActionListGen = GenFunction<std::list<Action<ObjectType, ModelType>>>;

template <typename ObjectType, typename ModelType>
class StatefulProperty {
    using InitialGen = GenFunction<ObjectType>;
    using PropertyType = Property<ObjectType, std::list<Action<ObjectType,ModelType>>>;
    using Func = std::function<bool(ObjectType, std::list<Action<ObjectType,ModelType>>)>;

public:
    StatefulProperty(Func func, InitialGen&& initialGen, ActionListGen<ObjectType, ModelType>& actionListGen)
    {
        auto genTup = std::make_tuple(std::forward<InitialGen>(initialGen), actionListGen);
        prop = std::make_shared<PropertyType>(func, genTup);
    }

    StatefulProperty& setSeed(uint64_t s)
    {
        prop->setSeed(s);
        return *this;
    }

    StatefulProperty& setNumRuns(uint32_t runs)
    {
        prop->setNumRuns(runs);
        return *this;
    }

    StatefulProperty& setOnStartup(std::function<void()> func) {
        prop->setOnStartup(func);
        return *this;
    }

    StatefulProperty& setOnCleanup(std::function<void()> func) {
        prop->setOnCleanup(func);
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

    bool go() { return prop->forAll(); }

private:
    std::shared_ptr<PropertyType> prop;
    std::shared_ptr<std::function<void(ObjectType&, ModelType&)>> postCheckPtr;
};

template <typename ObjectType, typename... GENS,
          std::enable_if_t<std::is_convertible<GENS, SimpleAction<ObjectType>>::value>...>
ActionListGen<ObjectType, EmptyModel> actionListGenOf(GENS... gens)
{
    auto actionGen = oneOf<SimpleAction<ObjectType>>(gens...);

    auto actionGen2 = actionGen.template map<Action<ObjectType, EmptyModel>>(
        [](SimpleAction<ObjectType>& simpleAction) {
            return Action<ObjectType,EmptyModel>(simpleAction);
        });
    auto actionListGen2 = Arbi<std::list<Action<ObjectType,EmptyModel>>>(actionGen2);
    return actionListGen2;
}

template <typename ObjectType, typename ModelType, typename... GENS,
          std::enable_if_t<std::is_convertible<GENS, Action<ObjectType,ModelType>>::value>...>
ActionListGen<ObjectType, ModelType> actionListGenOf(GENS... gens)
{
    auto actionGen = oneOf<Action<ObjectType,ModelType>>(gens...);
    auto actionListGen = Arbi<std::list<Action<ObjectType,ModelType>>>(actionGen);
    return actionListGen;
}

template <typename ObjectType, typename InitialGen>
decltype(auto) statefulProperty(InitialGen&& initialGen, ActionListGen<ObjectType, EmptyModel>& actionListGen)
{
    static EmptyModel emptyModel;
    return StatefulProperty<ObjectType, EmptyModel>(
        +[](ObjectType obj, std::list<Action<ObjectType,EmptyModel>> actions) {
            for (auto action : actions) {
                PROP_ASSERT(action(obj, emptyModel));
            }
            return true;
        },
        std::forward<InitialGen>(initialGen), actionListGen);
}

template <typename ObjectType, typename ModelType, typename InitialGen>
decltype(auto) statefulProperty(InitialGen&& initialGen, std::function<ModelType(ObjectType&)> modelFactory,
                                ActionListGen<ObjectType, ModelType>& actionListGen)
{
    using ModelFactoryFunction = std::function<ModelType(ObjectType&)>;
    std::shared_ptr<ModelFactoryFunction> modelFactoryPtr =
        std::make_shared<ModelFactoryFunction>(std::forward<ModelFactoryFunction>(modelFactory));

    return StatefulProperty<ObjectType, ModelType>(
        [modelFactoryPtr](ObjectType obj, std::list<Action<ObjectType,ModelType>> actions) {
            auto model = (*modelFactoryPtr)(obj);
            for (auto action : actions) {
                PROP_ASSERT(action(obj, model));
            }
            return true;
        },
        std::forward<InitialGen>(initialGen), actionListGen);
}

}  // namespace stateful
}  // namespace proptest
