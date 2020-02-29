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
