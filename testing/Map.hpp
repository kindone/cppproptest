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

template <typename GEN>
decltype(auto) GenToFunction(GEN&& gen) {
    return [&gen](Random& rand) {
        return gen(rand);
    };
}

template <typename T, typename... GENS>
decltype(auto) oneOf(GENS&&... gens) {
    std::vector<std::function<T(Random&)>> genVec{GenToFunction(gens)...};
    return [genVec](Random& rand) {
        auto dice = rand.getRandomSize(0, genVec.size());
        return genVec[dice](rand);
    };
}

}
