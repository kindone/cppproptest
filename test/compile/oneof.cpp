#include "proptest.hpp"
#include "googletest/googletest/include/gtest/gtest.h"
#include "googletest/googlemock/include/gmock/gmock.h"
#include "Random.hpp"

using namespace proptest;

TEST(Compile, oneOf)
{
    oneOf<int>(Arbitrary<int>(), Arbitrary<int>(), Arbitrary<int>());
    oneOf<int>(weightedGen<int>(Arbitrary<int>(), 0.1), Arbitrary<int>(), Arbitrary<int>());
    oneOf<int>(weightedGen<int>(Arbitrary<int>(), 0.1), weightedGen<int>(Arbitrary<int>(), 0.1),
               weightedGen<int>(Arbitrary<int>(), 0.8));
}
