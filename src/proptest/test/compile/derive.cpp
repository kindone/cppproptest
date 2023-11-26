#include "proptest/proptest.hpp"
#include "googletest/googletest/include/gtest/gtest.h"
#include "googletest/googlemock/include/gmock/gmock.h"


using namespace proptest;

TEST(Compile, derive)
{
    auto intGen = elementOf<int>(2, 4, 6);
    auto stringGen = derive<int, string>(intGen, [](int& value) {
        auto gen = Arbi<string>();
        gen.setMaxSize(value);
        return gen;
    });
}
