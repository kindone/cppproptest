#include "proptest.hpp"

#include "googletest/googletest/include/gtest/gtest.h"
#include "googletest/googlemock/include/gmock/gmock.h"

using namespace proptest;

TEST(Compile, Property)
{
    auto prop = property([](int i, float f) {
        std::cout << "matrix: " << i << ", " << f << std::endl;
    });

    prop.matrix({1,2,3}, {0.2f, 0.3f});

    forAll([](std::vector<int64_t>) {

    });
}
