#include "testing/proptest.hpp"
#include "googletest/googletest/include/gtest/gtest.h"
#include "googletest/googlemock/include/gmock/gmock.h"
#include "testing/Random.hpp"
#include <chrono>
#include <iostream>

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

template <typename T>
T abs(T t) {
    return t;
}

template <>
int8_t abs<int8_t>(int8_t val) {
    return std::abs(val);
}

template <>
int16_t abs<int16_t>(int16_t val) {
    return std::abs(val);
}

template <>
int32_t abs<int32_t>(int32_t val) {
    return std::abs(val);
}

template <>
int64_t abs<int64_t>(int64_t val) {
    return std::abs(val);
}

std::ostream& operator<<(std::ostream& os, const std::vector<int> &input)
{
	os << "[ ";
	for (auto const& i: input) {
		os << i << " ";
	}
	os << "]";
	return os;
}

TEST(PropTest, GenerateBool) {
    int64_t seed = getCurrentTime();
    Random rand(seed);
    Arbitrary<bool> gen;

    for(int i = 0; i < 20; i++) {
        bool val = gen(rand);
        std::cout << val << " " << std::endl;
    }
}


TYPED_TEST(NumericTest, NumericTypeGen) {
    int64_t seed = getCurrentTime();
    Random rand(seed);
    Arbitrary<TypeParam> gen;

    for(int i = 0; i < 20; i++) {
        TypeParam val = gen(rand);
        std::cout << val << " " << abs<TypeParam>(static_cast<TypeParam>(val)) << std::endl;
    }
}

TEST(PropTest, GenUTF8String) {
    int64_t seed = getCurrentTime();
    Random rand(seed);
    Arbitrary<UTF8String> gen(5);

    for(int i = 0; i < 20; i++) {
        std::cout << "str: \"" << static_cast<UTF8String>(gen(rand)) << "\"" << std::endl;
    }
}

TEST(PropTest, GenLttVectorOfInt) {
    int64_t seed = getCurrentTime();
    Random rand(seed);
    Arbitrary<std::vector<int>> gen(5);

    for(int i = 0; i < 20; i++) {
        std::vector<int> val(gen(rand));
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
    Shrinkable<int32_t> operator()(Random& rand) {
        constexpr size_t num = sizeof(boundaryValues)/sizeof(boundaryValues[0]);
        return make_shrinkable<int32_t>(boundaryValues[step++ % num]);
    }

    size_t step;
    static constexpr int32_t boundaryValues[13] = {INT32_MIN, 0, INT32_MAX, -1, 1, -2, 2, INT32_MIN+1, INT32_MAX-1, INT16_MIN, INT16_MAX, INT16_MIN+1, INT16_MAX-1};
};

constexpr int32_t GenSmallInt::boundaryValues[13];



TEST(PropTest, TestCheckBasic) {
    int64_t seed = getCurrentTime();
    Random rand(seed);
    check(rand, [](const int& a, const int& b) -> bool {
        EXPECT_EQ(a+b, b+a);

        std::cout << "a: " << a << ", b: " << b << std::endl;
        return true;
    });

    check(rand, [](int a) -> bool {
        std::cout << "a: " << a << std::endl;
        return true;
    });

    check(rand, [](std::string a, std::string b) -> bool {
        std::string c/*(allocator())*/, d/*(allocator())*/;
        c = a+b;
        d = b+a;
        EXPECT_EQ(c.size(), d.size());
        //EXPECT_EQ(c, d) << "a"; // << "a: " << a << " + b: " << b << ", a+b: " << (a+b) << ", b+a: " << (b+a);
        return true;
    });

    check(rand, [](UTF8String a, UTF8String b) -> bool {
        std::string c/*(allocator())*/, d/*(allocator())*/;
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


TEST(PropTest, TestCheckFail) {
    int64_t seed = getCurrentTime();
    std::cout << "seed: " << seed << std::endl;
    Random rand(seed);
    check(rand, [](int a, int b/*,std::string str, std::vector<int> vec*/) -> bool {
        PROP_ASSERT(-10 < a && a < 100 && -20 < b && b < 200, {});
        return true;
    });
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


TEST(PropTest, TestPropertyFunctionLambdaMethod) {
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


struct Animal {
    Animal(int f, std::string n, std::vector<int>& m) : numFeet(f), name(n/*, allocator()*/), measures(m/*, allocator()*/) {

    }
    int numFeet;
    std::string name;
    std::vector<int> measures;
};

struct Animal2 {
    Animal2(int f, std::string n) : numFeet(f), name(n/*, allocator()*/) {

    }
    int numFeet;
    std::string name;
};

std::ostream& operator<<(std::ostream& os, const Animal &input)
{
	os << "{ ";
    os << "numFeet: " << input.numFeet;
    os << ", name: " << input.name;
    os << ", measures: " << input.measures;
	os << " }";
	return os;
}



TEST(PropTest, TestConstruct) {
    int64_t seed = getCurrentTime();
    Random rand(seed);

    using AnimalGen = Construct<Animal, int, std::string, std::vector<int>&>;
    auto gen = AnimalGen();
    Animal animal = gen(rand);
    std::cout << "Gen animal: " << animal << std::endl;

    check<AnimalGen>(rand, [](Animal animal) -> bool {
        std::cout << "animal: " << animal << std::endl;
        return true;
    });
}

namespace PropertyBasedTesting {

// define arbitrary of Animal using Construct
template<>
class Arbitrary<Animal> : public Construct<Animal, int, std::string, std::vector<int>&> {
};

}

TEST(PropTest, TestCheckArbitraryWithConstruct) {
    int64_t seed = getCurrentTime();
    Random rand(seed);

    check(rand, [](std::vector<Animal> animals) -> bool {
        if(!animals.empty())
            std::cout << "animal: " << animals[0] << std::endl;
        return true;
    });
}

TEST(PropTest, TestFilter) {
    int64_t seed = getCurrentTime();
    Random rand(seed);

    Filter<Arbitrary<Animal>> filteredGen([](Animal& a) -> bool {
        return a.numFeet >= 0 && a.numFeet < 100 && a.name.size() == 3 && a.measures.size() < 10;
    });

    std::cout << "filtered animal: " << filteredGen(rand) << std::endl;
}


TEST(PropTest, TestFilter22) {
    int64_t seed = getCurrentTime();
    Random rand(seed);

    Arbitrary<int> gen;
    auto evenGen = filter<int>(gen, [](const int& value) {
        return value % 2 == 0;
    }); 

    for(int i = 0; i < 10; i++) {
        std::cout << "even: " << evenGen(rand) << std::endl;
    }
}

class Complicated {
public:
    int value;
    Complicated(int a) : value(a){
    }
 
    Complicated(const Complicated&) = delete;
    Complicated(Complicated&&) = default;
private:
    Complicated() {
    }
};


TEST(PropTest, TestShrinkable) {
    auto vec = make_shrinkable<std::vector<int>>(std::vector<int>());
    auto complicated = make_shrinkable<Complicated>(5);

    auto shrink = []() {
        return make_shrinkable<Complicated>(5);
    };

}



