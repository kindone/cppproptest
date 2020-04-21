#include "testbase.hpp"

using namespace PropertyBasedTesting;

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

TYPED_TEST(NumericTest, GenNumericType) {
    int64_t seed = getCurrentTime();
    Random rand(seed);
    Arbitrary<TypeParam> gen;

    for(int i = 0; i < 20; i++) {
        TypeParam val = gen(rand).get();
        std::cout << val << " " << abs<TypeParam>(static_cast<TypeParam>(val)) << std::endl;
    }
}

TYPED_TEST(IntegralTest, GenInRange) {
    int64_t seed = getCurrentTime();
    Random rand(seed);
    bool isSigned = std::numeric_limits<TypeParam>::min() < 0;

    auto gen0 = isSigned ? fromTo<TypeParam>(-10, 10) : fromTo<TypeParam>(0, 20);
    TypeParam min = gen0(rand).get();
    TypeParam max = min + rand.getRandomInt32(0,10);
    auto gen = fromTo<TypeParam>(min, max);
    std::cout << "min: " << min << ", max: " << max << std::endl;

    for(int i = 0; i < 20; i++) {
        TypeParam val = gen(rand).getRef();
        std::cout << val << " " << abs<TypeParam>(static_cast<TypeParam>(val)) << std::endl;
    }
}


TEST(PropTest, GenerateBool) {
    int64_t seed = getCurrentTime();
    Random rand(seed);
    Arbitrary<bool> gen;

    for(int i = 0; i < 20; i++) {
        bool val = gen(rand).get();
        std::cout << val << " " << std::endl;
    }
}


TEST(PropTest, GenUTF8String) {
    int64_t seed = getCurrentTime();
    Random rand(seed);
    Arbitrary<UTF8String> gen(5);

    for(int i = 0; i < 20; i++) {
        std::cout << "str: \"" << static_cast<UTF8String>(gen(rand).getRef()) << "\"" << std::endl;
    }
}

TEST(PropTest, GenSharedPtr) {
    int64_t seed = getCurrentTime();
    Random rand(seed);
    Arbitrary<std::shared_ptr<int>> gen;
    for(int i = 0; i < 20; i++) {
        std::cout << "int: " << *gen(rand).getRef() << std::endl;
    }
}
