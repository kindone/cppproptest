#pragma once

namespace PropertyBasedTesting {

template <typename T, typename U,  typename GEN>
decltype(auto) map(GEN&& gen, std::function<U(const T&)> mapper) {
    return [=, &gen](Random& rand) {
        T generated = gen(rand);
        return mapper(generated);
    };
}

}
