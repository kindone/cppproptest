#pragma once

#include "testing/gen.hpp"
#include "testing/Random.hpp"
#include "testing/shrinkable.hpp"
#include <tuple>


namespace PropertyBasedTesting
{

namespace TupleUtil {

bool OR()
{
    return false;
}

template <typename T, typename...Us>
auto OR(T&& t, Us&&... us)
{
    return OR(std::forward<Us>(us)...) || std::forward<T>(t);
}

template <typename T, size_t...Is>
auto or_components_impl(T const& t, std::index_sequence<Is...>)
{
    constexpr auto last_index = sizeof...(Is) - 1;
    return OR(std::get<last_index - Is>(t)...);
}

template <class Tuple>
bool or_components(const Tuple& t)
{
    constexpr auto Size = std::tuple_size<Tuple>{};
    return or_components_impl(t, std::make_index_sequence<Size>{});
}

}

template <typename... Ts>
class PROPTEST_API Arbitrary< std::tuple<Ts...>> : public Gen< std::tuple<Ts...> >
{
public:
    constexpr auto Size = sizeof...(Ts);

    Arbitrary(Gen<T>... _elemGens) : elemGenTuple(std::make_tuple(_elemGens))  {
    }

    template <typename T, std::size_t... index>
    decltype(auto) generateHelper(Random& rand, std::index_sequence<index> index_sequence) {
        return std::make_tuple(std::get<index>(elemGenTuple).generate(rand)...);
    }

    Shrinkable<std::tuple<Ts...>> generate(Random& rand) {
        constexpr auto Size = sizeof...(Ts);
        std::tuple<Ts...> val;        
        auto valueTuple = generateHelper(rand, std::make_index_sequence<Size>{});

        return make_shrinkable<std::tuple<Ts...>>(valueTuple).with([]() -> Stream<Shrinkable<std::tuple<Ts...>>> {
            /* strategy: 
             * assume elements are independent
             * shrink each element one by one upto its limit
             * stream of stream?
             * */
            constexpr auto Size = sizeof...(Ts);
            for(size_t i = 0; i < Size; i++) {

            }
            return Stream<Shrinkable<std::tuple<Ts...>>>::empty();
        });
    }

    std::tuple<Gen<T>...> elemGenTuple;
};


}
