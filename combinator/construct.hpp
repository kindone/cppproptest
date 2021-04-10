#pragma once
#include "../gen.hpp"
#include "../util/createGenTuple.hpp"
#include "../util/std.hpp"

namespace proptest {

class Random;

namespace util {

template <typename TO, typename SHRINKABLE>
enable_if_t<!is_lvalue_reference<TO>::value, TO> autoCast(SHRINKABLE&& shr)
{
    return shr.get();
}

template <typename TO, typename SHRINKABLE>
enable_if_t<is_pointer<TO>::value, TO> autoCast(SHRINKABLE&& shr)
{
    return shr.getPtr();
}

template <typename TO, typename SHRINKABLE>
enable_if_t<is_lvalue_reference<TO>::value, TO> autoCast(SHRINKABLE&& shr)
{
    return shr.getRef();
}

template <typename ToTuple, size_t N, typename FromTuple>
decltype(auto) autoCastTuple(FromTuple&& tuple)
{
    return autoCast<typename tuple_element<N, ToTuple>::type>(get<N>(tuple));
}

}  // namespace util

template <class CLASS, typename... ARGTYPES>
class Construct : public GenBase<CLASS> {
public:
    using ArgumentList = util::TypeList<ARGTYPES...>;
    using GenTuple = tuple<GenFunction<remove_reference_t<ARGTYPES>>...>;

    static constexpr auto Size = sizeof...(ARGTYPES);

    Construct() : genTup(util::createGenTuple(ArgumentList{})) {}

    Construct(GenTuple g) : genTup(g) {}

    Shrinkable<CLASS> operator()(Random& rand) override { return constructAccordingly(generateArgs(rand)); }

    template <typename U>
    Generator<U> map(function<U(CLASS&)> mapper)
    {
        auto thisPtr = clone();
        return Generator<U>(
            proptest::transform<CLASS, U>([thisPtr](Random& rand) { return thisPtr->operator()(rand); }, mapper));
    }

    template <typename Criteria>
    Generator<CLASS> filter(Criteria&& criteria)
    {
        auto thisPtr = clone();
        return Generator<CLASS>(proptest::filter<CLASS>([thisPtr](Random& rand) { return thisPtr->operator()(rand); },
                                                        util::forward<Criteria>(criteria)));
    }

    template <typename U>
    Generator<pair<CLASS, U>> pairWith(function<GenFunction<U>(const CLASS&)> genFactory)
    {
        auto thisPtr = clone();
        return proptest::dependency<CLASS, U>([thisPtr](Random& rand) { return thisPtr->operator()(rand); }, genFactory);
    }

    template <typename U>
    decltype(auto) tupleWith(function<GenFunction<U>(CLASS&)> genFactory)
    {
        auto thisPtr = clone();
        return proptest::chain([thisPtr](Random& rand) { return thisPtr->operator()(rand); }, genFactory);
    }

    template <typename U>
    Generator<U> flatmap(function<U(CLASS&)> genFactory)
    {
        auto thisPtr = clone();
        return Generator<U>(
            proptest::derive<CLASS, U>([thisPtr](Random& rand) { return thisPtr->operator()(rand); }, genFactory));
    }

    shared_ptr<Construct<CLASS, ARGTYPES...>> clone()
    {
        return util::make_shared<Construct<CLASS, ARGTYPES...>>(*dynamic_cast<Construct<CLASS, ARGTYPES...>*>(this));
    }

private:
    template <size_t... index>
    decltype(auto) generateArgsHelper(Random& rand, index_sequence<index...>)
    {
        return util::make_tuple(get<index>(genTup)(rand)...);
    }

    decltype(auto) generateArgs(Random& rand) { return generateArgsHelper(rand, make_index_sequence<Size>{}); }

    template <typename CastTuple, typename ValueTuple, size_t... index>
    static Shrinkable<CLASS> constructByTupleType(ValueTuple&& valueTuple, index_sequence<index...>)
    {
        return make_shrinkable<CLASS>(util::autoCastTuple<CastTuple, index>(util::forward<ValueTuple>(valueTuple))...);
    }

    template <typename ValueTuple>
    static Shrinkable<CLASS> constructAccordingly(ValueTuple&& valueTuple)
    {
        using ArgsAsTuple = tuple<ARGTYPES...>;
        constexpr auto arity = sizeof...(ARGTYPES);
        return constructByTupleType<ArgsAsTuple>(
            util::forward<ValueTuple>(valueTuple), make_index_sequence<arity>{}  // {0,1,2,3,...,N-1}
        );
    }

    GenTuple genTup;
};

/**
 * @brief Generates a CLASS type by specifying target constructor's paremeter types
 *
 * Usage:
 *
 * @code
 *      struct Point {
 *          Point() : x(0), y(0) {}
 *          Point(int x, int y) : x(x), y(y) {}
 *          int x;
 *          int y;
 *      };
 *      GenFunction<Point> objectGen = construct<Point>(); // calls Point()
 *      GenFunction<Point> objectGen2 = construct<Point, int, int>(nonNegative(), nonNegative()); // calls Point(int,
 * int) GenFunction<Point> objectGen3 = construct<Point, int, int>(); // ints are generated using Arbi<int>
 * @endcode
 */
template <typename CLASS, typename... ARGTYPES>
decltype(auto) construct()
{
    constexpr auto ImplicitSize = sizeof...(ARGTYPES);
    using ArgsAsTuple = tuple<decay_t<ARGTYPES>...>;
    auto implicits = util::createGenHelperListed<ArgsAsTuple>(make_index_sequence<ImplicitSize>{});
    return Construct<CLASS, ARGTYPES...>(implicits);
}

// some explicits
template <typename CLASS, typename... ARGTYPES, typename EXPGEN0, typename... EXPGENS>
Construct<CLASS, ARGTYPES...> construct(EXPGEN0&& gen0, EXPGENS&&... gens)
{
    constexpr auto ExplicitSize = sizeof...(EXPGENS) + 1;
    constexpr auto ImplicitSize = sizeof...(ARGTYPES) - ExplicitSize;
    using ArgsAsTuple = tuple<decay_t<ARGTYPES>...>;

    auto explicits = util::make_tuple(util::genToFunc(gen0), util::genToFunc(gens)...);
    auto implicits = util::createGenHelperListed<ArgsAsTuple>(
        util::addOffset<ExplicitSize>(make_index_sequence<ImplicitSize>{}));

    return Construct<CLASS, ARGTYPES...>(tuple_cat(explicits, implicits));
}

}  // namespace proptest
