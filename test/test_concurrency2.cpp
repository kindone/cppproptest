#include "proptest.hpp"
#include "googletest/googletest/include/gtest/gtest.h"
#include "googletest/googlemock/include/gmock/gmock.h"
#include "Random.hpp"
#include "../combinator/concurrency.hpp"
#include "../util/bitmap.hpp"
#include <chrono>
#include <iostream>


using namespace PropertyBasedTesting;

class ConcurrencyTest2 : public ::testing::Test {
};

TEST(ConcurrencyTest2, bitmap)
{
    Bitmap bitmap;
    std::cout << bitmap.occupyAvailable(0) << std::endl;
    std::cout << bitmap.states[0] << ", " << bitmap.states[1] << std::endl;
    std::cout << bitmap.occupyAvailable(0) << std::endl;
    std::cout << bitmap.states[0] << ", " << bitmap.states[1] << ", " << bitmap.states[2] << std::endl;
    std::cout << bitmap.occupyUnavailable(2) << std::endl;
}

TEST(ConcurrencyTest2, Container) {}