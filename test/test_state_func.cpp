#include "statefultest.hpp"
#include "googletest/googletest/include/gtest/gtest.h"
#include "googletest/googlemock/include/gmock/gmock.h"
#include "Random.hpp"
#include <chrono>
#include <iostream>
#include <memory>

using namespace proptest;
using namespace proptest::stateful;

struct VectorModel2
{
    VectorModel2(int size) : size(size) {}
    int size;
};

TEST(StateTest, StateFunction)
{
    using T = std::vector<int>;

    auto pushBackGen = Arbi<int>().map<SimpleAction<T>>([](int value) {
        return SimpleAction<T>([value](T& obj) {
            auto size = obj.size();
            obj.push_back(value);
            PROP_ASSERT(obj.size() == size + 1);
        });
    });

    // actionGen<T> is shorthand for just<SimpleAction<T>>
    auto popBackGen = just(SimpleAction<T>([](T& obj) {
        auto size = obj.size();
        if (obj.empty())
            return;
        obj.pop_back();
        PROP_ASSERT(obj.size() == size - 1);
    }));

    auto popBackGen2 = just(SimpleAction<T>([](T& obj) {
        auto size = obj.size();
        if (obj.empty())
            return;
        obj.pop_back();
        PROP_ASSERT(obj.size() == size - 1);
    }));

    // actionGen<T> is shorthand for just<SimpleAction<T>>
    auto clearGen = just(SimpleAction<T>([](T& obj) {
        // std::cout << "Clear" << std::endl;
        obj.clear();
        PROP_ASSERT(obj.size() == 0);
    }));

    auto actionListGen =
        actionListGenOf<T>(pushBackGen, popBackGen, popBackGen2, weightedGen<SimpleAction<T>>(clearGen, 0.1));
    auto prop = statefulProperty<T>(Arbi<T>(), actionListGen);
    prop.setOnStartup([]() {
        std::cout << "startup" << std::endl;
    });
    prop.setOnCleanup([]() {
        std::cout << "cleanup" << std::endl;
    });
    prop.setSeed(0).setNumRuns(100).go();
}

TEST(StateTest, StateFunctionWithModel)
{
    using T = std::vector<int>;
    using Model = VectorModel2;

    auto pushBackGen = Arbi<int>().map<Action<T, Model>>([](int value) {
        std::stringstream str;
        str << "PushBack(" << value << ")";
        return Action<T, Model>(str.str(), [value](T& obj, Model&) {
            auto size = obj.size();
            obj.push_back(value);
            PROP_ASSERT(obj.size() == size + 1);
        });
    });

    auto popBackGen = just(Action<T, Model>("PopBack", [](T& obj, Model&) {
        auto size = obj.size();
        if (obj.empty())
            return;
        obj.pop_back();
        PROP_ASSERT(obj.size() == size - 1);
    }));

    auto popBackGen2 = just(Action<T, Model>("PopBack2",[](T& obj, Model&) {
        auto size = obj.size();
        if (obj.empty())
            return;
        obj.pop_back();
        PROP_ASSERT(obj.size() == size - 1);
    }));

    auto clearGen = just(Action<T, Model>("Clear", [](T& obj, Model&) {
        obj.clear();
        PROP_ASSERT(obj.size() == 0);
    }));

    auto actionListGen = actionListGenOf<T, Model>(pushBackGen, popBackGen, popBackGen2, clearGen);
    auto prop = statefulProperty<T, Model>(
        Arbi<T>(), [](T& obj) -> Model { return VectorModel2(obj.size()); }, actionListGen);
    prop.setOnStartup([]() {
        std::cout << "startup" << std::endl;
    });
    prop.setOnCleanup([]() {
        std::cout << "cleanup" << std::endl;
        PROP_ASSERT(false);
    });
    prop.setSeed(0).setNumRuns(10).go();
}
