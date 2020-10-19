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

namespace proptest {

template <typename... ARGS>
class Property;

namespace util {

template <typename ActionType, typename GEN>
std::enable_if_t<std::is_pointer<typename function_traits<GEN>::return_type::type>::value,
                 GenFunction<std::shared_ptr<ActionType>>>
toSharedPtrGen(GEN&& gen)
{
    return transform<ActionType*, std::shared_ptr<ActionType>>(
        gen, +[](const ActionType* actionType) {
            std::shared_ptr<ActionType> sharedPtr{const_cast<ActionType*>(actionType)};
            return sharedPtr;
        });
}

template <typename ActionType, typename GEN>
std::enable_if_t<!std::is_pointer<typename function_traits<GEN>::return_type::type>::value, GEN&&> toSharedPtrGen(
    GEN&& gen)
{
    return std::forward<GEN>(gen);
}

}  // namespace util
namespace stateful {
namespace alt {
struct EmptyModel
{
    static EmptyModel value;
};

template <typename SYSTEM, typename MODEL>
struct Action
{
    using ObjectType = SYSTEM;
    using ModelType = MODEL;

    virtual ~Action() {}
    virtual bool precondition(const SYSTEM& system, const MODEL&) { return precondition(system); }

    virtual bool precondition(const SYSTEM&) { return true; }

    virtual bool run(SYSTEM& system, MODEL&) { return run(system); }

    virtual bool run(SYSTEM&) { throw std::runtime_error("attempt to call undefined run"); }
};

template <typename SYSTEM>
struct SimpleAction : public Action<SYSTEM, EmptyModel>
{
    virtual ~SimpleAction() {}
};

template <typename ActionType>
class StatefulProperty {
    using ObjectType = typename ActionType::ObjectType;
    using ModelType = typename ActionType::ModelType;
    using InitialGen = GenFunction<ObjectType>;
    // using ActionType = Action<ObjectType, ModelType>;
    using PropertyType = Property<ObjectType, std::list<std::shared_ptr<ActionType>>>;
    using Func = std::function<bool(ObjectType, std::list<std::shared_ptr<ActionType>>)>;
    using ActionListGen = GenFunction<std::list<std::shared_ptr<ActionType>>>;

public:
    StatefulProperty(Func func, InitialGen&& initialGen, ActionListGen& actionListGen)
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

    bool go() { return prop->forAll(); }

private:
    std::shared_ptr<PropertyType> prop;
};

template <typename ActionType, typename... GENS>
GenFunction<std::list<std::shared_ptr<ActionType>>> actionListGenOf(GENS&&... gens)
{
    auto actionGen = oneOf<std::shared_ptr<ActionType>>(util::toSharedPtrGen<ActionType>(std::forward<GENS>(gens))...);
    auto actionVecGen = Arbi<std::list<std::shared_ptr<ActionType>>>(actionGen);
    return actionVecGen;
}

template <typename ActionType, typename InitialGen, typename ActionListGen>
decltype(auto) statefulProperty(InitialGen&& initialGen, ActionListGen&& actionListGen)
{
    using ObjectType = typename ActionType::ObjectType;

    return StatefulProperty<ActionType>(
        +[](ObjectType obj, std::list<std::shared_ptr<ActionType>> actions) {
            for (auto action : actions) {
                if (action->precondition(obj))
                    PROP_ASSERT(action->run(obj));
            }
            return true;
        },
        std::forward<InitialGen>(initialGen), std::forward<ActionListGen>(actionListGen));
}

template <typename ActionType, typename InitialGen, typename ModelFactory, typename ActionListGen>
decltype(auto) statefulProperty(InitialGen&& initialGen, ModelFactory&& modelFactory, ActionListGen&& actionListGen)
{
    using ModelType = typename ActionType::ModelType;
    using ObjectType = typename ActionType::ObjectType;
    using ModelFactoryFunction = std::function<ModelType(ObjectType&)>;
    std::shared_ptr<ModelFactoryFunction> modelFactoryPtr =
        std::make_shared<ModelFactoryFunction>(std::forward<ModelFactory>(modelFactory));

    return StatefulProperty<ActionType>(
        [modelFactoryPtr](ObjectType obj, std::list<std::shared_ptr<ActionType>> actions) {
            auto model = (*modelFactoryPtr)(obj);
            for (auto action : actions) {
                if (action->precondition(obj, model))
                    PROP_ASSERT(action->run(obj, model));
            }
            return true;
        },
        std::forward<InitialGen>(initialGen), actionListGen);
}

}  // namespace alt
}  // namespace stateful
}  // namespace proptest
