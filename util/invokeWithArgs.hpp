#pragma once

#include "std.hpp"

namespace proptest {
namespace util {

template <typename Function, typename Tuple, size_t... index>
decltype(auto) invokeHelper(Function&& f, Tuple&& valueTup, index_sequence<index...>)
{
    return f(get<index>(forward<Tuple>(valueTup))...);
}

template <typename Function, typename ArgTuple>
decltype(auto) invokeWithArgTuple(Function&& f, ArgTuple&& argTup)
{
    constexpr auto Size = tuple_size<ArgTuple>::value;
    return invokeHelper(forward<Function>(f), forward<ArgTuple>(argTup), make_index_sequence<Size>{});
}

template <size_t N, size_t M, typename Tuple, typename Replace>
enable_if_t<N == M, Replace&&> ReplaceHelper(Tuple&&, Replace&& replace)
{
    return forward<Replace>(replace);
}

template <size_t N, size_t M, typename Tuple, typename Replace>
enable_if_t<N != M, tuple_element_t<M, Tuple>> ReplaceHelper(Tuple&& valueTup, Replace&&)
{
    return get<M>(forward<Tuple>(valueTup));
}

template <size_t N, typename Function, typename Tuple, typename Replace, size_t... index>
decltype(auto) invokeWithReplaceHelper(Function&& f, Tuple&& valueTup, Replace&& replace, index_sequence<index...>)
{
    /*static_assert(is_same<Replace,
        typename tuple_element<N,Tuple>::type >::value, "");*/
    return f(ReplaceHelper<N, index>(forward<Tuple>(valueTup), forward<Replace>(replace))...);
}

template <size_t N, typename Function, typename ArgTuple, typename Replace>
decltype(auto) invokeWithArgTupleWithReplace(Function&& f, ArgTuple&& argTup, Replace&& replace)
{
    constexpr auto Size = tuple_size<ArgTuple>::value;
    return invokeWithReplaceHelper<N>(forward<Function>(f), forward<decltype(argTup)>(argTup),
                                      forward<Replace>(replace), make_index_sequence<Size>{});
}

template <typename Function, typename... Args>
decltype(auto) invokeWithArgs(Function&& f, const tuple<Args...>& argTup)
{
    constexpr auto Size = sizeof...(Args);
    return invokeHelper(forward<Function>(f), forward<decltype(argTup)>(argTup),
                        make_index_sequence<Size>{});
}

}  // namespace util
}  // namespace proptest
