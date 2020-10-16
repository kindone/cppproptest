#pragma once
#include <vector>
#include <functional>

#include "../Random.hpp"
#include "../assert.hpp"
#include "../gen.hpp"
#include "../GenBase.hpp"

namespace proptest {

namespace util {
template <typename T>
struct Weighted;
}

template <typename T, typename GEN>
util::Weighted<T> weightedGen(GEN&& gen, double weight);

namespace util {

template <typename T>
struct Weighted
{
    using FuncType = GenFunction<T>;

    Weighted(std::shared_ptr<FuncType> _funcPtr, double _weight) : funcPtr(_funcPtr), weight(_weight) {}

    std::shared_ptr<FuncType> funcPtr;
    double weight;
};

template <typename T, typename GEN>
std::enable_if_t<!std::is_same<std::decay_t<GEN>, Weighted<T>>::value, Weighted<T>> GenToWeighted(GEN&& gen)
{
    return weightedGen<T>(std::forward<GEN>(gen), 0.0);
}

template <typename T>
Weighted<T> GenToWeighted(Weighted<T>&& weighted)
{
    return std::forward<Weighted<T>>(weighted);
}

template <typename T>
Weighted<T>& GenToWeighted(Weighted<T>& weighted)
{
    return weighted;
}

template <typename T>
decltype(auto) oneOfHelper(const std::shared_ptr<std::vector<util::Weighted<T>>>& genVecPtr)
{
    // calculate and assign unassigned weights
    double sum = 0.0;
    int numUnassigned = 0;
    for (size_t i = 0; i < genVecPtr->size(); i++) {
        double weight = (*genVecPtr)[i].weight;
        if (weight < 0.0 || weight > 1.0)
            throw std::runtime_error("invalid weight: " + std::to_string(weight));
        sum += weight;
        if (weight == 0.0)
            numUnassigned++;
    }

    if (sum > 1.0)
        throw std::runtime_error("sum of weight exceeds 1.0");

    if (numUnassigned > 0 && sum < 1.0)
        for (size_t i = 0; i < genVecPtr->size(); i++) {
            double& weight = (*genVecPtr)[i].weight;
            if (weight == 0.0)
                weight = (1.0 - sum) / static_cast<double>(numUnassigned);
        }

    return generator([genVecPtr](Random& rand) {
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
    });
}

}  // namespace util

template <typename T, typename GEN>
util::Weighted<T> weightedGen(GEN&& gen, double weight)
{
    using FuncType = GenFunction<T>;
    std::shared_ptr<FuncType> funcPtr = std::make_shared<FuncType>(std::forward<GEN>(gen));
    return util::Weighted<T>(funcPtr, weight);
}

// a GEN can be a generator or a weightedGen(GEN, weight)
template <typename T, typename... GENS>
decltype(auto) oneOf(GENS&&... gens)
{
    using WeightedVec = std::vector<util::Weighted<T>>;
    std::shared_ptr<WeightedVec> genVecPtr(new WeightedVec{util::GenToWeighted<T>(std::forward<GENS>(gens))...});

    return util::oneOfHelper<T>(genVecPtr);
}

}  // namespace proptest
