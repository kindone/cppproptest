#pragma once

#include <cstddef>
#include <type_traits>
#include <utility>
#include <functional>

namespace PropertyBasedTesting {

class Random;

namespace util {

template <std::size_t O, std::size_t ... Is>
std::index_sequence<(O + Is)...> addOffset(std::index_sequence<Is...>)
{
    return {};
}

template <typename GEN>
decltype(auto) genToFunc(GEN&& gen) {
    using retType = decltype(gen(std::declval<Random&>()));
    return static_cast<std::function<retType(Random&)>>(gen);
}

template<typename Tuple, std::size_t... index>
decltype( auto ) createGenHelperListed(std::index_sequence<index...> ) {
    return std::make_tuple(genToFunc(Arbitrary< itemAt<Tuple,index> >())...);
}

template<typename ... ARGS, std::size_t... index>
decltype( auto ) createGenHelperPacked(std::index_sequence<index...> ) {
    return std::make_tuple(ARGS()...);
}

// returns a std::Tuple<Arbitrary<ARGS...>>
template<typename ... ARGS >
decltype( auto ) createGenTuple(TypeList<ARGS...> argument_list ) {
    using ArgsAsTuple = std::tuple<std::decay_t<ARGS>...>;
    constexpr auto Size = std::tuple_size<ArgsAsTuple>::value;
    return createGenHelperListed<ArgsAsTuple>(
        std::make_index_sequence<Size>{}
    );
}


template<typename ... ARGS, typename ... EXPARGS, typename std::enable_if<(sizeof...(EXPARGS) > 0 && sizeof...(EXPARGS) == sizeof...(ARGS)), bool>::type = true >
decltype( auto ) createGenTuple(TypeList<ARGS...> fullArgTypes, EXPARGS&&... gens) {
    constexpr auto ExplicitSize = sizeof...(EXPARGS);
    auto explicits = std::make_tuple(gens...);
    return explicits;
}

template<typename ... ARGS, typename ... EXPARGS, typename std::enable_if<(sizeof...(EXPARGS) > 0 && sizeof...(EXPARGS) < sizeof...(ARGS)), bool>::type = true >
decltype( auto ) createGenTuple(TypeList<ARGS...> fullArgTypes, EXPARGS&&... gens) {
    constexpr auto ExplicitSize = sizeof...(EXPARGS);
    constexpr auto ImplicitSize = sizeof...(ARGS) - ExplicitSize;
    auto explicits = std::make_tuple(gens...);
    using ArgsAsTuple = std::tuple<std::decay_t<ARGS>...>;
    auto implicits = createGenHelperListed<ArgsAsTuple>(
        addOffset<ExplicitSize>(std::make_index_sequence<ImplicitSize>{})
    );

    return std::tuple_cat(explicits, implicits);
}

} // namespace util

} // namespace PropertyBasedTesting
