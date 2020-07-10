#pragma once
#include <algorithm>

namespace proptest {
namespace util {

template <size_t N, typename Tuple>
decltype(auto) forwardNth(Tuple&& tup)
{
    return std::forward<typename std::tuple_element<N, Tuple>::type>(std::get<N>(std::forward<Tuple>(tup)));
}

template <typename Function, typename Tuple, std::size_t... index>
decltype(auto) transformTupleHelper(Function&& f, Tuple&& tup, std::index_sequence<index...>)
{
    return std::make_tuple(f(forwardNth<index, Tuple>(std::forward<Tuple>(tup)))...);
}

template <typename Tuple, typename Function>
decltype(auto) transformTuple(Tuple&& argTup, Function&& f)
{
    constexpr auto Size = std::tuple_size<Tuple>::value;

    return transformTupleHelper(std::forward<Function>(f), std::forward<Tuple>(argTup),
                                std::make_index_sequence<Size>{});
}

template <template <typename> class FunctionTemplate, typename Tuple, int N>
decltype(auto) callTransform(Tuple&& tup)
{
    return FunctionTemplate<typename std::tuple_element<N, Tuple>::type>::transform(
        std::forward<typename std::tuple_element<N, Tuple>::type>(std::get<N>(tup)));
}

template <template <typename> class FunctionTemplate, typename Tuple, std::size_t... index>
decltype(auto) transformHeteroTupleHelper(Tuple&& tup, std::index_sequence<index...>)
{
    return std::make_tuple(callTransform<FunctionTemplate, Tuple, index>(std::forward<Tuple>(tup))...);
}

template <template <typename> class FunctionTemplate, typename Tuple>
decltype(auto) transformHeteroTuple(Tuple&& argTup)
{
    constexpr auto Size = std::tuple_size<Tuple>::value;

    return transformHeteroTupleHelper<FunctionTemplate>(std::forward<Tuple>(argTup), std::make_index_sequence<Size>{});
}

template <template <typename> class FunctionTemplate, typename Tuple, typename Arg, int N>
decltype(auto) callTransformWithArg(Tuple&& tup, Arg&& arg)
{
    return FunctionTemplate<typename std::tuple_element<N, Tuple>::type>::transform(
        std::forward<typename std::tuple_element<N, Tuple>::type>(std::get<N>(tup)), std::forward<Arg>(arg));
}

template <template <typename> class FunctionTemplate, typename Tuple, typename Arg, std::size_t... index>
decltype(auto) transformHeteroTupleWithArgHelper(Tuple&& tup, Arg&& arg, std::index_sequence<index...>)
{
    return std::make_tuple(
        callTransformWithArg<FunctionTemplate, Tuple, Arg, index>(std::forward<Tuple>(tup), std::forward<Arg>(arg))...);
}

template <template <typename> class FunctionTemplate, typename Tuple, typename Arg>
decltype(auto) transformHeteroTupleWithArg(Tuple&& tup, Arg&& arg)
{
    constexpr auto Size = std::tuple_size<Tuple>::value;

    return transformHeteroTupleWithArgHelper<FunctionTemplate>(std::forward<Tuple>(tup), std::forward<Arg>(arg),
                                                               std::make_index_sequence<Size>{});
}

}  // namespace util
}  // namespace proptest
