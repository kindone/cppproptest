#pragma once

#include "std.hpp"
#include "typelist.hpp"

namespace proptest {

class Random;

namespace util {

template <size_t O, size_t... Is>
index_sequence<(O + Is)...> addOffset(index_sequence<Is...>)
{
    return {};
}

template <typename GEN>
decltype(auto) genToFunc(GEN&& gen)
{
    using retType = decltype(gen(declval<Random&>()));
    return static_cast<function<retType(Random&)>>(gen);
}

template <typename Tuple, size_t... index>
decltype(auto) createGenHelperListed(index_sequence<index...>)
{
    return util::make_tuple(genToFunc(Arbi<tuple_element_t<index, Tuple>>())...);
}

// returns a Tuple<Arbi<ARGS...>>
template <typename... ARGS>
tuple<GenFunction<decay_t<ARGS>>...> createGenTuple(TypeList<ARGS...> /*argument_list*/)
{
    using ArgsAsTuple = tuple<decay_t<ARGS>...>;
    constexpr auto Size = tuple_size<ArgsAsTuple>::value;
    return createGenHelperListed<ArgsAsTuple>(make_index_sequence<Size>{});
}

template <typename... ARGS, typename... EXPGENS>
enable_if_t<(sizeof...(EXPGENS) > 0 && sizeof...(EXPGENS) == sizeof...(ARGS)),
                 tuple<decay_t<EXPGENS>...>>
createGenTuple(TypeList<ARGS...>, EXPGENS&&... gens)
{
    // constexpr auto ExplicitSize = sizeof...(EXPGENS);
    tuple<decay_t<EXPGENS>...> explicits = util::make_tuple(gens...);
    return explicits;
}

template <typename... ARGS, typename... EXPGENS>
enable_if_t<(sizeof...(EXPGENS) > 0 && sizeof...(EXPGENS) < sizeof...(ARGS)),
                 tuple<GenFunction<decay_t<ARGS>>...>>
createGenTuple(TypeList<ARGS...>, EXPGENS&&... gens)
{
    constexpr auto ExplicitSize = sizeof...(EXPGENS);
    constexpr auto ImplicitSize = sizeof...(ARGS) - ExplicitSize;
    auto explicits = util::make_tuple(gens...);
    using ArgsAsTuple = tuple<decay_t<ARGS>...>;
    auto implicits =
        createGenHelperListed<ArgsAsTuple>(addOffset<ExplicitSize>(make_index_sequence<ImplicitSize>{}));

    return tuple_cat(explicits, implicits);
}

}  // namespace util

}  // namespace proptest
