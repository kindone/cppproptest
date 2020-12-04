#include "proptest.hpp"

#include "googletest/googletest/include/gtest/gtest.h"
#include "googletest/googlemock/include/gmock/gmock.h"

using namespace proptest;

TEST(Compile, Shrinkable)
{
    auto shr1 = make_shrinkable<int>(1);
    auto shr2 = make_shrinkable<string>("hello");
    auto shr3 = make_shrinkable<vector<int>>(0);
    auto shr4 = make_shrinkable<function<int(int)>>([](int a) { return a + 5; });

    auto shr5 = make_shrinkable<map<string, string>>();
}
