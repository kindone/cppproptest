#pragma once
#include "../gen.hpp"
#include "../Stream.hpp"

namespace proptest {

template <>
class PROPTEST_API Arbitrary<bool> final : public ArbitraryBase<bool> {
public:
    Shrinkable<bool> operator()(Random& rand) override
    {
        bool value = rand.getRandomBool();
        if (value) {
            return make_shrinkable<bool>(value).with(
                []() { return Stream<Shrinkable<bool>>::one(make_shrinkable<bool>(false)); });
        } else {
            return make_shrinkable<bool>(value);
        }
    }
};

// template struct Arbitrary<bool>;

}  // namespace proptest
