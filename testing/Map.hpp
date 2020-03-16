#pragma once

namespace PropertyBasedTesting {

template <typename T>
std::function<Shrinkable<T>(Random&)> filter(Gen<T>& gen, std::function<bool(const T&)> criteria) {
    return [=](Random& rand) {
        while(true) {
            Shrinkable<T> generated = gen(rand);
            if(criteria(generated.getRef())) {
                return generated;
            }
        }
    };
}

template <typename T, typename U>
std::function<U(Random&)> map(const Gen<T>& gen, std::function<U(const T&)> mapper) {
    return [=](Random& rand) {
        T generated = gen(rand);
        return mapper(generated);
    };
}

}
