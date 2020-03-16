#ifndef __PROPTEST_BOOL_HPP
#define __PROPTEST_BOOL_HPP

#include "testing/gen.hpp"

namespace PropertyBasedTesting
{

template <>
class PROPTEST_API Arbitrary< bool > : public Gen< bool >
{
public:
    Shrinkable<bool> operator()(Random& rand) {
        return make_shrinkable<bool>(rand.getRandomBool());
    }
};

//template struct Arbitrary<bool>;

} // namespace PropertyBasedTesting

#endif
