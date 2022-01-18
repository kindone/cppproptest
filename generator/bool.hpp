#pragma once
#include "../gen.hpp"
#include "../Stream.hpp"
#include "../shrinker/bool.hpp"

namespace proptest {

template <>
class PROPTEST_API Arbi<bool> final : public ArbiBase<bool> {
public:
    Arbi(double prob = 0.5) : trueProb(prob) { }
    Shrinkable<bool> operator()(Random& rand) override
    {
        bool value = rand.getRandomBool(trueProb);
        return shrinkBool(value);
    }

    Arbi<bool>(const Arbi<bool>&) = default;

    virtual ~Arbi();

    double trueProb;
};

// template struct Arbi<bool>;

}  // namespace proptest
