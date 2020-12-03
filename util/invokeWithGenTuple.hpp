#pragma once

#include "invokeWithArgs.hpp"
#include "tuple.hpp"
#include "../generator/util.hpp"

namespace proptest {
namespace util {

template <typename Function, typename GenTuple, size_t... index>
decltype(auto) invokeWithGenHelper(Random& rand, Function&& f, GenTuple&& genTup, std::index_sequence<index...>)
{
    auto valueTup = std::make_tuple(std::get<index>(genTup)(rand)...);
    auto values = transformHeteroTuple<ShrinkableGet>(std::forward<decltype(valueTup)>(valueTup));
    try {
        return invokeWithArgTuple(std::forward<Function>(f), std::forward<decltype(values)>(values));
    } catch (const AssertFailed& e) {
        throw PropertyFailed<decltype(valueTup)>(e);
    }
}

template <typename Function, typename Tuple>
decltype(auto) invokeWithGenTuple(Random& rand, Function&& f, Tuple&& genTup)
{
    constexpr auto Arity = function_traits<std::remove_reference_t<decltype(f)> >::arity;
    return invokeWithGenHelper(rand, std::forward<Function>(f), std::forward<Tuple>(genTup),
                               std::make_index_sequence<Arity>{});
}

template <typename TUP1, typename TUP2, size_t N>
int setTupleFromTuple(TUP1&& tup1, TUP2&& tup2)
{
    std::get<N>(tup1) = std::get<N>(tup2);
    return 0;
}

template <typename TUP1, typename TUP2, size_t... index>
void overrideTupleHelper(TUP1&& tup1, TUP2&& tup2, std::index_sequence<index...>)
{
    std::make_tuple(setTupleFromTuple<TUP1, TUP2, index>(std::forward<TUP1>(tup1), std::forward<TUP2>(tup2))...);
}

template <typename... Ts, typename ARG0, typename... ARGS>
std::tuple<Ts...> overrideTuple(const std::tuple<Ts...>& tup, ARG0&& arg0, ARGS&&... args)
{
    constexpr auto Size = 1 + sizeof...(ARGS);
    auto argTup = std::make_tuple<ARG0, ARGS...>(std::forward<ARG0>(arg0), std::forward<ARGS>(args)...);
    std::tuple<Ts...> copy = tup;

    overrideTupleHelper(std::forward<decltype(copy)>(copy), std::forward<decltype(argTup)>(argTup),
                        std::make_index_sequence<Size>{});
    return copy;
}

template <typename... Ts>
std::tuple<Ts...> overrideTuple(const std::tuple<Ts...>& tup)
{
    return tup;
}

}  // namespace util
}  // namespace proptest
