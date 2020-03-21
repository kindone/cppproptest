#pragma once


namespace PropertyBasedTesting {

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
