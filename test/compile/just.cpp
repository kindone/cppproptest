#include "proptest.hpp"
#include "googletest/googletest/include/gtest/gtest.h"
#include "googletest/googlemock/include/gmock/gmock.h"
#include "Random.hpp"

using namespace proptest;

struct NonCopyable
{
    NonCopyable(int a) : a(a) { }
    NonCopyable(const NonCopyable&) = delete;
    NonCopyable& operator=(const NonCopyable&) = delete;
    NonCopyable(NonCopyable&&) = delete;
    ~NonCopyable() {
        cout << "~NonCopyable" << endl;
    }
    int a;
};

TEST(Compile, just)
{
    Random rand(3);
    auto gen1 = just(1);
    EXPECT_EQ(gen1(rand).get(), 1);

    auto gen2 = just<NonCopyable>(util::make_shared<NonCopyable>(2));
    EXPECT_EQ(gen2(rand).getRef().a, 2);
}
