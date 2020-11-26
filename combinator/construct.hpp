#pragma once
#include "../gen.hpp"
#include "../util/createGenTuple.hpp"
#include <type_traits>

namespace proptest {

class Random;

namespace util {

template <typename TO, typename SHRINKABLE>
std::enable_if_t<!std::is_lvalue_reference<TO>::value, TO> autoCast(SHRINKABLE&& shr)
{
    return shr.get();
}

template <typename TO, typename SHRINKABLE>
std::enable_if_t<std::is_pointer<TO>::value, TO> autoCast(SHRINKABLE&& shr)
{
    return shr.getPtr();
}

template <typename TO, typename SHRINKABLE>
std::enable_if_t<std::is_lvalue_reference<TO>::value, TO> autoCast(SHRINKABLE&& shr)
{
    return shr.getRef();
}

template <typename ToTuple, size_t N, typename FromTuple>
decltype(auto) autoCastTuple(FromTuple&& tuple)
{
    return autoCast<typename std::tuple_element<N, ToTuple>::type>(std::get<N>(tuple));
}

}  // namespace util

template <class CLASS, typename... ARGTYPES>
class Construct : public GenBase<CLASS> {
public:
    using ArgumentList = util::TypeList<ARGTYPES...>;
    using GenTuple = std::tuple<GenFunction<std::remove_reference_t<ARGTYPES>>...>;

    static constexpr auto Size = sizeof...(ARGTYPES);

    Construct() : genTup(util::createGenTuple(ArgumentList{})) {}

    Construct(GenTuple g) : genTup(g) {}

    Shrinkable<CLASS> operator()(Random& rand) override { return constructAccordingly(generateArgs(rand)); }

    template <typename U>
    Generator<U> map(std::function<U(CLASS&)> mapper)
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
                                                        std::forward<Criteria>(criteria)));
    }

    template <typename U>
    Generator<std::pair<CLASS, U>> pairWith(std::function<GenFunction<U>(const CLASS&)> gengen)
    {
        auto thisPtr = clone();
        return proptest::dependency<CLASS, U>([thisPtr](Random& rand) { return thisPtr->operator()(rand); }, gengen);
    }

    template <typename U>
    decltype(auto) tupleWith(std::function<GenFunction<U>(CLASS&)> gengen)
    {
        auto thisPtr = clone();
        return proptest::chain([thisPtr](Random& rand) { return thisPtr->operator()(rand); }, gengen);
    }

    template <typename U>
    Generator<U> flatmap(std::function<U(CLASS&)> gengen)
    {
        auto thisPtr = clone();
        return Generator<U>(
            proptest::derive<CLASS, U>([thisPtr](Random& rand) { return thisPtr->operator()(rand); }, gengen));
    }

    std::shared_ptr<Construct<CLASS, ARGTYPES...>> clone()
    {
        return std::make_shared<Construct<CLASS, ARGTYPES...>>(*dynamic_cast<Construct<CLASS, ARGTYPES...>*>(this));
    }

private:
    template <size_t... index>
    decltype(auto) generateArgsHelper(Random& rand, std::index_sequence<index...>)
    {
        return std::make_tuple(std::get<index>(genTup)(rand)...);
    }

    decltype(auto) generateArgs(Random& rand) { return generateArgsHelper(rand, std::make_index_sequence<Size>{}); }

    template <typename CastTuple, typename ValueTuple, size_t... index>
    static Shrinkable<CLASS> constructByTupleType(ValueTuple&& valueTuple, std::index_sequence<index...>)
    {
        return make_shrinkable<CLASS>(util::autoCastTuple<CastTuple, index>(std::forward<ValueTuple>(valueTuple))...);
    }

    template <typename ValueTuple>
    static Shrinkable<CLASS> constructAccordingly(ValueTuple&& valueTuple)
    {
        using ArgsAsTuple = std::tuple<ARGTYPES...>;
        constexpr auto arity = sizeof...(ARGTYPES);
        return constructByTupleType<ArgsAsTuple>(
            std::forward<ValueTuple>(valueTuple), std::make_index_sequence<arity>{}  // {0,1,2,3,...,N-1}
        );
    }

    GenTuple genTup;
};

/**
 * Generates a CLASS type by specifying target constructor's paremeter types
 *      struct Point {
 *          Point() : x(0), y(0) {}
 *          Point(int x, int y) : x(x), y(y) {}
 *          int x;
 *          int y;
 *      };
 *      GenFunction<Point> objectGen = construct<Point>(); // calls Point()
 *      GenFunction<Point> objectGen2 = construct<Point, int, int>(nonNegative(), nonNegative()); // calls Point(int,
 * int) GenFunction<Point> objectGen3 = construct<Point, int, int>(); // ints are generated using Arbi<int>
 */
template <typename CLASS, typename... ARGTYPES>
decltype(auto) construct()
{
    constexpr auto ImplicitSize = sizeof...(ARGTYPES);
    using ArgsAsTuple = std::tuple<std::decay_t<ARGTYPES>...>;
    auto implicits = util::createGenHelperListed<ArgsAsTuple>(std::make_index_sequence<ImplicitSize>{});
    return Construct<CLASS, ARGTYPES...>(implicits);
}

// some explicits
template <typename CLASS, typename... ARGTYPES, typename EXPGEN0, typename... EXPGENS>
Construct<CLASS, ARGTYPES...> construct(EXPGEN0&& gen0, EXPGENS&&... gens)
{
    constexpr auto ExplicitSize = sizeof...(EXPGENS) + 1;
    constexpr auto ImplicitSize = sizeof...(ARGTYPES) - ExplicitSize;
    using ArgsAsTuple = std::tuple<std::decay_t<ARGTYPES>...>;

    auto explicits = std::make_tuple(util::genToFunc(gen0), util::genToFunc(gens)...);
    auto implicits = util::createGenHelperListed<ArgsAsTuple>(
        util::addOffset<ExplicitSize>(std::make_index_sequence<ImplicitSize>{}));

    return Construct<CLASS, ARGTYPES...>(std::tuple_cat(explicits, implicits));
}

}  // namespace proptest
