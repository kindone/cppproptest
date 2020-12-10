#pragma once

#include "invokeWithArgs.hpp"
#include "tuple.hpp"
#include "../generator/util.hpp"

namespace proptest {
namespace util {

template <typename Function, typename GenTuple, size_t... index>
decltype(auto) invokeWithGenHelper(Random& rand, Function&& f, GenTuple&& genTup, index_sequence<index...>)
{
    auto valueTup = util::make_tuple(get<index>(genTup)(rand)...);
    auto values = transformHeteroTuple<ShrinkableGet>(util::forward<decltype(valueTup)>(valueTup));
    try {
        return invokeWithArgTuple(util::forward<Function>(f), util::forward<decltype(values)>(values));
    } catch (const AssertFailed& e) {
        throw PropertyFailed<decltype(valueTup)>(e);
    }
}

template <typename Function, typename Tuple>
decltype(auto) invokeWithGenTuple(Random& rand, Function&& f, Tuple&& genTup)
{
    constexpr auto Arity = function_traits<remove_reference_t<decltype(f)> >::arity;
    return invokeWithGenHelper(rand, util::forward<Function>(f), util::forward<Tuple>(genTup),
                               make_index_sequence<Arity>{});
}

template <typename TUP1, typename TUP2, size_t N>
int setTupleFromTuple(TUP1&& tup1, TUP2&& tup2)
{
    get<N>(tup1) = get<N>(tup2);
    return 0;
}

template <typename TUP1, typename TUP2, size_t... index>
void overrideTupleHelper(TUP1&& tup1, TUP2&& tup2, index_sequence<index...>)
{
    util::make_tuple(setTupleFromTuple<TUP1, TUP2, index>(util::forward<TUP1>(tup1), util::forward<TUP2>(tup2))...);
}

template <typename... Ts, typename ARG0, typename... ARGS>
tuple<Ts...> overrideTuple(const tuple<Ts...>& tup, ARG0&& arg0, ARGS&&... args)
{
    constexpr auto Size = 1 + sizeof...(ARGS);
    auto argTup = util::make_tuple<ARG0, ARGS...>(util::forward<ARG0>(arg0), util::forward<ARGS>(args)...);
    tuple<Ts...> copy = tup;

    overrideTupleHelper(util::forward<decltype(copy)>(copy), util::forward<decltype(argTup)>(argTup),
                        make_index_sequence<Size>{});
    return copy;
}

template <typename... Ts>
tuple<Ts...> overrideTuple(const tuple<Ts...>& tup)
{
    return tup;
}

}  // namespace util
}  // namespace proptest
