#pragma once

#include <stdexcept>
#include <vector>
#include <memory>
#include <functional>
#include "../Shrinkable.hpp"
#include "../Random.hpp"

namespace PropertyBasedTesting {

struct EmptyModel
{
    static EmptyModel value;
};

template <typename SYSTEM, typename MODEL>
struct Action
{
    using SystemType = SYSTEM;
    using ModelType = MODEL;

    virtual ~Action() {}
    virtual bool precondition(const SYSTEM& system, const MODEL& model) { return precondition(system); }

    virtual bool precondition(const SYSTEM& system) { return true; }

    virtual bool run(SYSTEM& system, MODEL& model) { return run(system); }

    virtual bool run(SYSTEM& system) { throw std::runtime_error("attempt to call undefined run"); };

    virtual bool postcondition(const SYSTEM& system, const MODEL& model) { return postcondition(system); }

    virtual bool postcondition(const SYSTEM& system) { return true; }
};

template <typename SYSTEM>
struct ActionWithoutModel : public Action<SYSTEM, EmptyModel>
{
    virtual ~ActionWithoutModel() {}
};

// template <typename ActionType, typename... GENS>
// std::function<Shrinkable<std::vector<std::shared_ptr<ActionType>>>(Random&)> actions(GENS&&... gens) {
//     auto actionGen = oneOf<std::shared_ptr<ActionType>>(std::forward<GENS>(gens)...);
//     auto actionVecGen = Arbitrary<std::vector<std::shared_ptr<ActionType>>>(actionGen);
//     return actionVecGen;
// }

namespace util {

template <typename ActionType, typename GEN,
          std::enable_if_t<std::is_pointer<typename function_traits<GEN>::return_type::type>::value, bool> = true>
decltype(auto) toSharedPtrGen(GEN&& gen)
{
    return transform<ActionType*, std::shared_ptr<ActionType>>(gen, [](const ActionType* actionType) {
        std::shared_ptr<ActionType> sharedPtr{const_cast<ActionType*>(actionType)};
        return sharedPtr;
    });
}

template <typename ActionType, typename GEN,
          std::enable_if_t<!std::is_pointer<typename function_traits<GEN>::return_type::type>::value, bool> = true>
decltype(auto) toSharedPtrGen(GEN&& gen)
{
    return std::forward<GEN>(gen);
}

}  // namespace util

template <typename ActionType, typename... GENS>
std::function<Shrinkable<std::vector<std::shared_ptr<ActionType>>>(Random&)> actions(GENS&&... gens)
{
    auto actionGen = oneOf<std::shared_ptr<ActionType>>(util::toSharedPtrGen<ActionType>(std::forward<GENS>(gens))...);
    auto actionVecGen = Arbitrary<std::vector<std::shared_ptr<ActionType>>>(actionGen);
    return actionVecGen;
}

template <typename ActionType, typename InitialGen, typename ActionsGen>
decltype(auto) statefulProperty(InitialGen&& initialGen, ActionsGen&& actionsGen)
{
    using SystemType = typename ActionType::SystemType;

    return property(
        [](SystemType obj, std::vector<std::shared_ptr<ActionType>> actions) {
            for (auto action : actions) {
                if (action->precondition(obj))
                    action->run(obj);
                PROP_ASSERT(action->postcondition(obj), {});
            }
            return true;
        },
        std::forward<InitialGen>(initialGen), std::forward<ActionsGen>(actionsGen));
}

template <typename ActionType, typename InitialGen, typename ModelFactory, typename ActionsGen>
decltype(auto) statefulProperty(InitialGen&& initialGen, ModelFactory&& modelFactory, ActionsGen&& actionsGen)
{
    using ModelType = typename ActionType::ModelType;
    using SystemType = typename ActionType::SystemType;
    using ModelFactoryFunction = std::function<ModelType(SystemType&)>;
    std::shared_ptr<ModelFactoryFunction> modelFactoryPtr =
        std::make_shared<ModelFactoryFunction>(std::forward<ModelFactory>(modelFactory));

    return property(
        [modelFactoryPtr](SystemType obj, std::vector<std::shared_ptr<ActionType>> actions) {
            auto model = (*modelFactoryPtr)(obj);
            for (auto action : actions) {
                if (action->precondition(obj, model))
                    action->run(obj, model);
                PROP_ASSERT(action->postcondition(obj, model), {});
            }
            return true;
        },
        std::forward<InitialGen>(initialGen), std::forward<ActionsGen>(actionsGen));
}

}  // namespace PropertyBasedTesting
