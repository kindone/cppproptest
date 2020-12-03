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

template <>
struct Arbi<std::shared_ptr<NiceMock<MockCat>>> : ArbiBase<std::shared_ptr<NiceMock<MockCat>>> {
    Shrinkable<std::shared_ptr<NiceMock<MockCat>>> operator()(Random& rand) {
        auto vecGen = Arbitrary<std::vector<int>>();
        auto mockGen = vecGen.map<std::shared_ptr<NiceMock<MockCat>>>([](std::vector<int>& vec) {
            auto cat = std::make_shared<NiceMock<MockCat>>();
            auto index = std::make_shared<int>(0);
            auto vecPtr = std::make_shared<std::vector<int>>(vec);
            ON_CALL(*cat, meow(_)).WillByDefault(Return(0));
            ON_CALL(*cat, meow(0)).WillByDefault(Invoke([vecPtr,index](int) {
                int& i = *index;
                if(i >= vecPtr->size())
                    return 0;
                i ++;
                return (*vecPtr)[i];
            }));
            return cat;
        });
        return mockGen(rand);
    }
};

namespace util {

template <typename T>
struct Functor {
    Functor(std::shared_ptr<std::vector<T>> _vecPtr) :  vecPtr(_vecPtr), i(0) {}

    T next() {
        if(i >= vecPtr->size())
            return vecPtr->last();
        return (*vecPtr)[i++];
    }

    std::shared_ptr<std::vector<T>> vecPtr;
    int i;
};

}


template <>
struct Arbi<std::shared_ptr<MockCat>> : ArbiBase<std::shared_ptr<MockCat>> {
    Shrinkable<std::shared_ptr<MockCat>> operator()(Random& rand) {
        auto vecGen = Arbitrary<std::vector<int>>();
        vecGen.setMinSize(1);
        auto mockGen = vecGen.map<std::shared_ptr<MockCat>>([](std::vector<int>& vec) {
            auto cat = std::make_shared<MockCat>();
            auto index = std::make_shared<int>(0);
            auto vecPtr = std::make_shared<std::vector<int>>(vec);
            ON_CALL(*cat, meow(_)).WillByDefault(Return(0));
            ON_CALL(*cat, meow(0)).WillByDefault(Invoke([vecPtr,index](int) {
                int& i = *index;
                if(i >= vecPtr->size())
                    return 0;
                i ++;
                return (*vecPtr)[i];
            }));
            return cat;
        });
        return mockGen(rand);
    }
};

}

TEST(PropTest, MockOnCall2)
{
    int64_t seed = getCurrentTime();
    Random rand(seed);
    auto mockGen = Arbitrary<std::shared_ptr<MockCat>>();
    auto mockptr = mockGen(rand).getRef();
    for(int i = 0; i < 20; i++)
        std::cout << "mock.meow(0): " << mockptr->meow(0) << std::endl;
}


TEST(PropTest, NiceMockOnCall)
{
    int64_t seed = getCurrentTime();
    Random rand(seed);
    auto mockGen = Arbitrary<std::shared_ptr<NiceMock<MockCat>>>();
    auto mockptr = mockGen(rand).getRef();
    for(int i = 0; i < 20; i++)
        std::cout << "mock.meow(0): " << mockptr->meow(0) << std::endl;
}
