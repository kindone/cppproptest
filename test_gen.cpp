#include "testing/proptest.hpp"
#include "gmock/fused_src/gtest/gtest.h"
#include "gmock/fused_src/gmock/gmock.h"
#include "gmock/utils/gmock.hpp"
#include "testing/random.hpp"
#include <chrono>
#include <iostream>


int64_t getCurrentTime() {
	auto curTime = std::chrono::system_clock::now();
	auto duration = curTime.time_since_epoch();
	auto millis = std::chrono::duration_cast<std::chrono::millisecond>(duration).count();
	return millis;
}

using namespace PropertyBasedTesting;


class PropTestCase : public ::testing::Test {
};


template<typename T>
struct NumericTest : public testing::Test
{
    using NumericType = T;
};

using NumericTypes = testing::Types<int8_t, int16_t, int32_t, int64_t, uint8_t, uint16_t, uint32_t, uint64_t, float, double>;

TYPED_TEST_CASE(NumericTest, NumericTypes);

TYPED_TEST(NumericTest, NumericTypeGen) {
    int64_t seed = getCurrentTime();
    Random rand(seed);
    Arbitrary<TypeParam> gen;

    for(int i = 0; i < 20; i++) {
        auto val = gen.generate(rand);
        std::cout << val << " " << std::abs(val) << std::endl;
    }
}

TEST(PropTest, GenUTF8String) {
    int64_t seed = getCurrentTime();
    Random rand(seed);
    Arbitrary<UTF8String> gen(5);

    for(int i = 0; i < 20; i++) {
        std::cout << "str: \"" << gen.generate(rand) << "\"" << std::endl;
    }
}

TEST(PropTest, GenLttVectorOfInt) {
    int64_t seed = getCurrentTime();
    Random rand(seed);
    Arbitrary<std::vector<int>> gen(5);

    for(int i = 0; i < 20; i++) {
        auto val = gen.generate(rand);
        std::cout << "vec: ";
        for(size_t j = 0; j < val.size(); j++)
        {
            std::cout << val[j] << ", ";
        }
        std::cout << std::endl;
    }
}

struct GenSmallInt : public Gen<int32_t> {
    GenSmallInt() : step(0ULL) {
    }
    int32_t generate(Random& rand) {
        constexpr size_t num = sizeof(boundaryValues)/sizeof(boundaryValues[0]);
        return boundaryValues[step++ % num];
    }

    size_t step;
    static constexpr int32_t boundaryValues[13] = {INT32_MIN, 0, INT32_MAX, -1, 1, -2, 2, INT32_MIN+1, INT32_MAX-1, INT16_MIN, INT16_MAX, INT16_MIN+1, INT16_MAX-1};
};

struct Animal {
    Animal(int f, std::string n, std::vector<int>& m) : numFeet(f), name(n, allocator()), measures(m, allocator()) {

    }
    int numFeet;
    std::string name;
    std::vector<int> measures;
};

struct Animal2 {
    Animal2(int f, std::string n) : numFeet(f), name(n, allocator()) {

    }
    int numFeet;
    std::string name;
};


struct GenAnimal : public Gen<Animal2> {
    Animal generate1(Random& rand) {
        return construct<Animal, Arbitrary<int>, Arbitrary<std::string>, Arbitrary<std::vector<int> > >(rand);
    }

    Animal2 generate2(Random& rand) {
        return construct<Animal2, Arbitrary<int>, Arbitrary<std::string> >(rand);
    }
};


TEST(PropTest, TestBasic) {
    int64_t seed = getCurrentTime();
    Random rand(seed);
    check(rand, [](int a, int b) -> bool {
        EXPECT_EQ(a+b, b+a);

        std::cout << "a: " << a << ", b: " << b << std::endl;
        return true;
    });

    check(rand, [](int a) -> bool {
        std::cout << "a: " << a << std::endl;
        return true;
    });

    check(rand, [](std::string a, std::string b) -> bool {
        std::string c(allocator()), d(allocator());
        c = a+b;
        d = b+a;
        EXPECT_EQ(c.size(), d.size());
        //EXPECT_EQ(c, d) << "a"; // << "a: " << a << " + b: " << b << ", a+b: " << (a+b) << ", b+a: " << (b+a);
        return true;
    });

    check(rand, [](UTF8String a, UTF8String b) -> bool {
        std::string c(allocator()), d(allocator());
        c = a+b;
        d = b+a;
        PROP_ASSERT(c.size() == d.size(), {});
        //EXPECT_EQ(c, d);// << "a: " << a << " + b: " << b << ", a+b: " << (a+b) << ", b+a: " << (b+a);
        return true;
    });


    check(rand, [](std::vector<int> a) -> bool {
        std::cout << "a: " << a << std::endl;
        return true;
    });

    check<GenSmallInt, GenSmallInt>(rand, [](int a, int b) -> bool {
        std::cout << "a: " << a << ", b: " << b << std::endl;
        return true;
    });

    property([](std::string a, int i, std::string b) -> bool {
        if(i % 2 == 0)
            PROP_DISCARD();
        return true;
    }).check();

    property([](std::string a, int i, std::string b) -> bool {
        if(i % 2 == 0)
            PROP_SUCCESS();
        PROP_DISCARD();
        return true;
    }).check();

    property([](std::vector<int> a) -> bool {
        std::cout << "a: " << a << std::endl;
        return true;
    }).check();


    auto func = [](std::vector<int> a) -> bool {
        return true;
    };

    property(func).check();
    check(rand, func);

    auto prop = property([](std::string a, int i, std::string b) -> bool {
        std::cout << "deferred check!" << std::endl;
        PROP_ASSERT(false, {});
        return true;
    });

    prop.check();
}

bool propertyAsFunc(std::string a, int i, std::vector<int> v) {
    return true;
}

class PropertyAsClass {
public:
    bool operator()(std::string a, int i, std::vector<int> v) {
        return true;
    }

    static bool propertyAsMethod(std::string a, int i, std::vector<int> v) {
        return true;
    }
};


TEST(PropTest, TestBasic2) {
    int64_t seed = getCurrentTime();
    Random rand(seed);

    property(propertyAsFunc).check();
    check(rand, propertyAsFunc);

    PropertyAsClass propertyAsClass;
    property(propertyAsClass).check();
    check(rand, propertyAsClass);

    property(PropertyAsClass::propertyAsMethod).check();
    check(rand, PropertyAsClass::propertyAsMethod);

}

