#include "statefultest.hpp"
#include "googletest/googletest/include/gtest/gtest.h"
#include "googletest/googlemock/include/gmock/gmock.h"
#include "Random.hpp"
#include "../util/bitmap.hpp"
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
        return SimpleAction<std::vector<int>>([value](std::vector<int>& obj) {
            // std::cout << "PushBack(" << value << ")" << std::endl;
            std::lock_guard<std::mutex> guard(getMutex());
            obj.push_back(value);
        });
    });

    auto popBackGen = just(SimpleAction<std::vector<int>>([](std::vector<int>& obj) {
        std::lock_guard<std::mutex> guard(getMutex());
        if (obj.empty())
            return;
        obj.pop_back();
    }));

    auto clearGen = just(SimpleAction<std::vector<int>>([](std::vector<int>& obj) {
        std::lock_guard<std::mutex> guard(getMutex());
        obj.clear();
    }));

    auto actionListGen = oneOf<SimpleAction<std::vector<int>>>(pushBackGen, popBackGen, clearGen);

    auto prop = concurrency<std::vector<int>>(Arbi<std::vector<int>>(), actionListGen);
    prop.go();
}

TEST(ConcurrencyTest, WithModel)
{
    struct Model
    {
    };

    auto pushBackGen = Arbi<int>().map<Action<std::vector<int>, Model>>([](int& value) {
        return Action<std::vector<int>, Model>([value](std::vector<int>& obj, Model&) {
            // std::cout << "PushBack(" << value << ")" << std::endl;
            std::lock_guard<std::mutex> guard(getMutex());
            obj.push_back(value);
        });
    });

    auto popBackGen = just(Action<std::vector<int>, Model>([](std::vector<int>& obj, Model&) {
        std::lock_guard<std::mutex> guard(getMutex());
        if (obj.empty())
            return;
        obj.pop_back();
    }));

    auto clearGen = just(Action<std::vector<int>, Model>([](std::vector<int>& obj, Model&) {
        std::lock_guard<std::mutex> guard(getMutex());
        obj.clear();
    }));

    auto actionListGen = oneOf<Action<std::vector<int>, Model>>(pushBackGen, popBackGen, clearGen);

    auto prop = concurrency<std::vector<int>, Model>(
        Arbi<std::vector<int>>(), [](std::vector<int>&) { return Model(); }, actionListGen);
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
                std::cout << "unacquired" << std::endl;
            } catch(std::runtime_error&) {
                std::cout << "failed to unacquire" << std::endl;
            }
        });
    });

    auto actionGen = oneOf<SimpleAction<Bitmap>>(acquireGen/*, unacquireGen*/);
    auto prop = concurrency<Bitmap>(
        just<Bitmap>(Bitmap()), actionGen);
    prop.go();
}
