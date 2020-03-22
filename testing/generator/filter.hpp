#pragma once

namespace PropertyBasedTesting {


// FIXME: filter doesn't honor shrinking behavior
template <typename T, typename GEN>
decltype(auto) filter(GEN&& gen, std::function<bool(const T&)> criteria) {
    return [=, &gen](Random& rand) {
        while(true) {
            Shrinkable<T> shrinkable = gen(rand);
            if(criteria(shrinkable.getRef())) {
                return shrinkable.filter(criteria);
            }
        }
    };
}

}
