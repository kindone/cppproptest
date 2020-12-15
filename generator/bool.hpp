#pragma once
#include "../gen.hpp"
#include "../Stream.hpp"
#include "../shrinker/bool.hpp"

namespace proptest {

template <>
class PROPTEST_API Arbi<bool> final : public ArbiBase<bool> {
public:
    Shrinkable<bool> operator()(Random& rand) override
    {
        bool value = rand.getRandomBool();
        return shrinkBool(value);
    }

    Arbi<bool>() = default;
    Arbi<bool>(const Arbi<bool>&) = default;

    virtual ~Arbi();
};

// template struct Arbi<bool>;

}  // namespace proptest
