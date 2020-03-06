#pragma once

#include "testing/gen.hpp"
#include "testing/Random.hpp"
#include <tuple>


namespace PropertyBasedTesting
{

template <typename... Ts>
class PROPTEST_API Arbitrary< std::tuple<Ts...>> : public Gen< std::tuple<Ts...> >
{
public:

    Arbitrary(Gen<T>... _elemGens) : elemGenTuple(std::make_tuple(_elemGens))  {
    }

    template <typename T, std::size_t... index>
    decltype(auto) generateHelper(Random& rand, std::index_sequence<index> index_sequence) {
        return std::make_tuple(std::get<index>(elemGenTuple).generate(rand)...);
    }

    Shrinkable<std::tuple<Ts...>> generate(Random& rand) {
        constexpr auto Size = sizeof...(Ts);
        int len = rand.getRandomSize(0, maxLen+1);
        std::tuple<Ts...> val;
        
        auto valueTuple = generateHelper(rand, std::make_index_sequence<Size>{});
        return Shrinkable<std::tuple<Ts...>>(std::move(valueTuple));
    }

    int maxLen;
    std::tuple<Gen<T>...> elemGenTuple;
};


}
