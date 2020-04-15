#pragma once

#include "testing/invokeWithArgs.hpp"
#include "testing/tuple.hpp"
#include "testing/generator/util.hpp"

namespace PropertyBasedTesting {

template< typename Function, typename GenTuple, std::size_t... index>
decltype( auto ) invokeWithGenHelper(Random& rand, Function&& f, GenTuple&& genTup, std::index_sequence<index...> index_sequence) {

    auto valueTup = std::make_tuple(std::get<index>(genTup)(rand)...);
    auto valueTupPtr = std::make_shared<decltype(valueTup)>(valueTup);
    try {
        auto values = transformHeteroTuple<ShrinkableGet>(std::forward<decltype(valueTup)>(valueTup));
        return invokeWithArgTuple(std::forward<Function>(f), std::forward<decltype(values)>(values));
    }
    catch(const AssertFailed& e) {
        throw PropertyFailed<decltype(valueTup)>(e, valueTupPtr);
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
