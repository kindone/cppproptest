#include "proptest.hpp"
#include "googletest/googletest/include/gtest/gtest.h"
#include "googletest/googlemock/include/gmock/gmock.h"

class MockTestCase : public ::testing::Test {
};

struct Cat
{
    Cat() {}
    virtual ~Cat() {}

    virtual int meow(int loudness) { return loudness * 2; }
};

struct MockCat : public Cat
{
    MockCat() {}
    ~MockCat() {}

    MOCK_METHOD1(meow, int(int));
};

using namespace PropertyBasedTesting;

using ::testing::_;
using ::testing::AssertionResult;
using ::testing::Invoke;
using ::testing::NiceMock;
using ::testing::Return;

TEST(PropTest, MockTest)
{
    MockCat* cat = new MockCat();
    // general
    EXPECT_CALL(*cat, meow(_)).WillRepeatedly(Return(0));

    // specific
    EXPECT_CALL(*cat, meow(5)).WillOnce(Return(5));

    EXPECT_EQ(cat->meow(5), 5);
    EXPECT_EQ(cat->meow(4), 0);
    EXPECT_EQ(cat->meow(3), 0);

    delete cat;
}

TEST(PropTest, MockOnCall)
{
    NiceMock<MockCat> cat;
    // general
    ON_CALL(cat, meow(_)).WillByDefault(Return(0));

    // specific
    ON_CALL(cat, meow(5)).WillByDefault(Return(5));

    int count = 0;
    ON_CALL(cat, meow(0)).WillByDefault(Invoke([&count](int) { return count++; }));

    EXPECT_EQ(cat.meow(5), 5);
    EXPECT_EQ(cat.meow(4), 0);
    EXPECT_EQ(cat.meow(3), 0);

    EXPECT_EQ(cat.meow(0), 0);
    EXPECT_EQ(cat.meow(0), 1);
}
