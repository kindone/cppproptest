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
    for (int i = 0; i < Bitmap::size; i++) {
        EXPECT_NE(bitmap.occupyAvailable(0), -1);
    }
    bitmap.reset();
    Bitmap copy = bitmap;
    for (int i = 0; i < Bitmap::size; i++) {
        EXPECT_NE(copy.occupyAvailable(0), -1);
    }
    std::cout << bitmap.occupyAvailable(0) << std::endl;
    std::cout << bitmap.states[0] << ", " << bitmap.states[1] << std::endl;
    std::cout << bitmap.occupyAvailable(0) << std::endl;
    std::cout << bitmap.states[0] << ", " << bitmap.states[1] << ", " << bitmap.states[2] << std::endl;
    std::cout << bitmap.occupyUnavailable(2) << std::endl;
}

TEST(ConcurrencyTest2, Container) {}

struct VectorAction4 : public Action<std::vector<int>, Bitmap>
{
};

extern std::mutex& getMutex();

struct PushBack4 : public VectorAction4
{
    PushBack4(int value) : value(value) {}

    virtual bool run(std::vector<int>& system, Bitmap&)
    {
        // std::cout << "PushBack(" << value << ")" << std::endl;
        std::lock_guard<std::mutex> guard(getMutex());
        system.push_back(value);
        return true;
    }

    int value;
};

struct Clear4 : public VectorAction4
{
    virtual bool run(std::vector<int>& system, Bitmap&)
    {
        // std::cout << "Clear" << std::endl;
        std::lock_guard<std::mutex> guard(getMutex());
        system.clear();
        return true;
    }
};

struct PopBack4 : public VectorAction4
{
    virtual bool run(std::vector<int>& system, Bitmap&)
    {
        // std::cout << "PopBack" << std::endl;
        std::lock_guard<std::mutex> guard(getMutex());
        if (system.empty())
            return true;

        system.pop_back();
        return true;
    }
};

TEST(ConcurrencyTest2, WithModel)
{
    auto pushBackActionGen = transform<int, std::shared_ptr<VectorAction4>>(
        Arbitrary<int>(), [](const int& value) { return std::make_shared<PushBack4>(value); });
    auto popBackActionGen = just<std::shared_ptr<VectorAction4>>([]() { return std::make_shared<PopBack4>(); });
    auto clearActionGen = just<std::shared_ptr<VectorAction4>>([]() { return std::make_shared<Clear4>(); });

    auto actionsGen = actions<VectorAction4>(pushBackActionGen, popBackActionGen, clearActionGen);

    auto prop = concurrency<VectorAction4>(Arbitrary<std::vector<int>>(), actionsGen);
    prop.check();
}
