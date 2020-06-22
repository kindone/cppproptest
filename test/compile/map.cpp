#include "proptest.hpp"
#include "googletest/googletest/include/gtest/gtest.h"
#include "googletest/googlemock/include/gmock/gmock.h"
#include "Random.hpp"

using namespace PropertyBasedTesting;

TEST(Compile, map)
{
    Random rand(3);
    auto gen = Arbitrary<std::map<std::string, std::string>>();
    gen(rand);
}