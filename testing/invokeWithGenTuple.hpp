#pragma once

#include "testing/invokeWithArgs.hpp"

namespace PropertyBasedTesting {

template< typename Function, typename GenTuple, std::size_t... index>
decltype( auto ) invokeWithGenHelper(Random& rand, Function&& f, GenTuple&& genTup, std::index_sequence<index...> index_sequence) {

    auto valueTup = std::make_tuple(std::get<index>(genTup)(rand)...); 
    try {
        return invokeWithArgTuple(std::move(f), std::move(valueTup));
    }
    catch(const AssertFailed& e) {
        throw PropertyFailed<decltype(valueTup)>(e, std::move(valueTup));
    }
}

template< typename Function, typename Tuple >
decltype( auto ) invokeWithGenTuple(Random& rand, Function&& f, Tuple&& genTup ) {
    constexpr auto Arity = function_traits< std::remove_reference_t<decltype(f)> >::arity;
    return invokeWithGenHelper(
        rand,
        std::forward<Function>(f),
        std::forward<Tuple>( genTup ),
        std::make_index_sequence<Arity>{}
    );
}

}
