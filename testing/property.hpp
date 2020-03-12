#ifndef TESTING_PROPERTY_HPP
#define TESTING_PROPERTY_HPP

#include "testing/api.hpp"
#include "testing/gen.hpp"
#include "testing/function_traits.hpp"
#include "testing/tuple.hpp"
#include "testing/Stream.hpp"
#include "testing/Map.hpp"
#include "testing/printing.hpp"

#include <iostream>

namespace PropertyBasedTesting
{

class Random;

class PROPTEST_API PropertyBase {
public:
    PropertyBase();
    bool check();
    virtual ~PropertyBase() {}

protected:
    virtual void invoke(Random& rand) = 0;
    virtual void handleShrink(const PropertyFailedBase& e) = 0;
protected:
    // TODO: configurations
    uint64_t seed;

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

template <typename T>
decltype(auto) GetShrinksHelper(const Shrinkable<T>& shr) {
    return shr.shrinks();
}
/*
template <typename T>
decltype(auto) GetShrinksHelper(Shrinkable<T> shr) {
    return shr.shrinks();
}
*/

template <typename T>
struct GetShrinks {
    static Stream<T> map(T&& v) {
        return GetShrinksHelper(v);
    }
};

template <typename P>
Iterator<Shrinkable<P>> GetIteratorHelper(Stream<Shrinkable<P>>&& stream) {
    return stream.iterator();
}


template <typename T>
struct GetIterator {
    static decltype(auto) map(T&& stream) {
        return GetIteratorHelper(std::move(stream));
    }
};

template <typename P>
bool HasNextHelper(Iterator<Shrinkable<P>>& itr) {
    return itr.hasNext();
}

template <typename T>
struct HasNext {
    static decltype(auto) map(T&& itr) {
        return  HasNextHelper(itr);
    }
};


template <typename P>
Shrinkable<P> GetNextHelper(Iterator<Shrinkable<P>>& itr) {
    return itr.next();
}

template <typename T>
struct GetNext {
    static decltype(auto) map(T&& itr) {
        return  GetNextHelper(itr);
    }
};

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

    template <size_t N, typename ValueTuple, typename Replace>
    bool test(ValueTuple&& valueTup, Replace&& replace) {
        //std::cout << "    test: tuple ";
        //show(std::cout, valueTup);
        //std::cout << " replaced with arg " << N << ": ";
        //show(std::cout, replace); 
        //std::cout << std::endl;

        bool result = false;
        try {
            result = invokeWithArgTupleWithReplace<N>(std::move(callableWrapper.callable), std::move(valueTup), replace);
            //std::cout << "    test done: result=" << (result ? "true" : "false") << std::endl;
        }
        catch(const AssertFailed& e) {
            //std::cout << "    test failed with AssertFailed: result=" << (result ? "true" : "false") << std::endl;
            // TODO: trace
        }
        catch(const std::exception& e) {
            //std::cout << "    test failed with std::exception: result=" << (result ? "true" : "false") << std::endl;
            // TODO: trace
        }
        return result;
    }

    template <typename Shrinks>
    static void printShrinks(const Shrinks& shrinks) {
        auto itr = shrinks.iterator();
        std::cout << "    shrinks: " << std::endl;
        for(int i = 0; i < 4 && itr.hasNext(); i++) {
            std::cout << "    ";
            show(std::cout, itr.next());
            std::cout << std::endl;
        }
    }

    template <size_t N, typename ValueTuple, typename ShrinksTuple>
    decltype(auto) shrinkN(ValueTuple&& valueTup, ShrinksTuple&& shrinksTuple) {
        std::cout << "  shrinking arg " << N << ":" << std::endl;
        auto shrinks = std::get<N>(shrinksTuple);
        // keep shrinking until no shrinking is possible
        while(!shrinks.isEmpty()) {
            //printShrinks(shrinks);
            auto iter = shrinks.iterator();
            bool shrinkFound = false;
            // keep trying until failure is reproduced
            while(iter.hasNext()) {
                // get shrinkable
                auto next = iter.next();
                if(!test<N>(valueTup, next)) {
                    shrinks = next.shrinks();
                    std::get<N>(valueTup) = std::move(next);
                    shrinkFound = true;
                    break;
                }
            }
            if(shrinkFound) {
                std::cout << "  shrinking arg " << N << " found: ";
                show(std::cout, valueTup);
                std::cout << std::endl;
            }
            else {
                break;
            }
        }
        std::cout << "  no more shrinking found for arg " << N << std::endl;
        return std::get<N>(valueTup);
    }

    template <typename ValueTuple, typename ShrinksTuple, std::size_t...index>
    decltype(auto) shrinkEach(ValueTuple&& valueTup, ShrinksTuple&& shrinksTup, std::index_sequence<index...>) {
        return std::make_tuple(shrinkN<index>(valueTup, shrinksTup)...);
    }

    template <typename ValueTuple>
    void shrink(ValueTuple&& valueTup) {
        std::cout << "shrinking value: ";
        show(std::cout, valueTup);
        std::cout << std::endl;

        // TODO: serialize initial value as stream before mutating
        static constexpr auto Size = std::tuple_size<std::decay_t<ValueTuple>>::value;
        auto shrinksTuple = mapHeteroTuple<GetShrinks>(std::move(valueTup));
        auto shrunk = shrinkEach(std::move(valueTup),
                std::move(shrinksTuple),
                std::make_index_sequence<Size>{});

    }

    virtual void handleShrink(const PropertyFailedBase& e) {
        auto retTypeTup = ReturnTypeTupleFromGenTup(genTup);
        using ValueTuple = typename decltype(retTypeTup)::type_tuple;
        auto failed = dynamic_cast<const PropertyFailed<ValueTuple>&>(e);
        shrink(failed.valueTup);
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

