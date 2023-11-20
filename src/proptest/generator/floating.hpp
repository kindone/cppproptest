#pragma once
#include "proptest/gen.hpp"
#include "proptest/api.hpp"
#include "proptest/Stream.hpp"
#include "proptest/generator/util.hpp"

/**
 * @file floating.hpp
 * @brief Arbitrary for float and double
 */

namespace proptest {

/**
 * @ingroup Generators
 * @brief Arbitrary for float
 */
template <>
struct PROPTEST_API Arbi<float> : public ArbiBase<float>
{
public:
    Shrinkable<float> operator()(Random& rand) override;
    static constexpr float boundaryValues[] = {0.0, 1.0, -1.0};
};

/**
 * @ingroup Generators
 * @brief Arbitrary for double
 */
template <>
struct PROPTEST_API Arbi<double> : public ArbiBase<double>
{
public:
    Shrinkable<double> operator()(Random& rand) override;
    static constexpr double boundaryValues[] = {0.0, 1.0, -1.0};
};

}  // namespace proptest
