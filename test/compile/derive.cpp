#include "proptest.hpp"
#include "googletest/googletest/include/gtest/gtest.h"
#include "googletest/googlemock/include/gmock/gmock.h"
#include "Random.hpp"

using namespace proptest;

TEST(Compile, derive)
{
    auto intGen = elementOf<int>(2, 4, 6);
    auto stringGen = derive<int, std::string>(intGen, [](int& value) {
        auto gen = Arbi<std::string>();
        gen.setMaxSize(value);
        return gen;
    });
}
