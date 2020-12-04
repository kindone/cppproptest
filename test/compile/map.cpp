#include "proptest.hpp"
#include "googletest/googletest/include/gtest/gtest.h"
#include "googletest/googlemock/include/gmock/gmock.h"
#include "Random.hpp"

using namespace proptest;

TEST(Compile, map)
{
    Random rand(3);
    auto gen = Arbi<map<string, string>>();
    gen(rand);
    gen.map([](map<string,string>&) {
        return 0;
    });
    gen.map<int>([](map<string,string>&) {
        return 0;
    });
}
