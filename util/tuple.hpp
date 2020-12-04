#pragma once
#include "std.hpp"

namespace proptest {
namespace util {

template <size_t N, typename Tuple>
decltype(auto) forwardNth(Tuple&& tup)
{
    return forward<typename tuple_element<N, Tuple>::type>(get<N>(forward<Tuple>(tup)));
}

template <typename Function, typename Tuple, size_t... index>
decltype(auto) transformTupleHelper(Function&& f, Tuple&& tup, index_sequence<index...>)
{
    return make_tuple(f(forwardNth<index, Tuple>(forward<Tuple>(tup)))...);
}

template <typename Tuple, typename Function>
decltype(auto) transformTuple(Tuple&& argTup, Function&& f)
{
    constexpr auto Size = tuple_size<Tuple>::value;

    return transformTupleHelper(forward<Function>(f), forward<Tuple>(argTup),
                                make_index_sequence<Size>{});
}

template <template <typename> class FunctionTemplate, typename Tuple, int N>
decltype(auto) callTransform(Tuple&& tup)
{
    return FunctionTemplate<typename tuple_element<N, Tuple>::type>::transform(
        forward<typename tuple_element<N, Tuple>::type>(get<N>(tup)));
}

template <template <typename> class FunctionTemplate, typename Tuple, size_t... index>
decltype(auto) transformHeteroTupleHelper(Tuple&& tup, index_sequence<index...>)
{
    return make_tuple(callTransform<FunctionTemplate, Tuple, index>(forward<Tuple>(tup))...);
}

template <template <typename> class FunctionTemplate, typename Tuple>
decltype(auto) transformHeteroTuple(Tuple&& argTup)
{
    constexpr auto Size = tuple_size<Tuple>::value;

    return transformHeteroTupleHelper<FunctionTemplate>(forward<Tuple>(argTup), make_index_sequence<Size>{});
}

template <template <typename> class FunctionTemplate, typename Tuple, typename Arg, int N>
decltype(auto) callTransformWithArg(Tuple&& tup, Arg&& arg)
{
    return FunctionTemplate<typename tuple_element<N, Tuple>::type>::transform(
        forward<typename tuple_element<N, Tuple>::type>(get<N>(tup)), forward<Arg>(arg));
}

template <template <typename> class FunctionTemplate, typename Tuple, typename Arg, size_t... index>
decltype(auto) transformHeteroTupleWithArgHelper(Tuple&& tup, Arg&& arg, index_sequence<index...>)
{
    return make_tuple(
        callTransformWithArg<FunctionTemplate, Tuple, Arg, index>(forward<Tuple>(tup), forward<Arg>(arg))...);
}

template <template <typename> class FunctionTemplate, typename Tuple, typename Arg>
decltype(auto) transformHeteroTupleWithArg(Tuple&& tup, Arg&& arg)
{
    constexpr auto Size = tuple_size<Tuple>::value;

    return transformHeteroTupleWithArgHelper<FunctionTemplate>(forward<Tuple>(tup), forward<Arg>(arg),
                                                               make_index_sequence<Size>{});
}

}  // namespace util
}  // namespace proptest
