#include "statefultest.hpp"
#include "googletest/googletest/include/gtest/gtest.h"
#include "googletest/googlemock/include/gmock/gmock.h"
#include "Random.hpp"
#include "../combinator/concurrency_class.hpp"
#include "../util/bitmap.hpp"
#include <chrono>
#include <iostream>

using namespace proptest;
using namespace proptest::concurrent::alt;

class ConcurrencyTestAlt2 : public ::testing::Test {
};

TEST_F(ConcurrencyTestAlt2, bitmap_internal)
{
    util::Bitmap bitmap;
    for (int i = 0; i < util::Bitmap::size; i++) {
        EXPECT_NE(bitmap.occupyAvailable(0), -1);
    }
    bitmap.reset();
    util::Bitmap copy = bitmap;
    for (int i = 0; i < util::Bitmap::size; i++) {
        EXPECT_NE(copy.occupyAvailable(0), -1);
    }
    std::cout << bitmap.occupyAvailable(0) << std::endl;
    std::cout << bitmap.states[0] << ", " << bitmap.states[1] << std::endl;
    std::cout << bitmap.occupyAvailable(0) << std::endl;
    std::cout << bitmap.states[0] << ", " << bitmap.states[1] << ", " << bitmap.states[2] << std::endl;
    std::cout << bitmap.occupyUnavailable(2) << std::endl;
}

TEST_F(ConcurrencyTestAlt2, bitmap)
{
    util::Bitmap bitmap;
    for (int i = 0; i < util::Bitmap::size; i++) {
        EXPECT_NE(bitmap.acquire(), -1);
    }
    bitmap.reset();
    util::Bitmap copy = bitmap;
    int n = -1;
    for (int i = 0; i < util::Bitmap::size; i++) {
        EXPECT_NE((n = copy.acquire()), -1);
    }
    EXPECT_EQ(copy.tryAcquire(), -1);
    copy.unacquire(n);
    EXPECT_NE(copy.tryAcquire(), -1);
    EXPECT_EQ(copy.tryAcquire(), -1);
}

TEST_F(ConcurrencyTestAlt2, Container) {}

struct VectorAction4 : public Action<std::vector<int>, util::Bitmap>
{
};

extern std::mutex& getMutex();

struct PushBack4 : public VectorAction4
{
    PushBack4(int value) : value(value) {}

    virtual bool run(std::vector<int>& system, util::Bitmap& bitmap)
    {

        // std::cout << "PushBack(" << value << ")" << std::endl;
        std::lock_guard<std::mutex> guard(getMutex());
        int pos = bitmap.acquire();
        system.push_back(value);
        bitmap.unacquire(pos);
        return true;
    }

    int value;
};

struct Clear4 : public VectorAction4
{
    virtual bool run(std::vector<int>& system, util::Bitmap& bitmap)
    {
        // std::cout << "Clear" << std::endl;
        std::lock_guard<std::mutex> guard(getMutex());
        int pos = bitmap.acquire();
        system.clear();
        bitmap.unacquire(pos);
        return true;
    }
};

struct PopBack4 : public VectorAction4
{
    virtual bool run(std::vector<int>& system, util::Bitmap& bitmap)
    {
        // std::cout << "PopBack" << std::endl;
        std::lock_guard<std::mutex> guard(getMutex());
        if (system.empty())
            return true;

        int pos = bitmap.acquire();
        system.pop_back();
        bitmap.unacquire(pos);
        return true;
    }
};

TEST_F(ConcurrencyTestAlt2, WithModel)
{
    auto pushBackActionGen =
        Arbi<int>().map<std::shared_ptr<VectorAction4>>([](int& value) { return std::make_shared<PushBack4>(value); });
    auto popBackActionGen = lazy<std::shared_ptr<VectorAction4>>([]() { return std::make_shared<PopBack4>(); });
    auto clearActionGen = lazy<std::shared_ptr<VectorAction4>>([]() { return std::make_shared<Clear4>(); });

    auto actionListGen = actionListGenOf<VectorAction4>(pushBackActionGen, popBackActionGen, clearActionGen);

    auto prop = concurrency<VectorAction4>(Arbi<std::vector<int>>(), actionListGen);
    prop.go();
}
