#include "proptest.hpp"

#include "googletest/googletest/include/gtest/gtest.h"
#include "googletest/googlemock/include/gmock/gmock.h"

using namespace pbt;

TEST(Compile, Stream)
{
    auto str1 = Stream<int>::one(1);
    auto str2 = Stream<std::string>::one("hello");
    auto str3 = Stream<std::vector<int>>::one(std::vector<int>());
    auto str4 = Stream<std::function<int(int)>>::one([](int a) {
        return a+76;
    });

    auto sht5 = Stream<std::map<std::string, std::string>>(std::map<std::string, std::string>());
}
