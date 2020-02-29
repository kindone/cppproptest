#pragma once

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
    using ArgsAsTuple = std::tuple<std::remove_reference_t<ARGS>...>;
    constexpr auto Size = std::tuple_size<ArgsAsTuple>::value;
    return createGenHelperListed<ArgsAsTuple>(
        std::make_index_sequence<Size>{}
    );
}

template<typename ... ARGS >
decltype( auto ) createGenTuple() {
    constexpr auto Size = sizeof...(ARGS);
    return createGenHelperPacked<ARGS...>(
        std::make_index_sequence<Size>{}
    );
}

}
