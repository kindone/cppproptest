#include "proptest.hpp"
#include "googletest/googletest/include/gtest/gtest.h"
#include "googletest/googlemock/include/gmock/gmock.h"
#include "Random.hpp"

using namespace pbt;

TEST(Compile, numeric)
{
    Random rand(1);
    auto gen = Arbitrary<int32_t>();
    gen(rand);
}
