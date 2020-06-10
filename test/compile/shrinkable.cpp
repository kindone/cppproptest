#include "proptest.hpp"

#include "googletest/googletest/include/gtest/gtest.h"
#include "googletest/googlemock/include/gmock/gmock.h"

using namespace PropertyBasedTesting;

TEST(Compile, Shrinkable)
{
    auto shr1 = make_shrinkable<int>(0);
    auto shr2 = make_shrinkable<std::string>("hello");
    auto shr3 = make_shrinkable<std::vector<int>>(0);
    auto shr4 = make_shrinkable<std::function<int(int)>>([](int a) {
        return a+5;
    });

    auto shr5 = make_shrinkable<std::map<std::string, std::string>>();
}
