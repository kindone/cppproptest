#include "proptest/proptest.hpp"
#include "googletest/googletest/include/gtest/gtest.h"
#include "googletest/googlemock/include/gmock/gmock.h"

using namespace proptest;

TEST(Compile, pair)
{
    Random rand(1);
    auto gen = Arbi<pair<int8_t, uint8_t>>();
    gen(rand);

    Arbi<bool>().pairWith(+[](bool& value) {
        if (value)
            return interval(0, 10);
        else
            return interval(10, 20);
    });
}
