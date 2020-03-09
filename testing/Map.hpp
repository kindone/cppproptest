#pragma once
#include <algorithm>

namespace PropertyBasedTesting {

template <typename Function, typename Tuple,  std::size_t... index>
decltype( auto ) mapTupleHelper(Function&& f, Tuple&& tup, std::index_sequence<index...> index_sequence) {
    return std::make_tuple(f(std::move(std::get<index>(tup)))...);
}

template <typename Tuple, typename Function >
decltype( auto ) mapTuple(Tuple&& argTup, Function&& f ) {
    constexpr auto Size = std::tuple_size<Tuple>::value;

    return mapTupleHelper(
        std::forward<Function>(f),
        std::forward<Tuple>( argTup ),
        std::make_index_sequence<Size>{}
    );
}


template <template <typename> class FunctionTemplate, typename Tuple, int N>
decltype(auto) callMap(Tuple&& tup) {
    return FunctionTemplate<typename std::tuple_element<N, Tuple>::type>::map(std::move(std::get<N>(tup)));
}

template<template <typename> class FunctionTemplate, typename Tuple,  std::size_t... index>
decltype( auto ) mapHeteroTupleHelper(Tuple&& tup, std::index_sequence<index...> index_sequence) {
    return std::make_tuple(callMap<FunctionTemplate, Tuple, index>(std::move(tup))...);
}

template<template <typename> class FunctionTemplate, typename Tuple>
decltype( auto ) mapHeteroTuple(Tuple&& argTup) {
    constexpr auto Size = std::tuple_size<Tuple>::value;

    return mapHeteroTupleHelper<FunctionTemplate>(
        std::forward<Tuple>( argTup ),
        std::make_index_sequence<Size>{}
    );

}

}
