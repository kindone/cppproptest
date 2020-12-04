#include "statefultest.hpp"
#include "googletest/googletest/include/gtest/gtest.h"
#include "googletest/googlemock/include/gmock/gmock.h"
#include "Random.hpp"
#include "../util/bitmap.hpp"
#include "../combinator/concurrency_function.hpp"
#include "../util/std.hpp"
#include <mutex>

using namespace proptest;
using namespace proptest::concurrent;

using std::mutex;
using std::lock_guard;

extern mutex& getMutex();

class ConcurrencyTest : public ::testing::Test {
public:
};

TEST(ConcurrencyTest, WithoutModel)
{
    auto pushBackGen = Arbi<int>().map<SimpleAction<vector<int>>>([](int& value) {
        return SimpleAction<vector<int>>([value](vector<int>& obj) {
            // cout << "PushBack(" << value << ")" << endl;
            lock_guard<mutex> guard(getMutex());
            obj.push_back(value);
        });
    });

    auto popBackGen = just(SimpleAction<vector<int>>([](vector<int>& obj) {
        lock_guard<mutex> guard(getMutex());
        if (obj.empty())
            return;
        obj.pop_back();
    }));

    auto clearGen = just(SimpleAction<vector<int>>([](vector<int>& obj) {
        lock_guard<mutex> guard(getMutex());
        obj.clear();
    }));

    auto actionListGen = oneOf<SimpleAction<vector<int>>>(pushBackGen, popBackGen, clearGen);

    auto prop = concurrency<vector<int>>(Arbi<vector<int>>(), actionListGen);
    prop.go();
}

TEST(ConcurrencyTest, WithModel)
{
    struct Model
    {
    };

    auto pushBackGen = Arbi<int>().map<Action<vector<int>, Model>>([](int& value) {
        return Action<vector<int>, Model>([value](vector<int>& obj, Model&) {
            // cout << "PushBack(" << value << ")" << endl;
            lock_guard<mutex> guard(getMutex());
            obj.push_back(value);
        });
    });

    auto popBackGen = just(Action<vector<int>, Model>([](vector<int>& obj, Model&) {
        lock_guard<mutex> guard(getMutex());
        if (obj.empty())
            return;
        obj.pop_back();
    }));

    auto clearGen = just(Action<vector<int>, Model>([](vector<int>& obj, Model&) {
        lock_guard<mutex> guard(getMutex());
        obj.clear();
    }));

    auto actionListGen = oneOf<Action<vector<int>, Model>>(pushBackGen, popBackGen, clearGen);

    auto prop = concurrency<vector<int>, Model>(
        Arbi<vector<int>>(), [](vector<int>&) { return Model(); }, actionListGen);
    prop.setMaxConcurrency(2);
    prop.go();
}

TEST(ConcurrencyTest, bitmap)
{
    using Bitmap = util::Bitmap;

    auto acquireGen = just(SimpleAction<Bitmap>("Acquire", [](Bitmap& bitmap) {
        [[maybe_unused]] int pos = bitmap.acquire();
        bitmap.unacquire(pos);
    }));

    [[maybe_unused]] auto unacquireGen = integers<int>(0, Bitmap::size).map<SimpleAction<Bitmap>>(+[](int& pos) {
        return SimpleAction<Bitmap>("Unacquire", [pos](Bitmap& bitmap) {
            try {
                bitmap.unacquire(pos);
                cout << "unacquired" << endl;
            } catch(runtime_error&) {
                cout << "failed to unacquire" << endl;
            }
        });
    });

    auto actionGen = oneOf<SimpleAction<Bitmap>>(acquireGen/*, unacquireGen*/);
    auto prop = concurrency<Bitmap>(
        just<Bitmap>(Bitmap()), actionGen);
    prop.go();
}
