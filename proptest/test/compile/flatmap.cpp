#include "proptest/proptest.hpp"
#include "googletest/googletest/include/gtest/gtest.h"
#include "googletest/googlemock/include/gmock/gmock.h"

using namespace proptest;

TEST(Compile, flatmap)
{
    Random rand(3);
    auto gen = Arbi<map<string, string>>();
    gen(rand);
    gen.flatMap([](map<string, string>&) { return interval(0, 10); });
    gen.flatMap<int>([](map<string, string>&) { return interval(0, 10); });
}
