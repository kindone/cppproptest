#include "proptest/proptest.hpp"
#include "googletest/googletest/include/gtest/gtest.h"
#include "googletest/googlemock/include/gmock/gmock.h"
#include "proptest/Random.hpp"

using namespace proptest;

TEST(Compile, floating)
{
    Random rand(1);
    auto gen = Arbi<float>();
    gen(rand);
}
