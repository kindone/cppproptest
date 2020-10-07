#include "statefultest.hpp"
#include "googletest/googletest/include/gtest/gtest.h"
#include "googletest/googlemock/include/gmock/gmock.h"
#include "Random.hpp"
#include "../combinator/concurrency_function.hpp"
#include <chrono>
#include <iostream>
#include <memory>
#include <mutex>

using namespace proptest;
using namespace proptest::concurrent;

extern std::mutex& getMutex();

class ConcurrencyTest : public ::testing::Test {
public:
};

TEST(ConcurrencyTest, WithoutModel)
{
    auto pushBackGen = Arbi<int>().map<SimpleAction<std::vector<int>>>([](int& value) {
        return [value](std::vector<int>& obj) {
            // std::cout << "PushBack(" << value << ")" << std::endl;
            std::lock_guard<std::mutex> guard(getMutex());
            obj.push_back(value);
            return true;
        };
    });

    auto popBackGen = just<SimpleAction<std::vector<int>>>([](std::vector<int>& obj) {
        std::lock_guard<std::mutex> guard(getMutex());
        if (obj.empty())
            return true;
        obj.pop_back();
        return true;
    });

    auto clearGen = just<SimpleAction<std::vector<int>>>([](std::vector<int>& obj) {
        std::lock_guard<std::mutex> guard(getMutex());
        obj.clear();
        return true;
    });

    auto actionListGen = actionListGenOf<std::vector<int>>(pushBackGen, popBackGen, clearGen);

    auto prop = concurrency<std::vector<int>>(Arbi<std::vector<int>>(), actionListGen);
    prop.check();
}

TEST(ConcurrencyTest, WithModel)
{
    struct Model
    {
    };

    auto pushBackGen = Arbi<int>().map<Action<std::vector<int>, Model>>([](int& value) {
        return [value](std::vector<int>& obj, Model&) {
            // std::cout << "PushBack(" << value << ")" << std::endl;
            std::lock_guard<std::mutex> guard(getMutex());
            obj.push_back(value);
            return true;
        };
    });

    auto popBackGen = just<Action<std::vector<int>, Model>>([](std::vector<int>& obj, Model&) {
        std::lock_guard<std::mutex> guard(getMutex());
        if (obj.empty())
            return true;
        obj.pop_back();
        return true;
    });

    auto clearGen = just<Action<std::vector<int>, Model>>([](std::vector<int>& obj, Model&) {
        std::lock_guard<std::mutex> guard(getMutex());
        obj.clear();
        return true;
    });

    auto actionListGen = actionListGenOf<std::vector<int>, Model>(pushBackGen, popBackGen, clearGen);

    auto prop = concurrency<std::vector<int>, Model>(
        Arbi<std::vector<int>>(), [](std::vector<int>&) { return Model(); }, actionListGen);
    prop.check();
}
