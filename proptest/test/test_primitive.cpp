#include "testbase.hpp"

using namespace proptest;

template <typename T>
T abs(T t)
{
    return t;
}

template <>
int8_t abs<int8_t>(int8_t val)
{
    return static_cast<int8_t>(std::abs(val));
}

template <>
int16_t abs<int16_t>(int16_t val)
{
    return static_cast<int16_t>(std::abs(val));
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
    Arbi<TypeParam> gen;

    for (int i = 0; i < 20; i++) {
        TypeParam val = gen(rand).get();
        cout << val << " " << abs<TypeParam>(static_cast<TypeParam>(val)) << endl;
    }
}

TYPED_TEST(IntegralTest, GenInRange)
{
    int64_t seed = getCurrentTime();
    Random rand(seed);
    bool isSigned = numeric_limits<TypeParam>::min() < 0;

    auto gen0 = isSigned ? interval<TypeParam>(-10, 10) : interval<TypeParam>(0, 20);
    TypeParam min = gen0(rand).get();
    TypeParam max = static_cast<TypeParam>(min + rand.getRandomInt32(0, 10));
    auto gen = interval<TypeParam>(min, max);
    cout << "min: " << min << ", max: " << max << endl;

    for (int i = 0; i < 20; i++) {
        TypeParam val = gen(rand).getRef();
        cout << val << " " << abs<TypeParam>(static_cast<TypeParam>(val)) << endl;
    }
}

TEST(PropTest, GenerateBool)
{
    int64_t seed = getCurrentTime();
    Random rand(seed);
    Arbi<bool> gen;

    for (int i = 0; i < 20; i++) {
        bool val = gen(rand).get();
        cout << val << " " << endl;
    }
}

TEST(PropTest, GenString)
{
    int64_t seed = getCurrentTime();
    Random rand(seed);
    auto alphabets = interval<char>('A', 'z');
    Arbi<string> gen(alphabets);
    gen.setSize(5);

    for (int i = 0; i < 20; i++) {
        cout << "str: \"" << static_cast<string>(gen(rand).getRef()) << "\"" << endl;
    }
}

TEST(PropTest, GenString2)
{
    int64_t seed = getCurrentTime();
    Random rand(seed);
    auto alphabets = interval<char>('A', 'z');
    Arbi<string> gen(alphabets);
    gen.setSize(1, 4);

    for (int i = 0; i < 3; i++) {
        auto shr = gen(rand);
        cout << "str: \"" << static_cast<string>(shr.getRef()) << "\"" << endl;
        exhaustive(shr, 0);
    }
}

