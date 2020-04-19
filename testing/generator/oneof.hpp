#pragma once
#include <vector>
#include <functional>

#include "testing/Random.hpp"

namespace PropertyBasedTesting {

template <typename T, typename GEN>
decltype(auto) GenToFunction(GEN&& gen) {
    using FuncType = std::function<Shrinkable<T>(Random&)>;
    std::shared_ptr<FuncType> funcPtr = std::make_shared<FuncType>(gen);
    return [funcPtr](Random& rand) {
        return (*funcPtr)(rand);
    };
}

template <typename T, typename... GENS>
decltype(auto) oneOf(GENS&&... gens) {
    return [genVec = std::vector<std::function<Shrinkable<T>(Random&)>>{GenToFunction<T>(gens)...}](Random& rand) {
        auto dice = rand.getRandomSize(0, genVec.size());
        return genVec[dice](rand);
    };
}

}
