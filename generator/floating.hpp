#pragma once
#include "../gen.hpp"
#include "../api.hpp"
#include "../Stream.hpp"
#include "util.hpp"

namespace proptest {

template <>
struct PROPTEST_API Arbitrary<float> : public ArbitraryBase<float>
{
public:
    Shrinkable<float> operator()(Random& rand) override;
    static constexpr float boundaryValues[] = {0.0, 1.0, -1.0};
};

template <>
struct PROPTEST_API Arbitrary<double> : public ArbitraryBase<double>
{
public:
    Shrinkable<double> operator()(Random& rand) override;
    static constexpr double boundaryValues[] = {0.0, 1.0, -1.0};
};

}  // namespace proptest
