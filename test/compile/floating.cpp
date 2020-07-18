#include "proptest.hpp"
#include "googletest/googletest/include/gtest/gtest.h"
#include "googletest/googlemock/include/gmock/gmock.h"
#include "Random.hpp"

using namespace proptest;

TEST(Compile, floating)
{
    Random rand(1);
    auto gen = Arbitrary<float>();
    gen(rand);
}
