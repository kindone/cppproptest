#ifndef TESTING_PROP_CONSTRUCT_HPP
#define TESTING_PROP_CONSTRUCT_HPP

#include "testing/gen.hpp"

namespace PropertyBasedTesting
{


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
    return autoCast<typename std::tuple_element<N, ToTuple>::type>(std::get<N>(tuple).value);
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

    Construct() : genTup(createGenTuple(ArgumentList{})) {
    }

    template <std::size_t... index>
    decltype(auto) generateArgsHelper(Random& rand, std::index_sequence<index...>) {
        return std::make_tuple(std::get<index>(genTup).generate(rand)...);
    }

    decltype(auto) generateArgs(Random& rand) {
        return generateArgsHelper(rand, std::make_index_sequence<Size>{});
    }

    Shrinkable<CLASS> generate(Random& rand) {
        auto argTup = generateArgs(rand);
        return Shrinkable<CLASS>{constructHelper<CLASS, ARGTYPES...>(argTup)};
    }
private:
    GenTuple genTup;
};



} // namespace PropertyBasedTesting

#endif // TESTING_PROP_CONSTRUCT_HPP

