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

    auto genPair = [gen1Ptr, gen2genPtr](Random& rand) -> Shrinkable<std::pair<T, U>> {
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

        // reformat std::pair<T, Shrinkable<U>> to std::pair<T, U>
        return intermediate.template transform<std::pair<T, U>>(
            +[](const Intermediate& interpair) -> Shrinkable<std::pair<T, U>> {
                return make_shrinkable<std::pair<T, U>>(std::make_pair(interpair.first, interpair.second.getRef()));
            });
    };

    return generator(genPair);
}

}  // namespace proptest
