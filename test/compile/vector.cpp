#include "proptest.hpp"
#include "googletest/googletest/include/gtest/gtest.h"
#include "googletest/googlemock/include/gmock/gmock.h"
#include "Random.hpp"

using namespace proptest;

TEST(Compile, vector)
{
    Random rand(1);
    auto gen = Arbitrary<std::vector<int64_t>>();
    gen(rand);
}
