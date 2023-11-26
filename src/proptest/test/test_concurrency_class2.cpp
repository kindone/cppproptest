#include "proptest/statefultest.hpp"
#include "googletest/googletest/include/gtest/gtest.h"
#include "googletest/googlemock/include/gmock/gmock.h"
#include "proptest/stateful/concurrency_class.hpp"


using namespace proptest;
using namespace proptest::concurrent::alt;

using std::mutex;
using std::lock_guard;

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
    cout << bitmap.occupyAvailable(0) << endl;
    cout << bitmap.states[0] << ", " << bitmap.states[1] << endl;
    cout << bitmap.occupyAvailable(0) << endl;
    cout << bitmap.states[0] << ", " << bitmap.states[1] << ", " << bitmap.states[2] << endl;
    cout << bitmap.occupyUnavailable(2) << endl;
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

struct VectorAction4 : public Action<vector<int>, util::Bitmap>
{
};

extern mutex& getMutex();

struct PushBack4 : public VectorAction4
{
    PushBack4(int value) : value(value) {}

    virtual bool run(vector<int>& system, util::Bitmap& bitmap)
    {

        // cout << "PushBack(" << value << ")" << endl;
        lock_guard<mutex> guard(getMutex());
        int pos = bitmap.acquire();
        system.push_back(value);
        bitmap.unacquire(pos);
        return true;
    }

    int value;
};

struct Clear4 : public VectorAction4
{
    virtual bool run(vector<int>& system, util::Bitmap& bitmap)
    {
        // cout << "Clear" << endl;
        lock_guard<mutex> guard(getMutex());
        int pos = bitmap.acquire();
        system.clear();
        bitmap.unacquire(pos);
        return true;
    }
};

struct PopBack4 : public VectorAction4
{
    virtual bool run(vector<int>& system, util::Bitmap& bitmap)
    {
        // cout << "PopBack" << endl;
        lock_guard<mutex> guard(getMutex());
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
        Arbi<int>().map<shared_ptr<VectorAction4>>([](int& value) { return util::make_shared<PushBack4>(value); });
    auto popBackActionGen = lazy<shared_ptr<VectorAction4>>([]() { return util::make_shared<PopBack4>(); });
    auto clearActionGen = lazy<shared_ptr<VectorAction4>>([]() { return util::make_shared<Clear4>(); });

    auto actionListGen = actionListGenOf<VectorAction4>(pushBackActionGen, popBackActionGen, clearActionGen);

    auto prop = concurrency<VectorAction4>(Arbi<vector<int>>(), actionListGen);
    prop.go();
}
