#include "proptest/proptest.hpp"
#include "googletest/googletest/include/gtest/gtest.h"
#include "googletest/googlemock/include/gmock/gmock.h"

using namespace proptest;

TEST(Compile, string)
{
    Random rand(1);
    auto gen = Arbi<string>();
    gen(rand);
}

TEST(Compile, stringGen)
{
    Arbi<string>(interval('A', 'Z'));
}
