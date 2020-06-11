#include "proptest.hpp"
#include "googletest/googletest/include/gtest/gtest.h"
#include "googletest/googlemock/include/gmock/gmock.h"
#include "Random.hpp"

using namespace PropertyBasedTesting;

TEST(Compile, pair)
{
    Random rand(1);
    auto gen = Arbitrary<std::pair<int8_t, uint8_t>>();
    gen(rand);
}
