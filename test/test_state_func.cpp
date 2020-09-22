#include "proptest.hpp"
#include "googletest/googletest/include/gtest/gtest.h"
#include "googletest/googlemock/include/gmock/gmock.h"
#include "Random.hpp"
#include <chrono>
#include <iostream>
#include <memory>

using namespace proptest;

TEST(StateTest, States2)
{
    using T = std::vector<int>;

    auto pushBackGen = Arbi<int>().map<Action<T>>([](int value) {
        return [value](T& obj) {
            auto size = obj.size();
            obj.push_back(value);
            PROP_ASSERT(obj.size() == size + 1);
            return true;
        };
    });

    auto popBackGen = actionGen<T>([](T& obj) {
        auto size = obj.size();
        if (obj.empty())
            return true;
        obj.pop_back();
        PROP_ASSERT(obj.size() == size - 1);
        return true;
    });

    auto popBackGen2 = just<Action<T>>([](T& obj) {
        auto size = obj.size();
        if (obj.empty())
            return true;
        obj.pop_back();
        PROP_ASSERT(obj.size() == size - 1);
        return true;
    });

    auto clearGen = actionGen<T>([](T& obj) {
        // std::cout << "Clear" << std::endl;
        obj.clear();
        PROP_ASSERT(obj.size() == 0);
        return true;
    });

    auto prop = actionProperty<T>(pushBackGen, popBackGen, popBackGen2, clearGen);
    prop.forAll();
}
