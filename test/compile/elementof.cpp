#include "proptest.hpp"
#include "googletest/googletest/include/gtest/gtest.h"
#include "googletest/googlemock/include/gmock/gmock.h"
#include "Random.hpp"

using namespace proptest;

struct Cat
{
    int age;
};

TEST(Compile, elementOf)
{
    elementOf<int>(0, 1, 2);
    elementOf<int>(weightedVal(0, 0.1), 1, 2);
    elementOf<int>(weightedVal(0, 0.1), weightedVal<int>(1, 0.1), weightedVal(2, 0.8));
}
