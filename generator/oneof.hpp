#pragma once
#include <vector>
#include <functional>

#include "../Random.hpp"
#include "../assert.hpp"

namespace PropertyBasedTesting {

namespace util {
template <typename T>
struct Weighted;
}

template <typename T, typename GEN>
util::Weighted<T> weighted(GEN&& gen, float weight);

namespace util {

template <typename T>
struct Weighted
{
    using FuncType = std::function<Shrinkable<T>(Random&)>;

    Weighted(std::shared_ptr<FuncType> funcPtr, float weight) : funcPtr(funcPtr), weight(weight) {}

    std::shared_ptr<FuncType> funcPtr;
    float weight;
};

template <typename T, typename GEN, std::enable_if_t<!std::is_same<GEN, Weighted<T>>::value, bool> = true>
Weighted<T> GenToWeighted(GEN&& gen)
{
    return weighted<T>(std::forward<GEN>(gen), 0.0);
}

template <typename T>
Weighted<T> GenToWeighted(Weighted<T>&& weighted)
{
    return std::forward<Weighted<T>>(weighted);
}

}  // namespace util

template <typename T, typename GEN>
util::Weighted<T> weighted(GEN&& gen, float weight)
{
    using FuncType = std::function<Shrinkable<T>(Random&)>;
    std::shared_ptr<FuncType> funcPtr = std::make_shared<FuncType>(std::forward<GEN>(gen));
    return util::Weighted<T>(funcPtr, weight);
}

// a GEN can be a generator or a weighted(GEN, weight)
template <typename T, typename... GENS>
decltype(auto) oneOf(GENS&&... gens)
{
    using WeightedVec = std::vector<util::Weighted<T>>;
    std::shared_ptr<WeightedVec> genVecPtr(new WeightedVec{util::GenToWeighted<T>(std::forward<GENS>(gens))...});

    // calculate and assign unassigned weights
    float sum = 0.0f;
    int numUnassigned = 0;
    for (size_t i = 0; i < genVecPtr->size(); i++) {
        float weight = (*genVecPtr)[i].weight;
        if (weight < 0.0f || weight > 1.0f)
            throw std::runtime_error("invalid weight: " + std::to_string(weight));
        sum += weight;
        if (weight == 0.0f)
            numUnassigned++;
    }

    if (sum > 1.0f)
        throw std::runtime_error("sum of weight exceeds 1.0");

    if (numUnassigned > 0 && sum < 1.0f)
        for (size_t i = 0; i < genVecPtr->size(); i++) {
            float& weight = (*genVecPtr)[i].weight;
            if (weight == 0.0f)
                weight = (1.0f - sum) / numUnassigned;
        }

    return [genVecPtr](Random& rand) {
        while (true) {
            auto dice = rand.getRandomSize(0, genVecPtr->size());
            const util::Weighted<T>& weighted = (*genVecPtr)[dice];
            if (rand.getRandomBool(weighted.weight)) {
                // retry the same generator if an exception is thrown
                while (true) {
                    try {
                        return (*weighted.funcPtr)(rand);
                    } catch (const Discard&) {
                        // TODO: trace level low
                    }
                }
            }
        };
    };
}

}  // namespace PropertyBasedTesting
