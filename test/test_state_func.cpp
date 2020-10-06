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
        return [value](T& obj) {
            auto size = obj.size();
            obj.push_back(value);
            PROP_ASSERT(obj.size() == size + 1);
            return true;
        };
    });

    // actionGen<T> is shorthand for just<SimpleAction<T>>
    auto popBackGen = just<SimpleAction<T>>([](T& obj) {
        auto size = obj.size();
        if (obj.empty())
            return true;
        obj.pop_back();
        PROP_ASSERT(obj.size() == size - 1);
        return true;
    });

    auto popBackGen2 = just<SimpleAction<T>>([](T& obj) {
        auto size = obj.size();
        if (obj.empty())
            return true;
        obj.pop_back();
        PROP_ASSERT(obj.size() == size - 1);
        return true;
    });

    // actionGen<T> is shorthand for just<SimpleAction<T>>
    auto clearGen = just<SimpleAction<T>>([](T& obj) {
        // std::cout << "Clear" << std::endl;
        obj.clear();
        PROP_ASSERT(obj.size() == 0);
        return true;
    });

    auto actionsGen = actionListGenOf<T>(pushBackGen, popBackGen, popBackGen2, clearGen);
    auto prop = statefulProperty<T>(Arbi<T>(), actionsGen);
    prop.forAll();
}

TEST(StateTest, StateFunctionWithModel)
{
    using T = std::vector<int>;
    using Model = VectorModel2;

    auto pushBackGen = Arbi<int>().map<Action<T, Model>>([](int value) {
        return [value](T& obj, Model&) {
            auto size = obj.size();
            obj.push_back(value);
            PROP_ASSERT(obj.size() == size + 1);
            return true;
        };
    });

    // actionGen<T> is shorthand for just<SimpleAction<T>>
    auto popBackGen = just<Action<T, Model>>([](T& obj, Model&) {
        auto size = obj.size();
        if (obj.empty())
            return true;
        obj.pop_back();
        PROP_ASSERT(obj.size() == size - 1);
        return true;
    });

    auto popBackGen2 = just<Action<T, Model>>([](T& obj, Model&) {
        auto size = obj.size();
        if (obj.empty())
            return true;
        obj.pop_back();
        PROP_ASSERT(obj.size() == size - 1);
        return true;
    });

    // actionGen<T> is shorthand for just<SimpleAction<T>>
    auto clearGen = just<Action<T, Model>>([](T& obj, Model&) {
        // std::cout << "Clear" << std::endl;
        obj.clear();
        PROP_ASSERT(obj.size() == 0);
        return true;
    });

    auto actionsGen = actionListGenOf<T, Model>(pushBackGen, popBackGen, popBackGen2, clearGen);
    auto prop = statefulProperty<T, Model, GenFunction<T>>(
        Arbi<T>(), [](T& obj) -> Model { return VectorModel2(obj.size()); }, actionsGen);
    prop.forAll();
}
