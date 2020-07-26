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

// returns a shrinkable pair of <T,U> where U depends on T
template <typename T, typename U>
Generator<std::pair<T, U>> dependency(std::function<Shrinkable<T>(Random&)> gen1,
                                      std::function<std::function<Shrinkable<U>(Random&)>(T&)> gen2gen)
{
    auto gen1Ptr = std::make_shared<decltype(gen1)>(gen1);
    auto gen2genPtr = std::make_shared<std::function<std::function<Shrinkable<U>(Random&)>(const T&)>>(
        [gen2gen](const T& t) { return gen2gen(const_cast<T&>(t)); });

    static auto genPair = [gen1Ptr, gen2genPtr](Random& rand) -> Shrinkable<std::pair<T, U>> {
        // generate T
        Shrinkable<T> shrinkableT = (*gen1Ptr)(rand);
        using Intermediate = std::pair<T, Shrinkable<U>>;

        // shrink strategy 1: expand Shrinkable<T>
        Shrinkable<std::pair<T, Shrinkable<U>>> intermediate =
            shrinkableT.template transform<std::pair<T, Shrinkable<U>>>([&rand, gen2genPtr](const T& t) {
                // generate U
                auto gen2 = (*gen2genPtr)(t);
                Shrinkable<U> shrinkableU = gen2(rand);
                return make_shrinkable<std::pair<T, Shrinkable<U>>>(std::make_pair(t, shrinkableU));
            });

        // shrink strategy 2: expand Shrinkable<U>
        intermediate =
            intermediate.andThen([](const Shrinkable<Intermediate>& interShr) -> Stream<Shrinkable<Intermediate>> {
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

        // reformat std::pair<T, Shrinkable<U>> to std::pair<T, U>
        return intermediate.template transform<std::pair<T, U>>(
            [](const Intermediate& interpair) -> Shrinkable<std::pair<T, U>> {
                return make_shrinkable<std::pair<T, U>>(std::make_pair(interpair.first, interpair.second.getRef()));
            });
    };

    return generator(genPair);
}

template <typename U, typename T0, typename... Ts>
Generator<std::tuple<T0, Ts..., U>> chainImpl(
    std::function<Shrinkable<std::tuple<T0, Ts...>>(Random&)> gen1,
    std::function<std::function<Shrinkable<U>(Random&)>(std::tuple<T0, Ts...>&)> gen2gen)
{
    auto gen1Ptr = std::make_shared<decltype(gen1)>(gen1);
    auto gen2genPtr =
        std::make_shared<std::function<std::function<Shrinkable<U>(Random&)>(const std::tuple<T0, Ts...>&)>>(
            [gen2gen](const std::tuple<T0, Ts...>& ts) { return gen2gen(const_cast<std::tuple<T0, Ts...>&>(ts)); });

    static auto genTuple = [gen1Ptr, gen2genPtr](Random & rand) -> Shrinkable<std::tuple<T0, Ts..., U>>
    {
        // generate T
        Shrinkable<std::tuple<T0, Ts...>> shrinkableTs = (*gen1Ptr)(rand);
        using Intermediate = std::pair<std::tuple<T0, Ts...>, Shrinkable<U>>;

        // shrink strategy 1: expand Shrinkable<std::tuple<Ts...>>
        Shrinkable<std::pair<std::tuple<T0, Ts...>, Shrinkable<U>>> intermediate =
            shrinkableTs.template transform<std::pair<std::tuple<T0, Ts...>, Shrinkable<U>>>(
                [&rand, gen2genPtr](const std::tuple<T0, Ts...>& ts) {
                    // generate U
                    auto gen2 = (*gen2genPtr)(ts);
                    Shrinkable<U> shrinkableU = gen2(rand);
                    return make_shrinkable<std::pair<std::tuple<T0, Ts...>, Shrinkable<U>>>(
                        std::make_pair(ts, shrinkableU));
                });

        // shrink strategy 2: expand Shrinkable<U>
        intermediate =
            intermediate.andThen([](const Shrinkable<Intermediate>& interShr) -> Stream<Shrinkable<Intermediate>> {
                // assume interShr has no shrinks
                Intermediate& interpair = interShr.getRef();
                std::tuple<T0, Ts...>& ts = interpair.first;
                Shrinkable<U>& shrinkableU = interpair.second;
                Shrinkable<Intermediate> newShrinkableU =
                    shrinkableU.template transform<Intermediate>([ts](const U& u) mutable {
                        return make_shrinkable<std::pair<std::tuple<T0, Ts...>, Shrinkable<U>>>(
                            std::make_pair(ts, make_shrinkable<U>(u)));
                    });
                return newShrinkableU.shrinks();
            });

        // reformat std::pair<std::tuple<T0, Ts...>, Shrinkable<U>> to std::tuple<T0, Ts..., U>
        return intermediate.template transform<std::tuple<T0, Ts..., U>>(
            [](const Intermediate& interpair) -> Shrinkable<std::tuple<T0, Ts..., U>> {
                std::tuple<T0, Ts...> ts = interpair.first;
                return make_shrinkable<std::tuple<T0, Ts..., U>>(
                    std::tuple_cat(ts, std::make_tuple(interpair.second.getRef())));
            });
    };

    return generator(genTuple);
}

// this is required to overcome deduction failure for callables that we'd like to have as std::functions
template <typename GEN1, typename GEN2GEN>
decltype(auto) chain(GEN1&& gen1, GEN2GEN&& gen2gen)
{
    using TUPLE = typename function_traits<GEN1>::return_type::type;
    using RetType = typename function_traits<GEN2GEN>::return_type;
    std::function<Shrinkable<TUPLE>(Random&)> funcGen1 = gen1;
    std::function<RetType(TUPLE&)> funcGen2Gen = gen2gen;
    return chainImpl(funcGen1, funcGen2Gen);
}

}  // namespace proptest
