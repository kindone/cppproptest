#include "proptest/proptest.hpp"
#include "googletest/googletest/include/gtest/gtest.h"
#include "googletest/googlemock/include/gmock/gmock.h"
#include "proptest/Random.hpp"

using namespace proptest;

TEST(Compile, lazy)
{
    Random rand(3);
    auto gen = lazy([]() { return 5; });

    gen(rand);
}
