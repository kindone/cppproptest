#pragma once

#include "proptest/util/std.hpp"
#include "proptest/util/function_traits.hpp"
#include "proptest/combinator/transform.hpp"
#include "proptest/combinator/oneof.hpp"
#include "proptest/gen.hpp"
#include "proptest/Shrinkable.hpp"
#include "proptest/Random.hpp"
#include "proptest/GenBase.hpp"

namespace proptest {

template <typename... ARGS>
class Property;

namespace util {

template <typename ActionType, typename GEN>
    requires(is_pointer<typename function_traits<GEN>::return_type::type>::value)
GenFunction<shared_ptr<ActionType>> toSharedPtrGen(GEN&& gen)
{
    return transform<ActionType*, shared_ptr<ActionType>>(
        gen, +[](const ActionType* actionType) {
            shared_ptr<ActionType> sharedPtr{const_cast<ActionType*>(actionType)};
            return sharedPtr;
        });
}

template <typename ActionType, typename GEN>
    requires(!is_pointer<typename function_traits<GEN>::return_type::type>::value)
GEN&& toSharedPtrGen(
    GEN&& gen)
{
    return util::forward<GEN>(gen);
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

    virtual bool run(SYSTEM&) { throw runtime_error("attempt to call undefined run"); }
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
    using PropertyType = Property<ObjectType, list<shared_ptr<ActionType>>>;
    using Func = function<bool(ObjectType, list<shared_ptr<ActionType>>)>;
    using ActionListGen = GenFunction<list<shared_ptr<ActionType>>>;

public:
    StatefulProperty(Func func, InitialGen&& initialGen, ActionListGen& actionListGen)
    {
        auto genTup = util::make_tuple(util::forward<InitialGen>(initialGen), actionListGen);
        prop = util::make_shared<PropertyType>(func, genTup);
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

    StatefulProperty& setMaxDurationMs(uint32_t maxDurationMs)
    {
        prop->setMaxDurationMs(maxDurationMs);
        return *this;
    }

    bool go() { return prop->forAll(); }

private:
    shared_ptr<PropertyType> prop;
};

template <typename ActionType, typename... GENS>
GenFunction<list<shared_ptr<ActionType>>> actionListGenOf(GENS&&... gens)
{
    auto actionGen = oneOf<shared_ptr<ActionType>>(util::toSharedPtrGen<ActionType>(util::forward<GENS>(gens))...);
    auto actionVecGen = Arbi<list<shared_ptr<ActionType>>>(actionGen);
    return actionVecGen;
}

template <typename ActionType, typename InitialGen, typename ActionListGen>
decltype(auto) statefulProperty(InitialGen&& initialGen, ActionListGen&& actionListGen)
{
    using ObjectType = typename ActionType::ObjectType;

    return StatefulProperty<ActionType>(
        +[](ObjectType obj, list<shared_ptr<ActionType>> actions) {
            for (auto action : actions) {
                if (action->precondition(obj))
                    PROP_ASSERT(action->run(obj));
            }
            return true;
        },
        util::forward<InitialGen>(initialGen), util::forward<ActionListGen>(actionListGen));
}

template <typename ActionType, typename InitialGen, typename ModelFactory, typename ActionListGen>
decltype(auto) statefulProperty(InitialGen&& initialGen, ModelFactory&& modelFactory, ActionListGen&& actionListGen)
{
    using ModelType = typename ActionType::ModelType;
    using ObjectType = typename ActionType::ObjectType;
    using ModelFactoryFunction = function<ModelType(ObjectType&)>;
    shared_ptr<ModelFactoryFunction> modelFactoryPtr =
        util::make_shared<ModelFactoryFunction>(util::forward<ModelFactory>(modelFactory));

    return StatefulProperty<ActionType>(
        [modelFactoryPtr](ObjectType obj, list<shared_ptr<ActionType>> actions) {
            auto model = (*modelFactoryPtr)(obj);
            for (auto action : actions) {
                if (action->precondition(obj, model))
                    PROP_ASSERT(action->run(obj, model));
            }
            return true;
        },
        util::forward<InitialGen>(initialGen), actionListGen);
}

}  // namespace alt
}  // namespace stateful
}  // namespace proptest
