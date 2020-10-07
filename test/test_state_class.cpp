#include "statefultest.hpp"
#include "googletest/googletest/include/gtest/gtest.h"
#include "googletest/googlemock/include/gmock/gmock.h"
#include "Random.hpp"
#include <chrono>
#include <iostream>
#include <memory>

using namespace proptest;
using namespace proptest::stateful::alt;

class StateTestCase : public ::testing::Test {
};

struct VectorAction : public SimpleAction<std::vector<int>>
{
};

struct PushBack : public VectorAction
{
    PushBack(int value) : value(value) {}

    virtual bool run(std::vector<int>& system)
    {
        std::cout << "PushBack(" << value << ")" << std::endl;
        auto size = system.size();
        system.push_back(value);
        PROP_ASSERT(system.size() == size + 1);
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
        PROP_ASSERT(system.size() == 0);
        return true;
    }
};

struct PopBack : public VectorAction
{
    virtual bool run(std::vector<int>& system)
    {
        std::cout << "PopBack" << std::endl;
        auto size = system.size();
        if (system.empty())
            return true;
        system.pop_back();
        PROP_ASSERT(system.size() == size - 1);
        return true;
    }
};

TEST(StateTest, States)
{
    auto actionListGen = actionListGenOf<VectorAction>(
        Arbi<int>().map<std::shared_ptr<VectorAction>>([](int& value) { return std::make_shared<PushBack>(value); }),
        just<std::shared_ptr<VectorAction>>(std::make_shared<PopBack>()),
        just<std::shared_ptr<VectorAction>>(std::make_shared<Clear>()));

    auto prop = statefulProperty<VectorAction>(Arbi<std::vector<int>>(), actionListGen);
    prop.forAll();
}

struct VectorModel
{
    VectorModel(int size) : size(size) {}
    int size;
};

struct VectorAction2 : public Action<std::vector<int>, VectorModel>
{
};

struct PushBack2 : public VectorAction2
{
    PushBack2(int value) : value(value) {}

    virtual bool run(std::vector<int>& system, VectorModel& model)
    {
        std::cout << "PushBack(" << value << ")" << std::endl;
        auto size = system.size();
        system.push_back(value);
        model.size++;
        PROP_ASSERT(size + 1 == system.size());
        PROP_ASSERT(model.size == system.size());
        return true;
    }

    int value;
};

struct Clear2 : public VectorAction2
{
    virtual bool run(std::vector<int>& system, VectorModel& model)
    {
        std::cout << "Clear" << std::endl;
        system.clear();
        model.size = 0;
        PROP_ASSERT(model.size == system.size());
        return true;
    }
};

struct PopBack2 : public VectorAction2
{
    virtual bool run(std::vector<int>& system, VectorModel& model)
    {
        std::cout << "PopBack" << std::endl;
        if (system.empty())
            return true;
        system.pop_back();
        model.size--;
        PROP_ASSERT(model.size == system.size());
        return true;
    }
};

TEST(StateTest, StatesWithModel)
{
    auto actionListGen = actionListGenOf<VectorAction2>(
        Arbi<int>().map<std::shared_ptr<VectorAction2>>([](int& value) { return std::make_shared<PushBack2>(value); }),
        just<std::shared_ptr<VectorAction2>>(std::make_shared<PopBack2>()),
        just<std::shared_ptr<VectorAction2>>(std::make_shared<Clear2>()));

    auto prop = statefulProperty<VectorAction2>(
        Arbi<std::vector<int>>(), [](std::vector<int>& sys) { return VectorModel(sys.size()); }, actionListGen);
    prop.forAll();
}

TEST(StateTest, StatesWithModel2)
{
    auto actionListGen = actionListGenOf<VectorAction2>(
        Arbi<int>().map<std::shared_ptr<VectorAction2>>([](int& value) { return std::make_shared<PushBack2>(value); }),
        Arbi<int>().map<VectorAction2*>([](int& value) { return new PushBack2(value); }),
        lazy<VectorAction2*>([]() { return new PopBack2(); }), lazy<VectorAction2*>([]() { return new Clear2(); }));

    auto prop = statefulProperty<VectorAction2>(
        Arbi<std::vector<int>>(), [](std::vector<int>& sys) { return VectorModel(sys.size()); }, actionListGen);
    prop.forAll();
}
