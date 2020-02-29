#ifndef TESTING_PROP_CONSTRUCT_HPP
#define TESTING_PROP_CONSTRUCT_HPP

#include "testing/gen.hpp"

namespace PropertyBasedTesting
{

/*
// generateByTupleType<std::tuple>(make_index_sequence<N>{})
template<typename Tuple, std::size_t... index>
decltype( auto ) generateByTupleType(std::index_sequence<index...> ) {
    return std::make_tuple(Arbitrary<itemAt<Tuple,index> >::generate()...);
}

// ArgsToTuple
// TupleToArgs?
// ApplyTuple

// generateByTypeList(argument_type_list)
template<typename ... ARGS >
decltype( auto ) generateByTypeList(TypeList<ARGS...>) {
    using ArgsAsTuple = std::tuple<ARGS...>;
    constexpr auto arity = sizeof...(ARGS);
    return generateByTupleType<ArgsAsTuple>(
        std::make_index_sequence<arity>{} // {0,1,2,3,...,N-1}
    );
}

template<typename ... ARGS >
decltype( auto ) generateByTypes() {
    using ArgsAsTuple = std::tuple<ARGS...>;
    constexpr auto arity = sizeof...(ARGS);
    return generateByTupleType<ArgsAsTuple>(
        std::make_index_sequence<arity>{} // {0,1,2,3,...,N-1}
    );
}

template <typename Callable>
auto generateByParamList(Callable&&) {
    typename function_traits<Callable>::argument_type_list argument_type_list;
    return generateByTypeList(argument_type_list);
}

*/

template <typename TO, typename FROM, std::enable_if_t<!std::is_lvalue_reference<TO>::value, bool> = false>
decltype(auto) autoCast(FROM&& f) {
    return std::move(f);
}

template <typename TO, typename FROM, std::enable_if_t<std::is_pointer<TO>::value, bool> = false>
decltype(auto) autoCast(FROM& f) {
    return &f;
}

template <typename TO, typename FROM, std::enable_if_t<std::is_lvalue_reference<TO>::value, bool> = true>
decltype(auto) autoCast(FROM&& f) {
    return f;
}

template <typename ToTuple, std::size_t N, typename FromTuple>
decltype(auto) autoCastTuple(FromTuple&& tuple) {
    return autoCast<typename std::tuple_element<N, ToTuple>::type>(std::get<N>(tuple));
}

template<typename Constructible, typename CastTuple, typename ValueTuple, std::size_t... index>
decltype( auto ) constructByTupleType(ValueTuple&& valueTuple, std::index_sequence<index...> ) {
    return Constructible(autoCastTuple<CastTuple,index>(valueTuple)...);
}

template<typename Constructible, typename ... ARGS, typename ValueTuple >
decltype( auto ) constructAccordingly(ValueTuple&& valueTuple) {
    using ArgsAsTuple = std::tuple<ARGS...>;
    constexpr auto arity = sizeof...(ARGS);
    return constructByTupleType<Constructible, ArgsAsTuple>(
        valueTuple,
        std::make_index_sequence<arity>{} // {0,1,2,3,...,N-1}
    );
}

/*
template <typename CLASS, typename ... ARGS>
auto forwardHelper(ARGS&& ...args) {
    return CLASS(std::move(args)...);
}

template< typename T, typename GenTuple, std::size_t... index>
decltype( auto ) constructHelper(Random& rand, GenTuple&& genTup, std::index_sequence<index...> ) {
    return forwardHelper<T>(std::get<index>(genTup).generate(rand)...);
}
*/


template <typename Constructible, typename ... ARGS, typename ValueTuple>
decltype(auto) constructHelper(ValueTuple&& valueTuple) {
    return constructAccordingly<Constructible, ARGS...>(valueTuple);
}



template <class CLASS, typename ...ARGTYPES>
class Construct : public Gen<CLASS>
{
public:
    using ArgumentList = TypeList<ARGTYPES...>;
    using GenTuple = std::tuple<Arbitrary<std::remove_reference_t<ARGTYPES>>...>;

    static constexpr auto Size = sizeof...(ARGTYPES);
    static ArgumentList argument_list;

    Construct() : genTup(createGenTuple(argument_list)) {
    }

    template <std::size_t... index>
    decltype(auto) generateArgsHelper(Random& rand, std::index_sequence<index...>) {
        return std::make_tuple(std::get<index>(genTup).generate(rand)...);
    }

    decltype(auto) generateArgs(Random& rand) {
        return generateArgsHelper(rand, std::make_index_sequence<Size>{});
    }

    CLASS generate(Random& rand) {
        auto argTup = generateArgs(rand);
        return constructHelper<CLASS, ARGTYPES...>(argTup);
    }
private:
    GenTuple genTup;
};



} // namespace PropertyBasedTesting

#endif // TESTING_PROP_CONSTRUCT_HPP

