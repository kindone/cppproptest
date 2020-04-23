#pragma once

#include "invokeWithArgs.hpp"
#include "tuple.hpp"
#include "generator/util.hpp"

namespace PropertyBasedTesting {
namespace util {

template< typename Function, typename GenTuple, std::size_t... index>
decltype( auto ) invokeWithGenHelper(Random& rand, Function&& f, GenTuple&& genTup, std::index_sequence<index...> index_sequence) {

    auto valueTup = std::make_tuple(std::get<index>(genTup)(rand)...);
    // FIXME: more efficient way than copying in to shared?
    auto valueTupPtr = std::make_shared<decltype(valueTup)>(std::move(valueTup));
    auto values = transformHeteroTuple<ShrinkableGet>(std::forward<decltype(valueTup)>(valueTup));
    try {
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

} // namespace util
} // namespace PropertyBasedTesting