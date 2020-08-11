#pragma once
#include "../gen.hpp"
#include "../Stream.hpp"

namespace proptest {

template <>
class PROPTEST_API Arbi<bool> final : public ArbiBase<bool> {
public:
    Shrinkable<bool> operator()(Random& rand) override
    {
        bool value = rand.getRandomBool();
        if (value) {
            return make_shrinkable<bool>(value).with(
                +[]() { return Stream<Shrinkable<bool>>::one(make_shrinkable<bool>(false)); });
        } else {
            return make_shrinkable<bool>(value);
        }
    }
};

// template struct Arbi<bool>;

}  // namespace proptest
