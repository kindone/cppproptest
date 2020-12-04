#include "statefultest.hpp"
#include "googletest/googletest/include/gtest/gtest.h"
#include "googletest/googlemock/include/gmock/gmock.h"
#include "Random.hpp"
#include "../util/std.hpp"

using namespace proptest;
using namespace proptest::stateful::alt;

class StateTestCase : public ::testing::Test {
};

struct VectorAction : public SimpleAction<vector<int>>
{
};

struct PushBack : public VectorAction
{
    PushBack(int value) : value(value) {}

    virtual bool run(vector<int>& system)
    {
        cout << "PushBack(" << value << ")" << endl;
        auto size = system.size();
        system.push_back(value);
        PROP_ASSERT(system.size() == size + 1);
        return true;
    }

    int value;
};

struct Clear : public VectorAction
{
    virtual bool run(vector<int>& system)
    {
        cout << "Clear" << endl;
        system.clear();
        PROP_ASSERT(system.size() == 0);
        return true;
    }
};

struct PopBack : public VectorAction
{
    virtual bool run(vector<int>& system)
    {
        cout << "PopBack" << endl;
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
        Arbi<int>().map<shared_ptr<VectorAction>>([](int& value) { return make_shared<PushBack>(value); }),
        just<shared_ptr<VectorAction>>(make_shared<PopBack>()),
        just<shared_ptr<VectorAction>>(make_shared<Clear>()));

    auto prop = statefulProperty<VectorAction>(Arbi<vector<int>>(), actionListGen);
    prop.go();
}

struct VectorModel
{
    VectorModel(int size) : size(size) {}
    int size;
};

struct VectorAction2 : public Action<vector<int>, VectorModel>
{
};

struct PushBack2 : public VectorAction2
{
    PushBack2(int value) : value(value) {}

    virtual bool run(vector<int>& system, VectorModel& model)
    {
        cout << "PushBack(" << value << ")" << endl;
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
    virtual bool run(vector<int>& system, VectorModel& model)
    {
        cout << "Clear" << endl;
        system.clear();
        model.size = 0;
        PROP_ASSERT(model.size == system.size());
        return true;
    }
};

struct PopBack2 : public VectorAction2
{
    virtual bool run(vector<int>& system, VectorModel& model)
    {
        cout << "PopBack" << endl;
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
        Arbi<int>().map<shared_ptr<VectorAction2>>([](int& value) { return make_shared<PushBack2>(value); }),
        just<shared_ptr<VectorAction2>>(make_shared<PopBack2>()),
        just<shared_ptr<VectorAction2>>(make_shared<Clear2>()));

    auto prop = statefulProperty<VectorAction2>(
        Arbi<vector<int>>(), [](vector<int>& sys) { return VectorModel(sys.size()); }, actionListGen);
    prop.go();
}

TEST(StateTest, StatesWithModel2)
{
    auto actionListGen = actionListGenOf<VectorAction2>(
        Arbi<int>().map<shared_ptr<VectorAction2>>([](int& value) { return make_shared<PushBack2>(value); }),
        Arbi<int>().map<VectorAction2*>([](int& value) { return new PushBack2(value); }),
        lazy<VectorAction2*>([]() { return new PopBack2(); }), lazy<VectorAction2*>([]() { return new Clear2(); }));

    auto prop = statefulProperty<VectorAction2>(
        Arbi<vector<int>>(), [](vector<int>& sys) { return VectorModel(sys.size()); }, actionListGen);
    prop.go();
}
