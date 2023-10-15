#include "proptest.hpp"
#include "googletest/googletest/include/gtest/gtest.h"
#include "googletest/googlemock/include/gmock/gmock.h"
#include "Random.hpp"

using namespace proptest;

TEST(Compile, aggregate1)
{
    auto gen1 = interval<int>(0, 1000);

    [[maybe_unused]] auto gen = aggregate(
        gen1,
        [](int& num) {
            return interval(num/2, num*2);
        },
        2, 10);
}

TEST(Compile, aggregate2)
{
    auto gen1 = interval<int>(0, 1).map([](int& num) {
        list<int> l{num};
        return l;
    });

    [[maybe_unused]] auto gen = aggregate(
        gen1,
        [](list<int>& nums) {
            auto last = nums.back();
            return interval(last, last + 1).map([nums](int& num) {
                auto newList = list<int>(nums);
                newList.push_back(num);
                return newList;
            });
        },
        2, 4);
}
