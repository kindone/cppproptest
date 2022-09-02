#include "proptest.hpp"
#include "googletest/googletest/include/gtest/gtest.h"
#include "googletest/googlemock/include/gmock/gmock.h"
#include "Random.hpp"

using namespace proptest;

TEST(Compile, vector)
{
    Random rand(1);
    auto numGen = Arbi<int64_t>();
    auto gen1 = Arbi<vector<int64_t>>();
    auto gen2 = Arbi<vector<int64_t>>(numGen);
    auto gen3 = Arbi<vector<int64_t>>(generator(numGen));
    gen1(rand);
    gen2(rand);
    gen3(rand);
}