void testUTF8(PropertyContext& context, Random rand)
{
    Arbi<UTF8String> gen;
    gen.setSize(1);

    for (int i = 0; i < 100000; i++) {
        UTF8String str = static_cast<UTF8String>(gen(rand).getRef());
        uint8_t c0 = str[0];
        context.tag(__FILE__, __LINE__, "charsize", to_string(str.charsize()));
        if (str.size() == 1) {
            if (c0 <= 0x7f)
                context.tag(__FILE__, __LINE__, "UTF-8 code", "1 byte U+0001..U+007F");
            else
                context.tag(__FILE__, __LINE__, "UTF-8 code", "1 byte error");
            context.tag(__FILE__, __LINE__, "UTF-8 code coarse", "U+0001..U+D7FF");
        } else if (str.size() == 2) {
            uint8_t c1 = str[1];
            if (0xc2 <= c0 && c0 <= 0xdf && 0x80 <= c1 && c1 <= 0xbf)
                context.tag(__FILE__, __LINE__, "UTF-8 code", "2 bytes U+0080..U+07FF");
            else
                context.tag(__FILE__, __LINE__, "UTF-8 code", "2 byte error");
            context.tag(__FILE__, __LINE__, "UTF-8 code coarse", "U+0001..U+D7FF");
        } else if (str.size() == 3) {
            uint8_t c1 = str[1];
            uint8_t c2 = str[2];
            if (0xe0 == c0 && 0xa0 <= c1 && c1 <= 0xbf && 0x80 <= c2 && c2 <= 0xbf) {
                context.tag(__FILE__, __LINE__, "UTF-8 code", "3 bytes U+0800..U+0FFF");
                context.tag(__FILE__, __LINE__, "UTF-8 code coarse", "U+0001..U+D7FF");
            } else if (0xe1 <= c0 && c0 <= 0xec && 0x80 <= c1 && c1 <= 0xbf && 0x80 <= c2 && c2 <= 0xbf) {
                context.tag(__FILE__, __LINE__, "UTF-8 code", "3 bytes U+1000..U+CFFF");
                context.tag(__FILE__, __LINE__, "UTF-8 code coarse", "U+0001..U+D7FF");
            } else if (0xed == c0 && 0x80 <= c1 && c1 <= 0x9f && 0x80 <= c2 && c2 <= 0xbf) {
                context.tag(__FILE__, __LINE__, "UTF-8 code", "3 bytes U+D000..U+D7FF");
                context.tag(__FILE__, __LINE__, "UTF-8 code coarse", "U+0001..U+D7FF");
            } else if (0xee <= c0 && c0 <= 0xef && 0x80 <= c1 && c1 <= 0xbf && 0x80 <= c2 && c2 <= 0xbf) {
                context.tag(__FILE__, __LINE__, "UTF-8 code", "3 bytes U+E000..U+FFFF");
                context.tag(__FILE__, __LINE__, "UTF-8 code coarse", "U+E000..U+FFFF");
            } else
                context.tag(__FILE__, __LINE__, "UTF-8 code", "3 bytes error");
        } else if (str.size() == 4) {
            uint8_t c1 = str[1];
            uint8_t c2 = str[2];
            uint8_t c3 = str[3];
            if (0xf0 == c0 && 0x90 <= c1 && c1 <= 0xbf && 0x80 <= c2 && c2 <= 0xbf && 0x80 <= c3 && c3 <= 0xbf) {
                context.tag(__FILE__, __LINE__, "UTF-8 code", "4 bytes U+10000..U+3FFFF");
            } else if (0xf1 <= c0 && c0 <= 0xf3 && 0x80 <= c1 && c1 <= 0xbf && 0x80 <= c2 && c2 <= 0xbf && 0x80 <= c3 &&
                       c3 <= 0xbf) {
                context.tag(__FILE__, __LINE__, "UTF-8 code", "4 bytes U+40000..U+FFFFF");
            } else if (0xf4 == c0 && 0x80 <= c1 && c1 <= 0x8f && 0x80 <= c2 && c2 <= 0xbf && 0x80 <= c3 && c3 <= 0xbf) {
                context.tag(__FILE__, __LINE__, "UTF-8 code", "4 bytes U+100000..U+10FFFF");
            } else
                context.tag(__FILE__, __LINE__, "UTF-8 code", "4 bytes error");
            context.tag(__FILE__, __LINE__, "UTF-8 code coarse", "U+10000..U+10FFFF");
        } else
            context.tag(__FILE__, __LINE__, "UTF-8 code", "error");
        // cout << "str: " << static_cast<CESU8String>(gen(rand).getRef()) << endl;
    }
}

void testUTF32(PropertyContext& context, Random rand)
{
    uint32_t rangeSize = 0x10FFFF - 0x0001 + (0xDFFF - 0xD800 + 1) + 1;
    auto unicodeGen = UnicodeGen();
    for (int i = 0; i < 100000; i++) {
        rand.getRandomSize(0, 1);  // dummy call to align with string generators
        uint32_t code = unicodeGen(rand).get();

        if (code <= 0xD7FF)
            context.tag(
                __FILE__, __LINE__, "code",
                "U+0001..U+D7FF (" + to_string(static_cast<double>(0xD7FF - 0x0001 + 1) / rangeSize * 100) + "%)");
        else if (0xE000 <= code && code <= 0xFFFF)
            context.tag(
                __FILE__, __LINE__, "code",
                "U+E000..U+FFFF (" + to_string(static_cast<double>(0xFFFF - 0xE000 + 1) / rangeSize * 100) + "%)");
        else
            context.tag(__FILE__, __LINE__, "code",
                        "U+10000..U+10FFFF(" +
                            to_string(static_cast<double>(0x10FFFF - 0x10000 + 1) / rangeSize * 100) + "%)");
    }
}

