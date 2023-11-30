#include "proptest/proptest.hpp"
#include "googletest/googletest/include/gtest/gtest.h"
#include "googletest/googlemock/include/gmock/gmock.h"

using namespace proptest;

TEST(Compile, integral)
{
    Random rand(1);
    auto gen = Arbi<int32_t>();
    gen(rand);

    inRange<int>(0, 10);
    interval<int>(0, 10);
    integers<int>(0, 10);
}
