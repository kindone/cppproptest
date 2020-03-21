#pragma once

namespace PropertyBasedTesting {

template <typename T, typename U,  typename GEN>
decltype(auto) transform(GEN&& gen, std::function<U(const T&)> transformer) {
    return [=, &gen](Random& rand) {
        T generated = gen(rand);
        return transformer(generated);
    };
}

}
