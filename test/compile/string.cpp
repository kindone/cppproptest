#include "proptest.hpp"
#include "googletest/googletest/include/gtest/gtest.h"
#include "googletest/googlemock/include/gmock/gmock.h"
#include "Random.hpp"

using namespace proptest;

TEST(Compile, string)
{
    Random rand(1);
    auto gen = Arbi<std::string>();
    gen(rand);
}

TEST(Compile, stringGen)
{
    Arbi<std::string>(interval('A', 'Z'));
}
