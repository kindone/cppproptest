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
    gen.setSize(1);

    PropertyContext context;

    for (int i = 0; i < 10000; i++) {
        UTF8String str = static_cast<UTF8String>(gen(rand).getRef());
        uint8_t c0 = str[0];
        if (str.size() == 1) {
            if (c0 <= 0x7f)
                context.tag(__FILE__, __LINE__, "code", "1 byte U+0000..U+007F");
            else
                context.tag(__FILE__, __LINE__, "code", "1 byte error");
        } else if (str.size() == 2) {
            uint8_t c1 = str[1];
            if (0xc2 <= c0 && c0 <= 0xdf && 0x80 <= c1 && c1 <= 0xbf)
                context.tag(__FILE__, __LINE__, "code", "2 bytes U+0080..U+07FF");
            else
                context.tag(__FILE__, __LINE__, "code", "2 byte error");
        } else if (str.size() == 3) {
            uint8_t c1 = str[1];
            uint8_t c2 = str[2];
            if (0xe0 == c0 && 0xa0 <= c1 && c1 <= 0xbf && 0x80 <= c2 && c2 <= 0xbf)
                context.tag(__FILE__, __LINE__, "code", "3 bytes U+0800..U+0FFF");
            else if (0xe1 <= c0 && c0 <= 0xec && 0x80 <= c1 && c1 <= 0xbf && 0x80 <= c2 && c2 <= 0xbf)
                context.tag(__FILE__, __LINE__, "code", "3 bytes U+1000..U+CFFF");
            else if (0xed == c0 && 0x80 <= c1 && c1 <= 0x9f && 0x80 <= c2 && c2 <= 0xbf)
                context.tag(__FILE__, __LINE__, "code", "3 bytes U+D000..U+D7FF");
            else if (0xee <= c0 && c0 <= 0xef && 0x80 <= c1 && c1 <= 0xbf && 0x80 <= c2 && c2 <= 0xbf)
                context.tag(__FILE__, __LINE__, "code", "3 bytes U+E000..U+FFFF");
            else
                context.tag(__FILE__, __LINE__, "code", "3 bytes error");
        } else if (str.size() == 4) {
            uint8_t c1 = str[1];
            uint8_t c2 = str[2];
            uint8_t c3 = str[2];
            if (0xf0 == c0 && 0x90 <= c1 && c1 <= 0xbf && 0x80 <= c2 && c2 <= 0xbf && 0x80 <= c3 && c3 <= 0xbf)
                context.tag(__FILE__, __LINE__, "code", "4 bytes U+10000..U+3FFFF");
            else if (0xf1 <= c0 && c0 <= 0xf3 && 0x80 <= c1 && c1 <= 0xbf && 0x80 <= c2 && c2 <= 0xbf && 0x80 <= c3 &&
                     c3 <= 0xbf)
                context.tag(__FILE__, __LINE__, "code", "4 bytes U+40000..U+FFFFF");
            else if (0xf4 == c0 && 0x80 <= c1 && c1 <= 0x8f && 0x80 <= c2 && c2 <= 0xbf && 0x80 <= c3 && c3 <= 0xbf)
                context.tag(__FILE__, __LINE__, "code", "4 bytes U+100000..U+10FFFF");
            else
                context.tag(__FILE__, __LINE__, "code", "4 bytes error");
        } else
            context.tag(__FILE__, __LINE__, "code", "error");
        // std::cout << "str: " << static_cast<CESU8String>(gen(rand).getRef()) << std::endl;
    }
    context.printSummary();
}

TEST(PropTest, GenUTF8String2)
{
    int64_t seed = getCurrentTime();
    Random rand(seed);
    Arbitrary<UTF8String> gen;
    gen.setMaxSize(8);
    for (int i = 0; i < 3; i++)
        exhaustive(gen(rand), 0);
}

TEST(PropTest, GenCESU8String)
{
    int64_t seed = getCurrentTime();
    Random rand(seed);
    Arbitrary<CESU8String> gen;
    gen.setSize(1);

    PropertyContext context;

    for (int i = 0; i < 10000; i++) {
        CESU8String str = static_cast<CESU8String>(gen(rand).getRef());
        uint8_t c0 = str[0];
        if (str.size() == 1) {
            if (c0 <= 0x7f)
                context.tag(__FILE__, __LINE__, "code", "1 byte U+0000..U+007F");
            else
                context.tag(__FILE__, __LINE__, "code", "1 byte error");
        } else if (str.size() == 2) {
            uint8_t c1 = str[1];
            if (0xc2 <= c0 && c0 <= 0xdf && 0x80 <= c1 && c1 <= 0xbf)
                context.tag(__FILE__, __LINE__, "code", "2 bytes U+0080..U+07FF");
            else
                context.tag(__FILE__, __LINE__, "code", "2 byte error");
        } else if (str.size() == 3) {
            uint8_t c1 = str[1];
            uint8_t c2 = str[2];
            if (0xe0 == c0 && 0xa0 <= c1 && c1 <= 0xbf && 0x80 <= c2 && c2 <= 0xbf)
                context.tag(__FILE__, __LINE__, "code", "3 bytes U+0800..U+0FFF");
            else if (0xe1 <= c0 && c0 <= 0xec && 0x80 <= c1 && c1 <= 0xbf && 0x80 <= c2 && c2 <= 0xbf)
                context.tag(__FILE__, __LINE__, "code", "3 bytes U+1000..U+CFFF");
            else if (0xed == c0 && 0x80 <= c1 && c1 <= 0x9f && 0x80 <= c2 && c2 <= 0xbf)
                context.tag(__FILE__, __LINE__, "code", "3 bytes U+D000..U+D7FF");
            else if (0xee <= c0 && c0 <= 0xef && 0x80 <= c1 && c1 <= 0xbf && 0x80 <= c2 && c2 <= 0xbf)
                context.tag(__FILE__, __LINE__, "code", "3 bytes U+E000..U+FFFF");
            else
                context.tag(__FILE__, __LINE__, "code", "3 bytes error");
        } else if (str.size() == 6) {
            uint8_t c1 = str[1];
            uint8_t c2 = str[2];
            uint8_t c3 = str[3];
            uint8_t c4 = str[4];
            uint8_t c5 = str[5];
            if (0xed == c0 && 0xa0 <= c1 && c1 <= 0xaf && 0x80 <= c2 && c2 <= 0xbf && 0xed == c3 && 0xb0 <= c4 &&
                c4 <= 0xbf && 0x80 <= c5 && c5 <= 0xbf)
                context.tag(__FILE__, __LINE__, "code", "6 bytes U+10000..U+10FFFF");
            else
                context.tag(__FILE__, __LINE__, "code", "6 bytes error");
        } else
            context.tag(__FILE__, __LINE__, "code", "error");
        // std::cout << "str: " << static_cast<CESU8String>(gen(rand).getRef()) << std::endl;
    }
    context.printSummary();
}

TEST(PropTest, GenCESU8String2)
{
    int64_t seed = getCurrentTime();
    Random rand(seed);
    Arbitrary<CESU8String> gen;
    gen.setMaxSize(8);
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
