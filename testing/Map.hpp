#pragma once

namespace PropertyBasedTesting {

template <typename T, typename GEN>
decltype(auto) filter(GEN&& gen, std::function<bool(const T&)> criteria) {
    return [=, &gen](Random& rand) {
        while(true) {
            Shrinkable<T> generated = gen(rand);
            if(criteria(generated.getRef())) {
                return generated;
            }
        }
    };
}

template <typename T, typename U,  typename GEN>
decltype(auto) map(GEN&& gen, std::function<U(const T&)> mapper) {
    return [=, &gen](Random& rand) {
        T generated = gen(rand);
        return mapper(generated);
    };
}

}
