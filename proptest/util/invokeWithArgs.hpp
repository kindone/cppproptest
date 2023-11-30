#pragma once

#include "std.hpp"

namespace proptest {
namespace util {

template <typename Function, typename Tuple, size_t... index>
decltype(auto) invokeHelper(Function&& f, Tuple&& valueTup, index_sequence<index...>)
{
    return f(get<index>(util::forward<Tuple>(valueTup))...);
}

template <typename Function, typename ArgTuple>
decltype(auto) invokeWithArgTuple(Function&& f, ArgTuple&& argTup)
{
    constexpr auto Size = tuple_size<ArgTuple>::value;
    return invokeHelper(util::forward<Function>(f), util::forward<ArgTuple>(argTup), make_index_sequence<Size>{});
}

template <size_t N, size_t M, typename Tuple, typename Replace>
    requires (N == M)
Replace&& ReplaceHelper(Tuple&&, Replace&& replace)
{
    return util::forward<Replace>(replace);
}

template <size_t N, size_t M, typename Tuple, typename Replace>
    requires (N != M)
tuple_element_t<M, Tuple> ReplaceHelper(Tuple&& valueTup, Replace&&)
{
    return get<M>(util::forward<Tuple>(valueTup));
}

template <size_t N, typename Function, typename Tuple, typename Replace, size_t... index>
bool invokeWithReplaceHelper(Function&& f, Tuple&& valueTup, Replace&& replace, index_sequence<index...>)
{
    /*static_assert(is_same<Replace,
        typename tuple_element<N,Tuple>::type >::value, "");*/
    return f(ReplaceHelper<N, index>(util::forward<Tuple>(valueTup), util::forward<Replace>(replace))...);
}

template <size_t N, typename Function, typename ArgTuple, typename Replace>
bool invokeWithArgTupleWithReplace(Function&& f, ArgTuple&& argTup, Replace&& replace)
{
    constexpr auto Size = tuple_size<ArgTuple>::value;
    return invokeWithReplaceHelper<N>(util::forward<Function>(f), util::forward<decltype(argTup)>(argTup),
                                      util::forward<Replace>(replace), make_index_sequence<Size>{});
}

template <typename Function, typename... Args>
decltype(auto) invokeWithArgs(Function&& f, const tuple<Args...>& argTup)
{
    constexpr auto Size = sizeof...(Args);
    return invokeHelper(util::forward<Function>(f), util::forward<decltype(argTup)>(argTup),
                        make_index_sequence<Size>{});
}

}  // namespace util
}  // namespace proptest
