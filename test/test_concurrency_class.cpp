#include "statefultest.hpp"
#include "googletest/googletest/include/gtest/gtest.h"
#include "googletest/googlemock/include/gmock/gmock.h"
#include "Random.hpp"
#include "../combinator/concurrency_class.hpp"
#include "../util/std.hpp"

using namespace proptest;
using namespace proptest::concurrent::alt;

using std::mutex;
using std::lock_guard;

class ConcurrencyAltTest : public ::testing::Test {
};

struct VectorAction3 : public SimpleAction<vector<int>>
{
};

mutex& getMutex()
{
    static mutex mtx;
    return mtx;
}

struct PushBack3 : public VectorAction3
{
    PushBack3(int value) : value(value) {}

    virtual bool run(vector<int>& system)
    {
        // cout << "PushBack(" << value << ")" << endl;
        lock_guard<mutex> guard(getMutex());
        system.push_back(value);
        return true;
    }

    int value;
};

struct Clear3 : public VectorAction3
{
    virtual bool run(vector<int>& system)
    {
        // cout << "Clear" << endl;
        lock_guard<mutex> guard(getMutex());
        system.clear();
        return true;
    }
};

struct PopBack3 : public VectorAction3
{
    virtual bool run(vector<int>& system)
    {
        // cout << "PopBack" << endl;
        lock_guard<mutex> guard(getMutex());
        if (system.empty())
            return true;

        system.pop_back();
        return true;
    }
};

TEST(ConcurrencyAltTest, States)
{
    auto pushBackActionGen =
        Arbi<int>().map<shared_ptr<VectorAction3>>([](int& value) { return make_shared<PushBack3>(value); });
    auto popBackActionGen = lazy<shared_ptr<VectorAction3>>([]() { return make_shared<PopBack3>(); });
    auto clearActionGen = lazy<shared_ptr<VectorAction3>>([]() { return make_shared<Clear3>(); });

    auto actionListGen = actionListGenOf<VectorAction3>(pushBackActionGen, popBackActionGen, clearActionGen);

    auto prop = concurrency<VectorAction3>(Arbi<vector<int>>(), actionListGen);
    prop.go();
}
