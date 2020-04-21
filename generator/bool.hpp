#pragma once
#include "gen.hpp"
#include "Stream.hpp"

namespace PropertyBasedTesting
{

template <>
class PROPTEST_API Arbitrary< bool > : public Gen< bool >
{
public:
    Shrinkable<bool> operator()(Random& rand) {
        bool value = rand.getRandomBool();
        if(value) {
            return make_shrinkable<bool>(value).with([]() {
                return Stream<Shrinkable<bool>>::one(make_shrinkable<bool>(false));
            });
        }
        else {
            return make_shrinkable<bool>(value);
        }

    }
};

//template struct Arbitrary<bool>;

} // namespace PropertyBasedTesting

