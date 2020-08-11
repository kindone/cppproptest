#include "proptest.hpp"
#include "googletest/googletest/include/gtest/gtest.h"
#include "googletest/googlemock/include/gmock/gmock.h"
#include "Random.hpp"

using namespace proptest;

TEST(Compile, tuple)
{
    Random rand(1);
    auto gen = Arbi<std::tuple<int8_t, uint8_t, float>>();
    gen(rand);
}
