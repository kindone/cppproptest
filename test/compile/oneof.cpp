#include "proptest.hpp"
#include "googletest/googletest/include/gtest/gtest.h"
#include "googletest/googlemock/include/gmock/gmock.h"
#include "Random.hpp"

using namespace proptest;

TEST(Compile, oneOf)
{
    oneOf<int>(Arbi<int>(), Arbi<int>(), Arbi<int>());
    oneOf<int>(weightedGen<int>(Arbi<int>(), 0.1), Arbi<int>(), Arbi<int>());
    oneOf<int>(weightedGen<int>(Arbi<int>(), 0.1), weightedGen<int>(Arbi<int>(), 0.1),
               weightedGen<int>(Arbi<int>(), 0.8));
}
