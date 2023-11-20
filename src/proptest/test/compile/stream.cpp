#include "proptest/proptest.hpp"
#include "googletest/googletest/include/gtest/gtest.h"
#include "googletest/googlemock/include/gmock/gmock.h"

using namespace proptest;

TEST(Compile, Stream)
{
    auto str1 = Stream::one(1);
    auto str2 = Stream::one<string>("hello");
    auto str3 = Stream::one(vector<int>());
    auto str4 = Stream::one<function<int(int)>>([](int a) { return a + 76; });

    auto sht5 = Stream(map<string, string>());
}
