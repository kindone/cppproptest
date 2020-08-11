#pragma once
#include "../gen.hpp"
#include "../api.hpp"
#include "../Stream.hpp"
#include "util.hpp"

namespace proptest {

template <>
struct PROPTEST_API Arbi<float> : public ArbiBase<float>
{
public:
    Shrinkable<float> operator()(Random& rand) override;
    static constexpr float boundaryValues[] = {0.0, 1.0, -1.0};
};

template <>
struct PROPTEST_API Arbi<double> : public ArbiBase<double>
{
public:
    Shrinkable<double> operator()(Random& rand) override;
    static constexpr double boundaryValues[] = {0.0, 1.0, -1.0};
};

}  // namespace proptest
