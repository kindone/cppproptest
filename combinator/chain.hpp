#pragma once
#include <functional>
#include <utility>
#include "../Shrinkable.hpp"
#include "../Random.hpp"

namespace proptest {

template <typename GEN>
decltype(auto) generator(GEN&& gen);
template <typename T>
struct Generator;

// template <class... Ts>
// struct Chain : public std::tuple<Ts...>
// {
// };

template <class... Ts>
using Chain = std::tuple<Ts...>;

template <typename U, typename T>
Generator<Chain<T, U>> chainImpl(std::function<Shrinkable<T>(Random&)> gen1,
                                 std::function<std::function<Shrinkable<U>(Random&)>(T&)> gen2gen)
{
    auto gen1Ptr = std::make_shared<decltype(gen1)>(gen1);
    auto gen2genPtr = std::make_shared<std::function<std::function<Shrinkable<U>(Random&)>(const T&)>>(
        [gen2gen](const T& t) { return gen2gen(const_cast<T&>(t)); });

    auto genTuple = [gen1Ptr, gen2genPtr](Random& rand) -> Shrinkable<Chain<T, U>> {
        // generate T
        Shrinkable<T> shrinkableTs = (*gen1Ptr)(rand);
        using Intermediate = std::pair<T, Shrinkable<U>>;

        // shrink strategy 1: expand Shrinkable<T>
        Shrinkable<std::pair<T, Shrinkable<U>>> intermediate =
            shrinkableTs.template transform<std::pair<T, Shrinkable<U>>>([&rand, gen2genPtr](const T& t) {
                // generate U
                auto gen2 = (*gen2genPtr)(t);
                Shrinkable<U> shrinkableU = gen2(rand);
                return make_shrinkable<std::pair<T, Shrinkable<U>>>(std::make_pair(t, shrinkableU));
            });

        // shrink strategy 2: expand Shrinkable<U>
        intermediate =
            intermediate.andThen(+[](const Shrinkable<Intermediate>& interShr) -> Stream<Shrinkable<Intermediate>> {
                // assume interShr has no shrinks
                Intermediate& interpair = interShr.getRef();
                T& t = interpair.first;
                Shrinkable<U>& shrinkableU = interpair.second;
                Shrinkable<Intermediate> newShrinkableU =
                    shrinkableU.template transform<Intermediate>([t](const U& u) mutable {
                        return make_shrinkable<std::pair<T, Shrinkable<U>>>(std::make_pair(t, make_shrinkable<U>(u)));
                    });
                return newShrinkableU.shrinks();
            });

        // reformat std::pair<T, Shrinkable<U>> to Chain<T, U>
        return intermediate.template transform<Chain<T, U>>(
            +[](const Intermediate& interpair) -> Shrinkable<std::tuple<T, U>> {
                const T& t = interpair.first;
                return make_shrinkable<Chain<T, U>>(
                    std::tuple_cat(std::tuple<T>(t), std::make_tuple(interpair.second.getRef())));
            });
    };

    return generator(genTuple);
}

template <typename U, typename T0, typename T1, typename... Ts>
Generator<Chain<T0, T1, Ts..., U>> chainImpl(
    std::function<Shrinkable<Chain<T0, T1, Ts...>>(Random&)> gen1,
    std::function<std::function<Shrinkable<U>(Random&)>(Chain<T0, T1, Ts...>&)> gen2gen)
{
    auto gen1Ptr = std::make_shared<decltype(gen1)>(gen1);
    auto gen2genPtr =
        std::make_shared<std::function<std::function<Shrinkable<U>(Random&)>(const Chain<T0, T1, Ts...>&)>>(
            [gen2gen](const Chain<T0, T1, Ts...>& ts) { return gen2gen(const_cast<Chain<T0, T1, Ts...>&>(ts)); });

    auto genTuple = [gen1Ptr, gen2genPtr](Random & rand) -> Shrinkable<Chain<T0, T1, Ts..., U>>
    {
        // generate T
        Shrinkable<Chain<T0, T1, Ts...>> shrinkableTs = (*gen1Ptr)(rand);
        using Intermediate = std::pair<Chain<T0, T1, Ts...>, Shrinkable<U>>;

        // shrink strategy 1: expand Shrinkable<std::tuple<Ts...>>
        Shrinkable<std::pair<Chain<T0, T1, Ts...>, Shrinkable<U>>> intermediate =
            shrinkableTs.template transform<std::pair<Chain<T0, T1, Ts...>, Shrinkable<U>>>(
                [&rand, gen2genPtr](const Chain<T0, T1, Ts...>& ts) {
                    // generate U
                    auto gen2 = (*gen2genPtr)(ts);
                    Shrinkable<U> shrinkableU = gen2(rand);
                    return make_shrinkable<std::pair<Chain<T0, T1, Ts...>, Shrinkable<U>>>(
                        std::make_pair(ts, shrinkableU));
                });

        // shrink strategy 2: expand Shrinkable<U>
        intermediate =
            intermediate.andThen(+[](const Shrinkable<Intermediate>& interShr) -> Stream<Shrinkable<Intermediate>> {
                // assume interShr has no shrinks
                Intermediate& interpair = interShr.getRef();
                Chain<T0, T1, Ts...>& ts = interpair.first;
                Shrinkable<U>& shrinkableU = interpair.second;
                Shrinkable<Intermediate> newShrinkableU =
                    shrinkableU.template transform<Intermediate>([ts](const U& u) mutable {
                        return make_shrinkable<std::pair<Chain<T0, T1, Ts...>, Shrinkable<U>>>(
                            std::make_pair(ts, make_shrinkable<U>(u)));
                    });
                return newShrinkableU.shrinks();
            });

        // reformat std::pair<Chain<T0, T1, Ts...>, Shrinkable<U>> to Chain<T0, T1, Ts..., U>
        return intermediate.template transform<Chain<T0, T1, Ts..., U>>(
            +[](const Intermediate& interpair) -> Shrinkable<std::tuple<T0, T1, Ts..., U>> {
                const Chain<T0, T1, Ts...>& ts = interpair.first;
                return make_shrinkable<Chain<T0, T1, Ts..., U>>(
                    std::tuple_cat(ts, std::tuple<U>(interpair.second.getRef())));
            });
    };

    return generator(genTuple);
}

// this is required to overcome deduction failure for callables that we'd like to have as std::functions
template <typename GEN1, typename GEN2GEN>
decltype(auto) chain(GEN1&& gen1, GEN2GEN&& gen2gen)
{
    using CHAIN = typename function_traits<GEN1>::return_type::type;  // get the T from shrinkable<T>(Random&)
    using RetType = typename function_traits<GEN2GEN>::return_type;
    std::function<Shrinkable<CHAIN>(Random&)> funcGen1 = gen1;
    std::function<RetType(CHAIN&)> funcGen2Gen = gen2gen;
    return chainImpl(funcGen1, funcGen2Gen);
}

}  // namespace proptest
