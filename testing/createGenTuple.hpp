#pragma once

#include <cstddef>

namespace PropertyBasedTesting {

template<typename Tuple, std::size_t... index>
decltype( auto ) createGenHelperListed(std::index_sequence<index...> ) {
    return std::make_tuple(Arbitrary< itemAt<Tuple,index> >()...);
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


template<typename ... IMPARGS, typename ... EXPARGS, typename std::enable_if<(sizeof...(EXPARGS) > 0 && sizeof...(EXPARGS) == sizeof...(IMPARGS)), bool>::type = true >
decltype( auto ) createGenTuple(TypeList<IMPARGS...> fullArgTypes, EXPARGS&&... gens) {
    constexpr auto ExplicitSize = sizeof...(EXPARGS);
    auto explicits = std::make_tuple(gens...);
    return explicits;
}

template<typename ... IMPARGS, typename ... EXPARGS, typename std::enable_if<(sizeof...(EXPARGS) > 0 && sizeof...(EXPARGS) < sizeof...(IMPARGS)), bool>::type = true >
decltype( auto ) createGenTuple(TypeList<IMPARGS...> fullArgTypes, EXPARGS&&... gens) {
    constexpr auto ExplicitSize = sizeof...(EXPARGS);
    constexpr auto ImplicitSize = sizeof...(IMPARGS) - ExplicitSize;
    auto explicits = std::make_tuple(gens...);
    using ArgsAsTuple = std::tuple<std::decay_t<IMPARGS>...>;
    auto implicits = createGenHelperListed<ArgsAsTuple>(
        std::make_index_sequence<ImplicitSize>{}
    );

    return std::tuple_cat(explicits, implicits);
}

}
