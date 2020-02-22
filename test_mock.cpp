#include "testing/proptest.hpp"
#include "gmock/fused_src/gtest/gtest.h"
#include "gmock/fused_src/gmock/gmock.h"
#include "gmock/utils/gmock.hpp"

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
