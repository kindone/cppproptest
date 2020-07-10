#pragma once

#include <utility>
#include <tuple>
#include <type_traits>

namespace proptest {
namespace util {

template <typename Function, typename Tuple, std::size_t... index>
decltype(auto) invokeHelper(Function&& f, Tuple&& valueTup, std::index_sequence<index...>)
{
    return f(std::get<index>(std::forward<Tuple>(valueTup))...);
}

template <typename Function, typename ArgTuple>
decltype(auto) invokeWithArgTuple(Function&& f, ArgTuple&& argTup)
{
    constexpr auto Size = std::tuple_size<ArgTuple>::value;
    return invokeHelper(std::forward<Function>(f), std::forward<ArgTuple>(argTup), std::make_index_sequence<Size>{});
}

template <size_t N, size_t M, typename Tuple, typename Replace>
std::enable_if_t<N == M, Replace&&> ReplaceHelper(Tuple&&, Replace&& replace)
{
    return std::forward<Replace>(replace);
}

template <size_t N, size_t M, typename Tuple, typename Replace>
std::enable_if_t<N != M, std::tuple_element_t<M, Tuple>> ReplaceHelper(Tuple&& valueTup, Replace&&)
{
    return std::get<M>(std::forward<Tuple>(valueTup));
}

template <size_t N, typename Function, typename Tuple, typename Replace, std::size_t... index>
decltype(auto) invokeWithReplaceHelper(Function&& f, Tuple&& valueTup, Replace&& replace, std::index_sequence<index...>)
{
    /*static_assert(std::is_same<Replace,
        typename std::tuple_element<N,Tuple>::type >::value, "");*/
    return f(ReplaceHelper<N, index>(std::forward<Tuple>(valueTup), std::forward<Replace>(replace))...);
}

template <size_t N, typename Function, typename ArgTuple, typename Replace>
decltype(auto) invokeWithArgTupleWithReplace(Function&& f, ArgTuple&& argTup, Replace&& replace)
{
    constexpr auto Size = std::tuple_size<ArgTuple>::value;
    return invokeWithReplaceHelper<N>(std::forward<Function>(f), std::forward<decltype(argTup)>(argTup),
                                      std::forward<Replace>(replace), std::make_index_sequence<Size>{});
}

template <typename Function, typename... Args>
decltype(auto) invokeWithArgs(Function&& f, Args&&... args)
{
    auto argTup = std::make_tuple<Args...>(std::forward<Args>(args)...);
    constexpr auto Size = sizeof...(Args);
    return invokeHelper(std::forward<Function>(f), std::forward<decltype(argTup)>(argTup),
                        std::make_index_sequence<Size>{});
}

}  // namespace util
}  // namespace proptest
