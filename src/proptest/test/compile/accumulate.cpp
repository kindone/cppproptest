#include "proptest/proptest.hpp"
#include "googletest/googletest/include/gtest/gtest.h"
#include "googletest/googlemock/include/gmock/gmock.h"
#include "proptest/Random.hpp"

using namespace proptest;

TEST(Compile, accumulate)
{
    auto gen1 = interval<int>(0, 1);

    [[maybe_unused]] auto gen = accumulate(
        gen1, [](int& num) { return interval(num, num + 2); }, 2, 4);
}