void testUTF16BE(PropertyContext& context, Random rand)
{
    Arbi<UTF16BEString> gen;
    gen.setSize(1);

    uint32_t rangeSize = 0x10FFFF - 0x0001 + (0xDFFF - 0xD800 + 1) + 1;

    for (int i = 0; i < 100000; i++) {
        UTF16BEString str = static_cast<UTF16BEString>(gen(rand).getRef());
        str.charsize();
        uint8_t c0 = str[0];
        context.tag(__FILE__, __LINE__, "charsize", to_string(str.charsize()));
        if (str.size() == 1) {
            context.tag(__FILE__, __LINE__, "UTF-16 BE code", "1 byte error");
        } else if (str.size() == 2) {
            if (c0 <= 0xD7)
                context.tag(__FILE__, __LINE__, "UTF-16 BE code",
                            "2 bytes U+0001..U+D7FF (" +
                                to_string(static_cast<double>(0xD7FF - 0x0001 + 1) / rangeSize * 100) + "%)");
            else if (0xE0 <= c0)
                context.tag(__FILE__, __LINE__, "UTF-16 BE code",
                            "2 bytes U+E000..U+FFFF (" +
                                to_string(static_cast<double>(0xFFFF - 0xE000 + 1) / rangeSize * 100) + "%)");
            else
                context.tag(__FILE__, __LINE__, "UTF-16 BE code", "2 byte error");
        } else if (str.size() == 3) {
            context.tag(__FILE__, __LINE__, "UTF-16 BE code", "3 bytes error");
        } else if (str.size() == 4) {
            uint8_t c2 = str[2];
            if (0xD8 <= c0 && c0 <= 0xDB && 0xDC <= c2 && c2 <= 0xDF)
                context.tag(__FILE__, __LINE__, "UTF-16 BE code",
                            "4 bytes U+10000..U+10FFFF(" +
                                to_string(static_cast<double>(0x10FFFF - 0x10000 + 1) / rangeSize * 100) + "%)");
            else
                context.tag(__FILE__, __LINE__, "UTF-16 BE code", "4 bytes error");
        } else
            context.tag(__FILE__, __LINE__, "UTF-16 BE code", "error");
    }
}

void testUTF16LE(PropertyContext& context, Random rand)
{
    Arbi<UTF16LEString> gen;
    gen.setSize(1);

    uint32_t rangeSize = 0x10FFFF - 0x0001 + (0xDFFF - 0xD800 + 1) + 1;

    for (int i = 0; i < 100000; i++) {
        UTF16LEString str = static_cast<UTF16LEString>(gen(rand).getRef());
        str.charsize();
        uint8_t c0 = str[1];
        context.tag(__FILE__, __LINE__, "charsize", to_string(str.charsize()));
        if (str.size() == 1) {
            context.tag(__FILE__, __LINE__, "UTF-16 LE code", "1 byte error");
        } else if (str.size() == 2) {
            if (c0 <= 0xD7)
                context.tag(__FILE__, __LINE__, "UTF-16 LE code",
                            "2 bytes U+0001..U+D7FF (" +
                                to_string(static_cast<double>(0xD7FF - 0x0001 + 1) / rangeSize * 100) + "%)");
            else if (0xE0 <= c0)
                context.tag(__FILE__, __LINE__, "UTF-16 LE code",
                            "2 bytes U+E000..U+FFFF (" +
                                to_string(static_cast<double>(0xFFFF - 0xE000 + 1) / rangeSize * 100) + "%)");
            else
                context.tag(__FILE__, __LINE__, "UTF-16 LE code",
                            "2 byte error: " + to_string(static_cast<int>(c0)));
        } else if (str.size() == 3) {
            context.tag(__FILE__, __LINE__, "UTF-16 LE code", "3 bytes error");
        } else if (str.size() == 4) {
            uint8_t c2 = str[3];
            if (0xD8 <= c0 && c0 <= 0xDB && 0xDC <= c2 && c2 <= 0xDF)
                context.tag(__FILE__, __LINE__, "UTF-16 LE code",
                            "4 bytes U+10000..U+10FFFF(" +
                                to_string(static_cast<double>(0x10FFFF - 0x10000 + 1) / rangeSize * 100) + "%)");
            else
                context.tag(__FILE__, __LINE__, "UTF-16 LE code", "4 bytes error");
        } else
            context.tag(__FILE__, __LINE__, "UTF-16 LE code", "error");
    }
}

