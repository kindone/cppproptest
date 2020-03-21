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

}
