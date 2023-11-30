#include "proptest/proptest.hpp"
#include "googletest/googletest/include/gtest/gtest.h"
#include "googletest/googlemock/include/gmock/gmock.h"

using namespace proptest;

TEST(Compile, map)
{
    Random rand(3);
    auto gen = Arbi<map<string, string>>();
    gen(rand);
    gen.map([](map<string, string>&) { return 0; });
    gen.map<int>([](map<string, string>&) { return 0; });
}

TEST(Compile, map_keygen)
{
    auto mapGen = Arbi<map<int, int>>();
    auto intArbi = Arbi<int>();
    auto intGen = inRange(0,1);
    auto intGenFunc = +[](Random&) -> Shrinkable<int> {
        return make_shrinkable<int>(0);
    };

    mapGen.setKeyGen(intArbi);
    mapGen.setKeyGen(intGen);
    mapGen.setKeyGen(intGenFunc);
    mapGen.setElemGen(intArbi);
    mapGen.setElemGen(intGen);
    mapGen.setElemGen(intGenFunc);
}
