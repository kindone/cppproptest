#pragma once
#include "testing/gen.hpp"
#include <type_traits>

namespace PropertyBasedTesting
{

class Random;

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

template <class CLASS, typename ...ARGTYPES>
class Construct : public Gen<CLASS>
{
public:
    using ArgumentList = TypeList<ARGTYPES...>;
    using GenTuple = std::tuple<std::function<Shrinkable<std::remove_reference_t<ARGTYPES>>(Random&)>...>;

    static constexpr auto Size = sizeof...(ARGTYPES);

    Construct() : genTup(createGenTuple(ArgumentList{})) {
    }

    Construct(GenTuple g) : genTup(g) {
    }

    template <std::size_t... index>
    decltype(auto) generateArgsHelper(Random& rand, std::index_sequence<index...>) {
        return std::make_tuple(std::get<index>(genTup)(rand)...);
    }

    decltype(auto) generateArgs(Random& rand) {
        return generateArgsHelper(rand, std::make_index_sequence<Size>{});
    }

    Shrinkable<CLASS> operator()(Random& rand) {
        return constructAccordingly(generateArgs(rand));
    }
private:

    template<typename CastTuple, typename ValueTuple, std::size_t... index>
    static Shrinkable<CLASS> constructByTupleType(ValueTuple&& valueTuple, std::index_sequence<index...> ) {
        return make_shrinkable<CLASS>(autoCastTuple<CastTuple,index>(std::forward<ValueTuple>(valueTuple))...);
    }

    template<typename ValueTuple>
    static Shrinkable<CLASS> constructAccordingly(ValueTuple&& valueTuple) {
        using ArgsAsTuple = std::tuple<ARGTYPES...>;
        constexpr auto arity = sizeof...(ARGTYPES);
        return constructByTupleType<ArgsAsTuple>(
            std::forward<ValueTuple>(valueTuple),
            std::make_index_sequence<arity>{} // {0,1,2,3,...,N-1}
        );
    }

    GenTuple genTup;
};

// all implicits
template <typename CLASS, typename ... ARGTYPES>
decltype(auto) construct() {
    constexpr auto ImplicitSize = sizeof...(ARGTYPES);
    using ArgsAsTuple = std::tuple<std::decay_t<ARGTYPES>...>;
    auto implicits = createGenHelperListed<ArgsAsTuple>(
        std::make_index_sequence<ImplicitSize>{}
    );
    return Construct<CLASS, ARGTYPES...>(implicits);
}

// all explicits
template <typename CLASS, typename ... ARGTYPES, typename ... EXPGENS, typename std::enable_if<(sizeof...(EXPGENS) > 0 && sizeof...(EXPGENS) == sizeof...(ARGTYPES)), bool>::type = true >
decltype(auto) construct(EXPGENS&&... gens) {
    constexpr auto ExplicitSize = sizeof...(EXPGENS);
    auto explicits = std::make_tuple(gens...);
    return Construct<CLASS, ARGTYPES...>(explicits);
}


// some explicits
template <typename CLASS, typename ... ARGTYPES, typename ... EXPGENS, typename std::enable_if<(sizeof...(EXPGENS) > 0 && sizeof...(EXPGENS) < sizeof...(ARGTYPES)), bool>::type = true >
decltype(auto) construct(EXPGENS&&... gens) {
    constexpr auto ExplicitSize = sizeof...(EXPGENS);
    constexpr auto ImplicitSize = sizeof...(ARGTYPES) - ExplicitSize;
    auto explicits = std::make_tuple(genToFunc(gens)...);
    using ArgsAsTuple = std::tuple<std::decay_t<ARGTYPES>...>;
    auto implicits = createGenHelperListed<ArgsAsTuple>(
        addOffset<ExplicitSize>(std::make_index_sequence<ImplicitSize>{})
    );

    return Construct<CLASS, ARGTYPES...>(std::tuple_cat(explicits, implicits));
}


} // namespace PropertyBasedTesting


