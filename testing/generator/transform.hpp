#pragma once

namespace PropertyBasedTesting {

// FIXME: doesn't honor shrinking
template <typename T, typename U,  typename GEN>
decltype(auto) transform(GEN&& gen, std::function<U(const T&)> transformer) {
    return [=, &gen](Random& rand) {
        Shrinkable<T> shrinkable = gen(rand);
        return shrinkable.transform(transformer);
    };
}

}
