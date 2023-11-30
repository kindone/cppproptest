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

using namespace proptest;

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

namespace proptest {

DEFINE_ARBITRARY(shared_ptr<NiceMock<MockCat>>, []() {
    auto vecGen = Arbitrary<vector<int>>();
    auto mockGen = vecGen.map<shared_ptr<NiceMock<MockCat>>>([](vector<int>& vec) {
        auto cat = util::make_shared<NiceMock<MockCat>>();
        auto index = util::make_shared<int>(0);
        auto vecPtr = util::make_shared<vector<int>>(vec);
        ON_CALL(*cat, meow(_)).WillByDefault(Return(0));
        ON_CALL(*cat, meow(0)).WillByDefault(Invoke([vecPtr,index](int) {
            int& i = *index;
            if(i >= static_cast<int>(vecPtr->size()))
                return 0;
            i ++;
            return (*vecPtr)[i];
        }));
        return cat;
    });
    return mockGen;
});

namespace util {

template <typename T>
struct Functor {
    Functor(shared_ptr<vector<T>> _vecPtr) :  vecPtr(_vecPtr), i(0) {}

    T next() {
        if(i >= vecPtr->size())
            return vecPtr->last();
        return (*vecPtr)[i++];
    }

    shared_ptr<vector<T>> vecPtr;
    int i;
};

}

DEFINE_ARBITRARY(shared_ptr<MockCat>, []() {
    auto vecGen = Arbitrary<vector<int>>();
    vecGen.setMinSize(1);
    auto mockGen = vecGen.map<shared_ptr<MockCat>>([](vector<int>& vec) {
        auto cat = util::make_shared<MockCat>();
        auto index = util::make_shared<int>(0);
        auto vecPtr = util::make_shared<vector<int>>(vec);
        ON_CALL(*cat, meow(_)).WillByDefault(Return(0));
        ON_CALL(*cat, meow(0)).WillByDefault(Invoke([vecPtr,index](int) {
            int& i = *index;
            if(i >= static_cast<int>(vecPtr->size()))
                return 0;
            i ++;
            return (*vecPtr)[i];
        }));
        return cat;
    });
    return mockGen;
});

}

TEST(PropTest, MockOnCall2)
{
    int64_t seed = getCurrentTime();
    Random rand(seed);
    auto mockGen = Arbitrary<shared_ptr<MockCat>>();
    auto mockptr = mockGen(rand).getRef();
    for(int i = 0; i < 20; i++)
        cout << "mock.meow(0): " << mockptr->meow(0) << endl;
}


TEST(PropTest, NiceMockOnCall)
{
    int64_t seed = getCurrentTime();
    Random rand(seed);
    auto mockGen = Arbitrary<shared_ptr<NiceMock<MockCat>>>();
    auto mockptr = mockGen(rand).getRef();
    for(int i = 0; i < 20; i++)
        cout << "mock.meow(0): " << mockptr->meow(0) << endl;
}
