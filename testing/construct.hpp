#ifndef TESTING_PROP_CONSTRUCT_HPP
#define TESTING_PROP_CONSTRUCT_HPP

#include "testing/gen.hpp"

namespace PropertyBasedTesting
{

class Random;

template <typename CLASS, typename ... ARGS>
auto forwardHelper(ARGS&& ...args) {
    return CLASS(std::move(args)...);
}

template< typename T, typename GenTuple, std::size_t... index>
decltype( auto ) constructHelper(Random& rand, GenTuple&& genTup, std::index_sequence<index...> ) {
    return forwardHelper<T>(std::get<index>(genTup).generate(rand)...);
}


template <typename CLASS, typename ... GENS>
decltype(auto) construct(Random& rand) {
    auto genTup = createGenTuple<GENS...>();
    using ArgsAsTuple = std::tuple<std::remove_reference_t<GENS>...>;
    constexpr auto Arity = std::tuple_size<ArgsAsTuple>::value;
    return constructHelper<CLASS>(
        rand,
        std::forward<decltype(genTup)>(genTup),
        std::make_index_sequence<Arity>{}
    );
}


} // namespace PropertyBasedTesting

#endif // TESTING_PROP_CONSTRUCT_HPP

