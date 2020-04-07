#pragma once
#include <functional>
#include "testing/Shrinkable.hpp"
#include "testing/Random.hpp"

namespace PropertyBasedTesting {

template <typename T, typename U>
decltype(auto) dependency(std::function<Shrinkable<T>(Random&)> gen1, std::function<std::function<Shrinkable<U>(Random&)>(const T&)> gen2gen) {
    auto gen2 = [gen1, gen2gen](Random& rand) -> Shrinkable<U> {
        Shrinkable<T> shrinkable1 = gen1(rand);

        return gen2gen(shrinkable1.getRef())(rand);
    };

    return tuple(gen1, gen2);
}

}