#pragma once
#include "../util/std.hpp"
#include "../Random.hpp"
#include "../assert.hpp"
#include "../gen.hpp"
#include "../GenBase.hpp"

/**
 * @file oneof.hpp
 * @brief Generator combinator for generating a type by choosing one of given generators with some probability
 */

namespace proptest {

namespace util {
template <typename T>
struct Weighted;
}  // namespace util

template <typename T>
util::Weighted<T> weightedGen(GenFunction<T> gen, double weight);

template <typename GEN>
auto weightedGen(GEN&& gen, double weight) -> util::Weighted<typename invoke_result_t<GEN, Random&>::type>;

namespace util {

template <typename T>
struct Weighted
{
    using type = T;
    using FuncType = GenFunction<T>;

    Weighted(shared_ptr<FuncType> _funcPtr, double _weight) : funcPtr(_funcPtr), weight(_weight) {}

    shared_ptr<FuncType> funcPtr;
    double weight;
};

template <typename T, typename GEN>
enable_if_t<!is_same<decay_t<GEN>, Weighted<T>>::value, Weighted<T>> GenToWeighted(GEN&& gen)
{
    return weightedGen<T>(util::forward<GEN>(gen), 0.0);
}

template <typename T>
Weighted<T> GenToWeighted(Weighted<T>&& weighted)
{
    return util::forward<Weighted<T>>(weighted);
}

template <typename T>
Weighted<T>& GenToWeighted(Weighted<T>& weighted)
{
    return weighted;
}

template <typename T>
decltype(auto) oneOfHelper(const shared_ptr<vector<util::Weighted<T>>>& genVecPtr)
{
    // calculate and assign unassigned weights
    double sum = 0.0;
    int numUnassigned = 0;
    for (size_t i = 0; i < genVecPtr->size(); i++) {
        double weight = (*genVecPtr)[i].weight;
        if (weight < 0.0 || weight > 1.0)
            throw runtime_error("invalid weight: " + to_string(weight));
        sum += weight;
        if (weight == 0.0)
            numUnassigned++;
    }

    if (sum > 1.0)
        throw runtime_error("sum of weight exceeds 1.0");

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


template <typename T>
util::Weighted<T> weightedGen(GenFunction<T> gen, double weight)
{
    auto funcPtr = util::make_shared<GenFunction<T>>(gen);
    return util::Weighted<T>(funcPtr, weight);
}

template <typename GEN>
auto weightedGen(GEN&& gen, double weight) -> util::Weighted<typename invoke_result_t<GEN, Random&>::type>
{
    using T = typename invoke_result_t<GEN, Random&>::type;
    return weightedGen<T>(util::forward<GEN>(gen), weight);
}

/**
 * @ingroup Combinators
 * @brief Generator combinator for generating a type by choosing one of given generators with some probability
 * @details You can combine generators into a single generator that can generate one of them with some probability. This
 * can be considered as taking a union of generators. It can generate a type T from multiple generators for type T, by
 * choosing one of the generators randomly, with even probability, or weighted probability. a GEN can be a generator or
 * a weightedGen(generator, weight) with the weight between 0 and 1 (exclusive). Unweighted generators take rest of
 * unweighted probability evenly.
 */
template <typename T, typename... GENS>
decltype(auto) oneOf(GENS&&... gens)
{
    static_assert(
        conjunction_v<std::bool_constant<(is_convertible_v<GENS, function<Shrinkable<T>(Random&)>> ||
                                          is_convertible_v<GENS, util::Weighted<T>>)>...>,
        "A GENS must be a generator callable for T (GenFunction<T> or Random& -> Shrinkable<T>) or a WeightGen<T>");
    using WeightedVec = vector<util::Weighted<T>>;
    shared_ptr<WeightedVec> genVecPtr(new WeightedVec{util::GenToWeighted<T>(util::forward<GENS>(gens))...});

    return util::oneOfHelper<T>(genVecPtr);
}

/**
 * @ingroup Combinators
 * @brief Alias for \ref oneOf combinator
 */
template <typename T, typename... GENS>
decltype(auto) unionOf(GENS&&... gens)
{
    static_assert(
        conjunction_v<std::bool_constant<(is_convertible_v<GENS, function<Shrinkable<T>(Random&)>> ||
                                          is_convertible_v<GENS, util::Weighted<T>>)>...>,
        "A GENS must be a generator callable for T (GenFunction<T> or Random& -> Shrinkable<T>) or a WeightGen<T>");
    return oneOf(util::forward<GENS>(gens)...);
}

}  // namespace proptest