void testCESU8(PropertyContext& context, Random rand)
{
    Arbi<CESU8String> gen;
    gen.setSize(1);

    for (int i = 0; i < 100000; i++) {
        CESU8String str = static_cast<CESU8String>(gen(rand).getRef());
        uint8_t c0 = str[0];

        context.tag(__FILE__, __LINE__, "charsize", to_string(str.charsize()));

        if (str.size() == 1) {
            if (c0 <= 0x7f)
                context.tag(__FILE__, __LINE__, "CESU-8 code", "1 byte U+0001..U+007F");
            else
                context.tag(__FILE__, __LINE__, "CESU-8 code", "1 byte error");
        } else if (str.size() == 2) {
            uint8_t c1 = str[1];
            if (0xc2 <= c0 && c0 <= 0xdf && 0x80 <= c1 && c1 <= 0xbf)
                context.tag(__FILE__, __LINE__, "CESU-8 code", "2 bytes U+0080..U+07FF");
            else
                context.tag(__FILE__, __LINE__, "CESU-8 code", "2 byte error");
        } else if (str.size() == 3) {
            uint8_t c1 = str[1];
            uint8_t c2 = str[2];
            if (0xe0 == c0 && 0xa0 <= c1 && c1 <= 0xbf && 0x80 <= c2 && c2 <= 0xbf)
                context.tag(__FILE__, __LINE__, "CESU-8 code", "3 bytes U+0800..U+0FFF");
            else if (0xe1 <= c0 && c0 <= 0xec && 0x80 <= c1 && c1 <= 0xbf && 0x80 <= c2 && c2 <= 0xbf)
                context.tag(__FILE__, __LINE__, "CESU-8 code", "3 bytes U+1000..U+CFFF");
            else if (0xed == c0 && 0x80 <= c1 && c1 <= 0x9f && 0x80 <= c2 && c2 <= 0xbf)
                context.tag(__FILE__, __LINE__, "CESU-8 code", "3 bytes U+D000..U+D7FF");
            else if (0xee <= c0 && c0 <= 0xef && 0x80 <= c1 && c1 <= 0xbf && 0x80 <= c2 && c2 <= 0xbf)
                context.tag(__FILE__, __LINE__, "CESU-8 code", "3 bytes U+E000..U+FFFF");
            else
                context.tag(__FILE__, __LINE__, "CESU-8 code", "3 bytes error");
        } else if (str.size() == 6) {
            uint8_t c1 = str[1];
            uint8_t c2 = str[2];
            uint8_t c3 = str[3];
            uint8_t c4 = str[4];
            uint8_t c5 = str[5];
            if (0xed == c0 && 0xa0 <= c1 && c1 <= 0xaf && 0x80 <= c2 && c2 <= 0xbf && 0xed == c3 && 0xb0 <= c4 &&
                c4 <= 0xbf && 0x80 <= c5 && c5 <= 0xbf)
                context.tag(__FILE__, __LINE__, "CESU-8 code", "6 bytes U+10000..U+10FFFF");
            else
                context.tag(__FILE__, __LINE__, "CESU-8 code", "6 bytes error");
        } else
            context.tag(__FILE__, __LINE__, "CESU-8 code", "error");
        // cout << "str: " << static_cast<CESU8String>(gen(rand).getRef()) << endl;
    }
}

TEST(PropTest, GenUnicodeString)
{
    int64_t seed = getCurrentTime();
    Random rand(seed);

    PropertyContext context;
    testUTF32(context, rand);
    testUTF16BE(context, rand);
    testUTF16LE(context, rand);
    testUTF8(context, rand);
    testCESU8(context, rand);
    context.printSummary();
}

TEST(PropTest, GenUTF8String)
{
    int64_t seed = getCurrentTime();
    Random rand(seed);
    Arbi<UTF8String> gen;
    gen.setMaxSize(8);
    for (int i = 0; i < 3; i++)
        exhaustive(gen(rand), 0);
}

TEST(PropTest, GenUTF8String2)
{
    int64_t seed = getCurrentTime();
    Random rand(seed);
    Arbi<UTF8String> gen;
    gen.setSize(1, 3);

    for (int i = 0; i < 3; i++) {
        auto shr = gen(rand);
        cout << "str: \"" << static_cast<UTF8String>(shr.getRef()) << "\"" << endl;
        exhaustive(shr, 0);
    }
}

TEST(PropTest, GenUTF16BEString)
{
    int64_t seed = getCurrentTime();
    Random rand(seed);
    Arbi<UTF16BEString> gen;
    gen.setMaxSize(8);
    for (int i = 0; i < 3; i++)
        exhaustive(gen(rand), 0);
}

TEST(PropTest, GenUTF16BEString2)
{
    int64_t seed = getCurrentTime();
    Random rand(seed);
    Arbi<UTF16BEString> gen;
    gen.setSize(1, 3);

    for (int i = 0; i < 3; i++) {
        auto shr = gen(rand);
        cout << "str: \"" << static_cast<UTF16BEString>(shr.getRef()) << "\"" << endl;
        exhaustive(shr, 0);
    }
}

