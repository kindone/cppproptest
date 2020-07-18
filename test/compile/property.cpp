#include "proptest.hpp"

#include "googletest/googletest/include/gtest/gtest.h"
#include "googletest/googlemock/include/gmock/gmock.h"

using namespace proptest;

TEST(Compile, Property)
{
    property([](int) {

    });

    forAll([](std::vector<int64_t>) {

    });
}
