#pragma once
#include <functional>
#include <utility>
#include "../Shrinkable.hpp"
#include "../Random.hpp"

namespace PropertyBasedTesting {


// returns a shrinkable pair of <T,U> where U depends on T
template <typename T, typename U>
std::function<Shrinkable<std::pair<T,U>>(Random&)> dependency(std::function<Shrinkable<T>(Random&)> gen1, std::function<std::function<Shrinkable<U>(Random&)>(const T&)> gen2gen) {
    auto genPair = [gen1, gen2gen](Random& rand) -> Shrinkable<std::pair<T,U>> {
        Shrinkable<T> shrinkableT = gen1(rand);
        using Intermediate = std::pair<T, Shrinkable<U>>;

        // expand Shrinkable<T>
        Shrinkable<std::pair<T, Shrinkable<U>>> intermediate = shrinkableT.template transform<std::pair<T, Shrinkable<U>>>([&rand, gen2gen](const T& t) {
            auto gen2 = gen2gen(t);
            Shrinkable<U> shrinkableU = gen2(rand);
            return make_shrinkable<std::pair<T, Shrinkable<U>> >(std::make_pair(t, shrinkableU));
        });

        // expand Shrinkable<U>
        intermediate = intermediate.andThen([](const Shrinkable<Intermediate>& interShr) -> Stream<Shrinkable<Intermediate>> {
            // assume interShr has no shrinks
            Intermediate& interpair = interShr.getRef();
            T& t = interpair.first;
            Shrinkable<U>& shrinkableU = interpair.second;
            Shrinkable<Intermediate> newShrinkableU = shrinkableU.template transform<Intermediate>([t](const U& u) mutable {
                return make_shrinkable<std::pair<T, Shrinkable<U>> >(std::make_pair(t, make_shrinkable<U>(u)));
            });
            return newShrinkableU.shrinks();
        });

        // reformat std::pair<T, Shrinkable<U>> to std::pair<T, U>
        return intermediate.template transform<std::pair<T, U>>([](const Intermediate& interpair) -> Shrinkable<std::pair<T,U>>{
            return make_shrinkable<std::pair<T, U>>(std::make_pair(interpair.first, interpair.second.getRef()));
        });
    };

    return genPair;
}

}

