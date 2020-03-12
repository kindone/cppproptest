#pragma once

namespace PropertyBasedTesting {

template<typename Function, typename Tuple, std::size_t...index>
decltype(auto) invokeHelper(Function&& f, Tuple&& valueTup, std::index_sequence<index...>) {
    return f(std::get<index>(valueTup)...);
}

template< typename Function, typename ArgTuple>
decltype(auto) invokeWithArgTuple(Function&& f, ArgTuple&& argTup) {
    constexpr auto Size = std::tuple_size<ArgTuple>::value;
    return invokeHelper(
            std::move(f),
            std::move(argTup),
            std::make_index_sequence<Size>{});
}


template<size_t N, typename Function, typename Tuple, typename Replace, std::size_t...index>
decltype(auto) invokeWithReplaceHelper(Function&& f, Tuple&& valueTup, Replace&& replace, std::index_sequence<index...>) {
    return f((index == N ? std::get<index>(valueTup) : replace)...);
}

template <size_t N, typename Function, typename ArgTuple, typename Replace>
decltype(auto) invokeWithArgTupleWithReplace(Function&& f, ArgTuple&& argTup, Replace&& replace) {
    constexpr auto Size = std::tuple_size<ArgTuple>::value;
    return invokeWithReplaceHelper<N>(
            std::move(f),
            std::move(argTup),
            std::move(replace),
            std::make_index_sequence<Size>{});
}


template< typename Function, typename ...Args>
decltype(auto) invokeWithArgs(Function&& f, Args ... args) {
    auto argTup = std::make_tuple<Args...>(std::move(args)...);
    constexpr auto Size = sizeof...(Args);
    return invokeHelper(
            std::move(f),
            std::move(argTup),
            std::make_index_sequence<Size>{});
}

}
