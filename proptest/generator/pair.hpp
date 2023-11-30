#pragma once

#include "../gen.hpp"
#include "../Random.hpp"
#include "../Shrinkable.hpp"
#include "../shrinker/pair.hpp"
#include "../util/std.hpp"

/**
 * @file pair.hpp
 * @brief Arbitrary for pair<ARG1,ARG2> and utility function pairOf(gen1, gen2)
 */

namespace proptest {

/**
 * @ingroup Combinators
 * @brief Generator combinator for pair<T1, T2>
 * @details shrinking is done by one parameter and then continues to the next
 */
template <typename GEN1, typename GEN2>
decltype(auto) pairOf(GEN1&& gen1, GEN2&& gen2)
{
    auto genPairPtr =
        util::make_shared<pair<decay_t<GEN1>, decay_t<GEN2>>>(util::forward<GEN1>(gen1), util::forward<GEN2>(gen2));
    // generator
    return generator(
        [genPairPtr](Random& rand) mutable { return shrinkPair(genPairPtr->first(rand), genPairPtr->second(rand)); });
}

/**
 * @ingroup Generators
 * @brief Arbitrary for pair<T1, T2> based on Arbitrary<T1> and Arbitrary<T2>
 */
template <typename ARG1, typename ARG2>
class PROPTEST_API Arbi<pair<ARG1, ARG2>> final : public ArbiBase<pair<ARG1, ARG2>> {
public:
    Shrinkable<pair<ARG1, ARG2>> operator()(Random& rand) override { return pairOf(Arbi<ARG1>(), Arbi<ARG2>())(rand); }
};

}  // namespace proptest
