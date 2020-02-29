#pragma once

namespace PropertyBasedTesting {

/*
template <typename Tuple, typename... I>
struct remove_reference_from_tuple_helper;

template <typename Tuple, std::size_t... index>
struct remove_reference_from_tuple_helper<Tuple, typename std::index_sequence<index...>> {
    using type = std::tuple<std::remove_reference_t<typename std::tuple_element<index, Tuple>::type>...>;
};

template <typename Tuple>
struct remove_reference_from_tuple {
    static constexpr auto Size = std::tuple_size<Tuple>::value;
    static constexpr auto Sequence = std::make_index_sequence<Size>{};
    using type = typename remove_reference_from_tuple_helper<Tuple, Sequence>::type;
};
**/

};
