#include "testing/proptest.hpp"
#include "googletest/googletest/include/gtest/gtest.h"
#include "googletest/googlemock/include/gmock/gmock.h"

class MockTestCase : public ::testing::Test {
};


struct Cat
{
    Cat() {
    }
    virtual ~Cat() {
    }

    virtual int meow(int loudness) {
        return loudness * 2;
    }
};

struct MockCat : public Cat
{
    MockCat() {
    }
    ~MockCat() {
    }

    MOCK_METHOD1(meow, int(int));
};

using namespace PropertyBasedTesting;

using ::testing::AssertionResult;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::_;

TEST(PropTest, MockTest) {
    MockCat* cat = new MockCat();
    // general
    EXPECT_CALL(*cat, meow(_))
        .WillOnce(Return(0));

    // specific
    EXPECT_CALL(*cat, meow(5))
        .WillOnce(Return(5));

    EXPECT_EQ(cat->meow(5), 5);
    EXPECT_EQ(cat->meow(4), 0);

    delete cat;
}
