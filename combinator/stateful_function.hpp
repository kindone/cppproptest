#pragma once

#include <stdexcept>
#include <list>
#include <memory>
#include <functional>
#include "../util/function_traits.hpp"
#include "../combinator/transform.hpp"
#include "../combinator/oneof.hpp"
#include "../gen.hpp"
#include "../Shrinkable.hpp"
#include "../Random.hpp"
#include "../GenBase.hpp"
#include "just.hpp"

namespace proptest {

namespace stateful {

struct EmptyModel
{
};

template <typename ObjectType>
using SimpleAction = std::function<bool(ObjectType&)>;

template <typename ObjectType, typename ModelType>
using Action = std::function<bool(ObjectType&, ModelType&)>;

template <typename ObjectType, typename ModelType>
using ActionListGen = GenFunction<std::list<Action<ObjectType, ModelType>>>;

template <typename ObjectType, typename... GENS,
          std::enable_if_t<std::is_convertible<GENS, std::function<bool(ObjectType&)>>::value>...>
ActionListGen<ObjectType, EmptyModel> actionListGenOf(GENS... gens)
{
    auto actionGen = oneOf<std::function<bool(ObjectType&)>>(gens...);

    auto actionGen2 = actionGen.template map<std::function<bool(ObjectType&, EmptyModel&)>>(
        [](std::function<bool(ObjectType&)>& action) {
            // TODO: shared_ptr action?
            return [action](ObjectType& obj, EmptyModel&) { return action(obj); };
        });
    auto actionListGen2 = Arbi<std::list<std::function<bool(ObjectType&, EmptyModel&)>>>(actionGen2);
    return actionListGen2;
}

template <typename ObjectType, typename ModelType, typename... GENS,
          std::enable_if_t<std::is_convertible<GENS, std::function<bool(ObjectType&, ModelType&)>>::value>...>
ActionListGen<ObjectType, ModelType> actionListGenOf(GENS... gens)
{
    auto actionGen = oneOf<std::function<bool(ObjectType&, ModelType&)>>(gens...);
    auto actionListGen = Arbi<std::list<std::function<bool(ObjectType&, ModelType&)>>>(actionGen);
    return actionListGen;
}

template <typename ObjectType, typename InitialGen>
decltype(auto) statefulProperty(InitialGen&& initialGen, ActionListGen<ObjectType, EmptyModel>& actionsGen)
{
    static EmptyModel emptyModel;
    return property(
        [](ObjectType obj, std::list<std::function<bool(ObjectType&, EmptyModel&)>> actions) {
            for (auto action : actions) {
                PROP_ASSERT(action(obj, emptyModel));
            }
            return true;
        },
        /*Arbi<ObjectType>()*/ std::forward<InitialGen>(initialGen), actionsGen);
}

template <typename ObjectType, typename ModelType, typename InitialGen>
decltype(auto) statefulProperty(InitialGen&& initialGen, std::function<ModelType(ObjectType&)> modelFactory,
                                ActionListGen<ObjectType, ModelType>& actionsGen)
{
    using ModelFactoryFunction = std::function<ModelType(ObjectType&)>;
    std::shared_ptr<ModelFactoryFunction> modelFactoryPtr =
        std::make_shared<ModelFactoryFunction>(std::forward<ModelFactoryFunction>(modelFactory));

    return property(
        [modelFactoryPtr](ObjectType obj, std::list<std::function<bool(ObjectType&, ModelType&)>> actions) {
            auto model = (*modelFactoryPtr)(obj);
            for (auto action : actions) {
                PROP_ASSERT(action(obj, model));
            }
            return true;
        },
        std::forward<InitialGen>(initialGen), actionsGen);
}

}  // namespace stateful
}  // namespace proptest
