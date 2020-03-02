#ifndef TESTING_PROPERTY_HPP
#define TESTING_PROPERTY_HPP

#include "testing/api.hpp"
#include "testing/gen.hpp"
#include "testing/function_traits.hpp"
#include "testing/tuple.hpp"

namespace PropertyBasedTesting
{

class Random;

class PROPTEST_API PropertyBase {
public:
    bool check();
    virtual ~PropertyBase() {}

protected:
    virtual void invoke(Random& rand) = 0;
    virtual void shrink(const PropertyFailedBase& e) = 0;
protected:
    // TODO: configurations
    uint64_t seed;

};


struct X {
    int generate(Random& rand) {
        return 0;
    }
};

template <typename T>
decltype(auto) ReturnTypeOf() {
    TypeHolder<typename std::result_of<decltype(&T::generate)(T, Random&)>::type> typeHolder;
    return typeHolder;
}

template <typename ...ARGS>
decltype(auto) ReturnTypeTupleFromGenTup(std::tuple<ARGS...>& tup) {
    TypeList<typename decltype(ReturnTypeOf<ARGS>())::type...> typeList;
    return typeList;
}

template <typename CallableWrapper, typename GenTuple>
class Property : public PropertyBase {
public:
    Property(CallableWrapper&& c, GenTuple& g) : callableWrapper(std::move(c)), genTup(g) {
    }

    virtual void invoke(Random& rand) {
        invokeWithGenTuple(rand, std::forward<decltype(callableWrapper.callable)>(callableWrapper.callable), genTup);
    }

    Property& setSeed(uint64_t s) {
        seed = s;
    }

    void shrinkFurther() {
        //TODO
    }

    virtual void shrink(const PropertyFailedBase& e) {
        auto retTup = ReturnTypeTupleFromGenTup(genTup);
        using ValueTuple = typename decltype(retTup)::type_tuple;
        auto failed = dynamic_cast<const PropertyFailed<ValueTuple>&>(e);
        
        try {
            bool result = invokeWithArgTuple(std::move(callableWrapper.callable), std::move(failed.valueTup));
            if(!result)
                shrinkFurther();
        }
        catch(const AssertFailed& e) {
            std::cerr << "oops, failed again!" << std::endl;
            shrinkFurther();
        }
        catch(const std::exception& e) {
            std::cerr << "oops, failed again!" << std::endl;
        }
    }

private:
    CallableWrapper callableWrapper;
    GenTuple genTup;
};

template <class Callable>
class CallableWrapper {
public:
    Callable&& callable;
    CallableWrapper(Callable&& c) : callable(std::forward<Callable>(c)) {
    }
};

template <class Callable>
auto make_CallableWrapper(Callable&& callable) {
    return CallableWrapper<typename std::remove_reference<Callable>::type>(std::move(callable));
}



template <typename Callable>
auto property(Callable&& callable) {
    using argument_type_tuple = typename function_traits<Callable>::argument_type_list::type_tuple;
    typename function_traits<Callable>::argument_type_list argument_type_list;
    // acquire tuple of generators
    auto genTup = createGenTuple(argument_type_list);
    return Property<CallableWrapper<typename std::remove_reference<Callable>::type>, decltype(genTup)>(make_CallableWrapper(callable), genTup);
}

template <typename ... GENS, typename Callable, typename std::enable_if<(sizeof...(GENS) > 0), bool>::type = true>
auto property(Callable&& callable) {
    // acquire tuple of generators
    using argument_type_tuple = typename std::tuple<GENS...>;
    auto genTup = createGenTuple<GENS...>();
    return Property<CallableWrapper<typename std::remove_reference<Callable>::type>, decltype(genTup)>(make_CallableWrapper(callable), genTup);
    //return Property<CallableWrapper<Callable>, decltype(genTup)>(wrapper, genTup);
}


// check with Arbitrary<ARG1>, Arbitrary<ARG2>, ... as generators
// check(rand, [](ARG1 arg1, ARG2) -> bool {});
template <typename Callable>
bool check(Random& rand, Callable&& callable) {
    return property(callable).check();
}

// check with generators specified
// check<CUSTOM_GEN_ARG1>, CUSTOM_GEN_ARG2>,...>(rand, [](ARG1 arg1, ARG2) -> bool {});
template <typename ... GENS, typename Callable, typename std::enable_if<(sizeof...(GENS) > 0), bool>::type = true>
bool check(Random& rand, Callable&& callable) {
    return property<GENS...>(callable).check();
}



} // namespace PropertyBasedTesting

#endif // TESTING_PROPERTY_HPP

