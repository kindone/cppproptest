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

struct VectorAction3 : public ActionWithoutModel<std::vector<int>>
{
};

struct PushBack3 : public VectorAction3
{
    PushBack3(int value) : value(value) {}

    virtual bool run(std::vector<int>& system)
    {
        // std::cout << "PushBack(" << value << ")" << std::endl;
        system.push_back(value);
        return true;
    }

    int value;
};

struct Clear3 : public VectorAction3
{
    virtual bool run(std::vector<int>& system)
    {
        // std::cout << "Clear" << std::endl;
        system.clear();
        return true;
    }
};

struct PopBack3 : public VectorAction3
{
    virtual bool run(std::vector<int>& system)
    {
        // std::cout << "PopBack" << std::endl;
        if (system.empty())
            return true;
        system.pop_back();
        return true;
    }
};

TEST(ConcurrencyTest, States)
{
    auto pushBackActionGen = transform<int, std::shared_ptr<VectorAction3>>(
        Arbitrary<int>(), [](const int& value) { return std::make_shared<PushBack3>(value); });
    auto popBackActionGen = just<std::shared_ptr<VectorAction3>>([]() { return std::make_shared<PopBack3>(); });
    auto clearActionGen = just<std::shared_ptr<VectorAction3>>([]() { return std::make_shared<Clear3>(); });

    auto actionsGen = actions<VectorAction3>(pushBackActionGen, popBackActionGen, clearActionGen);

    auto prop = concurrency<VectorAction3>(Arbitrary<std::vector<int>>(), actionsGen);
    prop.forAll();
}
