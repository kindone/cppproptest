#ifndef TESTING_GEN_HPP
#define TESTING_GEN_HPP
#include <functional>
#include <tuple>
#include "testing/function_traits.hpp"
#include "testing/api.hpp"
#include "testing/Random.hpp"
#include "testing/typelist.hpp"


namespace PropertyBasedTesting
{
    class Random;

template <typename T>
struct Gen
{
    using type = T;
    Gen() {
    }

    T generate(Random& rand);
};

template <typename T, typename ElemGen = void>
struct Arbitrary : public Gen<T>{
};

template< typename Function, typename GenTuple, std::size_t... index>
decltype( auto ) invokeWithGenHelper(Random& rand, Function&& f, GenTuple&& genTup, std::index_sequence<index...> ) {
    return f( std::get<index>(genTup).generate(rand)... );
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



} // namespace PropertyBasedTesting


#endif // TESTING_GEN_HPP
