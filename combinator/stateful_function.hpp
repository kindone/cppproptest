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

template <typename SystemType>
using Action = std::function<bool(SystemType&)>;

template <typename SystemType, typename ModelType>
using ActionListGen = GenFunction<std::list<std::function<bool(SystemType&, ModelType&)>>>;

template <typename SystemType, typename ModelType>
using ActionWithModel = std::function<bool(SystemType&, ModelType&)>;

/*
template <typename SystemType>
decltype(auto) actionGen(Action<SystemType> func)
{
    return just<Action<SystemType>>(func);
}
*/

template <typename SystemType, typename... GENS,
          std::enable_if_t<std::is_convertible<GENS, std::function<bool(SystemType&)>>::value>...>
ActionListGen<SystemType, EmptyModel> actionListGenFrom(GENS... gens)
{
    auto actionGen = oneOf<std::function<bool(SystemType&)>>(gens...);

    auto actionGen2 = actionGen.template map<std::function<bool(SystemType&, EmptyModel&)>>(
        [](std::function<bool(SystemType&)>& action) {
            // TODO: shared_ptr action?
            return [action](SystemType& obj, EmptyModel&) { return action(obj); };
        });
    auto actionListGen2 = Arbi<std::list<std::function<bool(SystemType&, EmptyModel&)>>>(actionGen2);
    return actionListGen2;
}

template <typename SystemType, typename ModelType, typename... GENS,
          std::enable_if_t<std::is_convertible<GENS, std::function<bool(SystemType&, ModelType&)>>::value>...>
ActionListGen<SystemType, ModelType> actionListGenFrom(GENS... gens)
{
    auto actionGen = oneOf<std::function<bool(SystemType&, ModelType&)>>(gens...);
    auto actionListGen = Arbi<std::list<std::function<bool(SystemType&, ModelType&)>>>(actionGen);
    return actionListGen;
}
/*
template <typename SystemType, typename... GENS>
decltype(auto) actionProperty(GENS&&... gens)
{
    auto actionsGen = actions<SystemType>(std::forward<GENS>(gens)...);

    return property(
        [](SystemType obj, std::list<std::function<bool(SystemType&)>> actions) {
            for (auto action : actions) {
                PROP_ASSERT(action(obj));
            }
            return true;
        },
        Arbi<SystemType>(), actionsGen);
}
*/

template <typename SystemType, typename InitialGen>
decltype(auto) actionProperty(InitialGen&& initialGen, ActionListGen<SystemType, EmptyModel>& actionsGen)
{
    static EmptyModel emptyModel;
    return property(
        [](SystemType obj, std::list<std::function<bool(SystemType&, EmptyModel&)>> actions) {
            for (auto action : actions) {
                PROP_ASSERT(action(obj, emptyModel));
            }
            return true;
        },
        /*Arbi<SystemType>()*/ std::forward<InitialGen>(initialGen), actionsGen);
}

template <typename SystemType, typename ModelType, typename InitialGen>
decltype(auto) actionProperty(InitialGen&& initialGen, std::function<ModelType(SystemType&)> modelFactory,
                              ActionListGen<SystemType, ModelType>& actionsGen)
{
    using ModelFactoryFunction = std::function<ModelType(SystemType&)>;
    std::shared_ptr<ModelFactoryFunction> modelFactoryPtr =
        std::make_shared<ModelFactoryFunction>(std::forward<ModelFactoryFunction>(modelFactory));

    return property(
        [modelFactoryPtr](SystemType obj, std::list<std::function<bool(SystemType&, ModelType&)>> actions) {
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
