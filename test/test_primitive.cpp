#include "testbase.hpp"

using namespace PropertyBasedTesting;

template <typename T>
T abs(T t)
{
    return t;
}

template <>
int8_t abs<int8_t>(int8_t val)
{
    return std::abs(val);
}

template <>
int16_t abs<int16_t>(int16_t val)
{
    return std::abs(val);
}

template <>
int32_t abs<int32_t>(int32_t val)
{
    return std::abs(val);
}

template <>
int64_t abs<int64_t>(int64_t val)
{
    return std::abs(val);
}

TYPED_TEST(NumericTest, GenNumericType)
{
    int64_t seed = getCurrentTime();
    Random rand(seed);
    Arbitrary<TypeParam> gen;

    for (int i = 0; i < 20; i++) {
        TypeParam val = gen(rand).get();
        std::cout << val << " " << abs<TypeParam>(static_cast<TypeParam>(val)) << std::endl;
    }
}

TYPED_TEST(IntegralTest, GenInRange)
{
    int64_t seed = getCurrentTime();
    Random rand(seed);
    bool isSigned = std::numeric_limits<TypeParam>::min() < 0;

    auto gen0 = isSigned ? fromTo<TypeParam>(-10, 10) : fromTo<TypeParam>(0, 20);
    TypeParam min = gen0(rand).get();
    TypeParam max = min + rand.getRandomInt32(0, 10);
    auto gen = fromTo<TypeParam>(min, max);
    std::cout << "min: " << min << ", max: " << max << std::endl;

    for (int i = 0; i < 20; i++) {
        TypeParam val = gen(rand).getRef();
        std::cout << val << " " << abs<TypeParam>(static_cast<TypeParam>(val)) << std::endl;
    }
}

TEST(PropTest, GenerateBool)
{
    int64_t seed = getCurrentTime();
    Random rand(seed);
    Arbitrary<bool> gen;

    for (int i = 0; i < 20; i++) {
        bool val = gen(rand).get();
        std::cout << val << " " << std::endl;
    }
}

TEST(PropTest, GenString)
{
    int64_t seed = getCurrentTime();
    Random rand(seed);
    auto alphabets = inRange<char>('A', 'z');
    Arbitrary<std::string> gen(alphabets);
    gen.setSize(5);

    for (int i = 0; i < 20; i++) {
        std::cout << "str: \"" << static_cast<std::string>(gen(rand).getRef()) << "\"" << std::endl;
    }
}

TEST(PropTest, GenUTF8String)
{
    int64_t seed = getCurrentTime();
    Random rand(seed);
    Arbitrary<UTF8String> gen;
    gen.setSize(5);

    for (int i = 0; i < 20; i++) {
        std::cout << "str: " << static_cast<UTF8String>(gen(rand).getRef()) << std::endl;
    }
}

TEST(PropTest, GenUTF8String2)
{
    int64_t seed = getCurrentTime();
    Random rand(seed);
    Arbitrary<UTF8String> gen;
    gen.setMaxSize(8);

    // for(int i = 0; i < 20; i++) {
    //     std::vector<int> val(gen(rand).get());
    //     std::cout << "vec: ";
    //     for(size_t j = 0; j < val.size(); j++)
    //     {
    //         std::cout << val[j] << ", ";
    //     }
    //     std::cout << std::endl;
    // }
    for (int i = 0; i < 3; i++)
        exhaustive(gen(rand), 0);
}

TEST(PropTest, GenSharedPtr)
{
    int64_t seed = getCurrentTime();
    Random rand(seed);
    Arbitrary<std::shared_ptr<int>> gen;
    for (int i = 0; i < 20; i++) {
        std::cout << "int: " << *gen(rand).getRef() << std::endl;
    }
}

TEST(PropTest, GenSet)
{
    int64_t seed = getCurrentTime();
    Random rand(seed);
    Arbitrary<std::set<int>> gen;
    gen.setMaxSize(8);
    auto shr = gen(rand);
    exhaustive(shr, 0);
    // for (int i = 0; i < 20; i++) {
    //     std::cout << "int: " << *gen(rand).getRef() << std::endl;
    // }
}

TEST(PropTest, GenMap)
{
    int64_t seed = getCurrentTime();
    Random rand(seed);
    Arbitrary<std::map<int, int>> gen;
    gen.setMaxSize(8);
    auto shr = gen(rand);
    exhaustive(shr, 0);
    // for (int i = 0; i < 20; i++) {
    //     std::cout << "int: " << *gen(rand).getRef() << std::endl;
    // }
}

TEST(PropTest, GenMap2)
{
    int64_t seed = getCurrentTime();
    Random rand(seed);
    Arbitrary<std::map<int, int>> gen;
    gen.setMaxSize(8);
    gen.setElemGen(inRange<int>(0, 100));
    gen.setKeyGen(Arbitrary<int>());
    auto shr = gen(rand);
    exhaustive(shr, 0);
    // for (int i = 0; i < 20; i++) {
    //     std::cout << "int: " << *gen(rand).getRef() << std::endl;
    // }
}

TEST(PropTest, GenList)
{
    int64_t seed = getCurrentTime();
    Random rand(seed);
    Arbitrary<std::list<int>> gen(inRange<int>(0, 100));
    gen.setMaxSize(8);
    auto shr = gen(rand);
    exhaustive(shr, 0);
    // for (int i = 0; i < 20; i++) {
    //     std::cout << "int: " << *gen(rand).getRef() << std::endl;
    // }
}