TEST(PropTest, GenUTF16LEString)
{
    int64_t seed = getCurrentTime();
    Random rand(seed);
    Arbi<UTF16LEString> gen;
    gen.setMaxSize(8);
    for (int i = 0; i < 3; i++)
        exhaustive(gen(rand), 0);
}

TEST(PropTest, GenUTF16LEString2)
{
    int64_t seed = getCurrentTime();
    Random rand(seed);
    Arbi<UTF16LEString> gen;
    gen.setSize(1, 3);

    for (int i = 0; i < 3; i++) {
        auto shr = gen(rand);
        cout << "str: \"" << static_cast<UTF16LEString>(shr.getRef()) << "\"" << endl;
        exhaustive(shr, 0);
    }
}

TEST(PropTest, GenCESU8String)
{
    int64_t seed = getCurrentTime();
    Random rand(seed);
    Arbi<CESU8String> gen;
    gen.setMaxSize(8);
    for (int i = 0; i < 3; i++)
        exhaustive(gen(rand), 0);
}

TEST(PropTest, GenCESU8String2)
{
    int64_t seed = getCurrentTime();
    Random rand(seed);
    Arbi<CESU8String> gen;
    gen.setSize(1, 3);

    for (int i = 0; i < 3; i++) {
        auto shr = gen(rand);
        cout << "str: \"" << static_cast<CESU8String>(shr.getRef()) << "\"" << endl;
        exhaustive(shr, 0);
    }
}

TEST(PropTest, GenSharedPtr)
{
    int64_t seed = getCurrentTime();
    Random rand(seed);
    Arbi<shared_ptr<bool>> gen;
    for (int i = 0; i < 20; i++) {
        auto shr = gen(rand);
        auto ptr = shr.getRef();
        auto value = *ptr;
        cout << "int: " << value << endl;
    }
}

TEST(PropTest, GenSet)
{
    int64_t seed = getCurrentTime();
    Random rand(seed);
    Arbi<set<int>> gen;
    gen.setMaxSize(8);
    auto shr = gen(rand);
    exhaustive(shr, 0);
    // for (int i = 0; i < 20; i++) {
    //     cout << "int: " << *gen(rand).getRef() << endl;
    // }
}

TEST(PropTest, GenMap)
{
    int64_t seed = getCurrentTime();
    Random rand(seed);
    Arbi<map<int, int>> gen;
    gen.setMaxSize(8);
    auto shr = gen(rand);
    exhaustive(shr, 0);
    // for (int i = 0; i < 20; i++) {
    //     cout << "int: " << *gen(rand).getRef() << endl;
    // }
}

TEST(PropTest, GenMap2)
{
    int64_t seed = getCurrentTime();
    Random rand(seed);
    Arbi<map<int, int>> gen;
    gen.setMaxSize(8);
    gen.setElemGen(integers<int>(0, 100));
    gen.setKeyGen(Arbi<int>());
    auto shr = gen(rand);
    exhaustive(shr, 0);
    // for (int i = 0; i < 20; i++) {
    //     cout << "int: " << *gen(rand).getRef() << endl;
    // }
}

TEST(PropTest, GenList)
{
    int64_t seed = getCurrentTime();
    Random rand(seed);
    Arbi<list<int>> gen(integers<int>(0, 100));
    gen.setMaxSize(8);
    auto shr = gen(rand);
    exhaustive(shr, 0);
    // for (int i = 0; i < 20; i++) {
    //     cout << "int: " << *gen(rand).getRef() << endl;
    // }
}

TEST(PropTest, DISABLED_GenLargeVector)
{
    int64_t seed = getCurrentTime();
    Random rand(seed);
    for(int i = 1 ; i < INT_MAX/16; i*=2) {
        static uint64_t time = getCurrentTime();
        cout << "starting generation: vector of size " << i << endl;
        Arbi<vector<int>> gen(integers<int>(0, 100));
        gen.setSize(i);
        gen(rand);
        uint64_t elapsed = (getCurrentTime() - time) / 1000;
        cout << "vector of size " << i << " generated in " << elapsed << " seconds" << endl;
    }
}

TEST(PropTest, GenNullable)
{
    int64_t seed = getCurrentTime();
    Random rand(seed);
    Arbi<list<int>> gen(integers<int>(0, 100));
    gen.setMaxSize(8);
    Arbi<Nullable<list<int>>> nullableGen(gen);
    auto shr = nullableGen(rand);
    // exhaustive(shr, 0);

    for (int i = 0; i < 100; i++) {
        cout << "int: " << nullableGen(rand).getRef() << endl;
    }

    // exhaustive(nullableGen(rand), 0);
}
