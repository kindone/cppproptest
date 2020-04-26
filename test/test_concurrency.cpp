#include "proptest.hpp"
#include "googletest/googletest/include/gtest/gtest.h"
#include "googletest/googlemock/include/gmock/gmock.h"
#include "Random.hpp"
#include "../combinator/concurrency.hpp"
#include <chrono>
#include <iostream>
#include <memory>

using namespace PropertyBasedTesting;

class ConcurrencyTest : public ::testing::Test {
};

struct VectorAction : public ActionWithoutModel<std::vector<int>>
{
};

struct PushBack : public VectorAction
{
    PushBack(int value) : value(value) {}

    virtual bool run(std::vector<int>& system)
    {
        std::cout << "PushBack(" << value << ")" << std::endl;
        system.push_back(value);
        return true;
    }

    int value;
};

struct Clear : public VectorAction
{
    virtual bool run(std::vector<int>& system)
    {
        std::cout << "Clear" << std::endl;
        system.clear();
        return true;
    }
};

struct PopBack : public VectorAction
{
    virtual bool run(std::vector<int>& system)
    {
        std::cout << "PopBack" << std::endl;
        if (system.empty())
            return true;
        system.pop_back();
        return true;
    }
};

TEST(ConcurrencyTest, States)
{
    auto pushBackActionGen = transform<int, std::shared_ptr<VectorAction>>(
        Arbitrary<int>(), [](const int& value) { return std::make_shared<PushBack>(value); });
    auto popBackActionGen = just<std::shared_ptr<VectorAction>>([]() { return std::make_shared<PopBack>(); });
    auto clearActionGen = just<std::shared_ptr<VectorAction>>([]() { return std::make_shared<Clear>(); });

    auto actionsGen = actions<VectorAction>(pushBackActionGen, popBackActionGen, clearActionGen);

    auto prop = concurrency<VectorAction>(Arbitrary<std::vector<int>>(), actionsGen);
    prop.check();
}
