#pragma once
#include "testing/gen.hpp"

namespace PropertyBasedTesting
{

template <typename TO, typename SHRINKABLE, std::enable_if_t<!std::is_lvalue_reference<TO>::value, bool> = false>
decltype(auto) autoCast(SHRINKABLE&& shr) {
    return shr.get();
}

template <typename TO, typename SHRINKABLE, std::enable_if_t<std::is_pointer<TO>::value, bool> = false>
decltype(auto) autoCast(SHRINKABLE&& shr) {
    return shr.getPtr();
}

template <typename TO, typename SHRINKABLE, std::enable_if_t<std::is_lvalue_reference<TO>::value, bool> = true>
decltype(auto) autoCast(SHRINKABLE&& shr) {
    return shr.getRef();
}

template <typename ToTuple, std::size_t N, typename FromTuple>
decltype(auto) autoCastTuple(FromTuple&& tuple) {
    return autoCast<typename std::tuple_element<N, ToTuple>::type>(std::get<N>(tuple));
}

template<typename Constructible, typename CastTuple, typename ValueTuple, std::size_t... index>
decltype( auto ) constructByTupleType(ValueTuple&& valueTuple, std::index_sequence<index...> ) {
    return make_shrinkable<Constructible>(autoCastTuple<CastTuple,index>(std::forward<ValueTuple>(valueTuple))...);
}

template<typename Constructible, typename ... ARGS, typename ValueTuple >
decltype( auto ) constructAccordingly(ValueTuple&& valueTuple) {
    using ArgsAsTuple = std::tuple<ARGS...>;
    constexpr auto arity = sizeof...(ARGS);
    return constructByTupleType<Constructible, ArgsAsTuple>(
        std::forward<ValueTuple>(valueTuple),
        std::make_index_sequence<arity>{} // {0,1,2,3,...,N-1}
    );
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
        return std::make_tuple(std::get<index>(genTup)(rand)...);
    }

    decltype(auto) generateArgs(Random& rand) {
        return generateArgsHelper(rand, std::make_index_sequence<Size>{});
    }

    Shrinkable<CLASS> operator()(Random& rand) {
        return constructAccordingly<CLASS, ARGTYPES...>(generateArgs(rand));
    }
private:
    GenTuple genTup;
};



} // namespace PropertyBasedTesting


