#pragma once
#include "proptest/gen.hpp"
#include "proptest/Stream.hpp"
#include "proptest/shrinker/bool.hpp"

/**
 * @file bool.hpp
 * @brief Arbitrary for bool
 */

namespace proptest {

/**
 * @ingroup Generators
 * @brief Arbitrary for boolean with configurable true/false probability
 */
template <>
class PROPTEST_API Arbi<bool> final : public ArbiBase<bool> {
public:
    Arbi(double prob = 0.5);

    Shrinkable<bool> operator()(Random& rand) override;

    Arbi(const Arbi<bool>&) = default;

    virtual ~Arbi();

    double trueProb;
};

// template struct Arbi<bool>;

}  // namespace proptest
