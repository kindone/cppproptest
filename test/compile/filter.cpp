#include "proptest.hpp"
#include "googletest/googletest/include/gtest/gtest.h"
#include "googletest/googlemock/include/gmock/gmock.h"
#include "Random.hpp"

using namespace proptest;

TEST(Compile, filter)
{
    Random rand(3);
    auto gen = Arbi<map<string, string>>();
    gen(rand);
    filter(gen, [](map<string, string>&) { return true; });
    suchThat<map<string, string>>(gen, [](map<string, string>&) { return true; });
}
