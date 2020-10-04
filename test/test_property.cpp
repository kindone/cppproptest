#include "testbase.hpp"

using namespace proptest;

TEST(PropTest, TestCheckAssert)
{
    forAll([](std::string a, int i, std::string b) -> bool {
        if (i % 2 == 0)
            PROP_DISCARD();
        PROP_EXPECT_STREQ2(a.c_str(), b.c_str(), a.size(), b.size());
        return true;
    });

    forAll([](std::string a, int i, std::string b) -> bool {
        if (i % 2 == 0)
            PROP_SUCCESS();

        PROP_ASSERT_STREQ2(a.c_str(), b.c_str(), a.size(), b.size());
        PROP_STAT(i < 0);
        return true;
    });
}

TEST(PropTest, TestPropertyBasic)
{
    property([](std::vector<int> a) -> bool {
        PROP_STAT(a.size() > 5);
        return true;
    }).forAll();

    auto func = [](std::vector<int>) -> bool { return true; };

    property(func).forAll();
    forAll(func);

    auto prop = property([](std::string, int i, std::string) -> bool {
        PROP_STAT(i > 0);
        PROP_ASSERT(false);
        return true;
    });

    // chaining
    prop.setSeed(0).forAll();
    // with specific arguments
    prop.example(std::string("hello"), 10, std::string("world"));
}

TEST(PropTest, TestPropertyExample)
{
    auto func = [](std::string, int i, std::string) -> bool {
        PROP_STAT(i > 0);
        return false;
    };
    auto prop = property(func);
    // with specific arguments
    EXPECT_FALSE(prop.example(std::string("hello"), 10, std::string("world")));
}

TYPED_TEST(SignedNumericTest, TestCheckFail)
{
    forAll([](TypeParam a, TypeParam b /*,std::string str, std::vector<int> vec*/) -> bool {
        PROP_ASSERT(-10 < a && a < 100 && -20 < b && b < 200);
        return true;
    });
}

TEST(PropTest, TestCheckBasic)
{
    forAll([](const int& a, const int& b) -> bool {
        EXPECT_EQ(a + b, b + a);
        PROP_STAT(a + b > 0);
        return true;
    });

    forAll([](int a) -> bool {
        PROP_STAT(a > 0);
        return true;
    });

    std::string a = "Hello";
    std::string b = "World";
    forAll([](std::string a, std::string b) -> bool {
        std::string c /*(allocator())*/, d /*(allocator())*/;
        c = a + b;
        d = b + a;
        EXPECT_EQ(c.size(), d.size());
        EXPECT_EQ((a + b).substr(0, a.length()), a);
        EXPECT_EQ((a + b).substr(a.length()), b);
        return true;
    });

    forAll([=](UTF8String a, UTF8String b) -> bool {
        std::string c /*(allocator())*/, d /*(allocator())*/;
        c = a + b;
        d = b + a;
        PROP_ASSERT(c.size() == d.size());
        EXPECT_EQ(c.size(), d.size());
        // ASSERT_EQ(c.size(), d.size());
        // EXPECT_EQ(c, d);// << "a: " << a << " + b: " << b << ", a+b: " << (a+b) << ", b+a: " << (b+a);
        return true;
    });
}

struct Bit
{
    uint8_t v;
    const uint16_t len;
    bool null;

    Bit(uint8_t vbit, bool null) : v(vbit), len(sizeof(uint8_t)), null(null) {}
    ~Bit() {}
};

namespace proptest {

template <>
class Arbi<Bit> : public GenBase<Bit> {
public:
    Shrinkable<Bit> operator()(Random& rand)
    {
        static auto gen_v =
            proptest::transform<uint8_t, uint8_t>(Arbi<uint8_t>(), [](uint8_t& vbit) { return (1 << 0) & vbit; });
        static auto gen_bit = construct<Bit, uint8_t, bool>(gen_v, Arbi<bool>());
        return gen_bit(rand);
    }
};
}  // namespace proptest

TEST(PropTest, TestCheckBit)
{
    forAll([](Bit bit) {
        PROP_STAT(bit.v == 1);
        PROP_STAT(bit.v != 1 && bit.v != 0);
    });
}

TEST(PropTest, TestCheckWithGen)
{
    /*check([](std::vector<int> a) -> bool {
        std::cout << "a: " << a << std::endl;
        PROP_TAG("a.size() > 0", a.size() > 5);
        return true;
    });*/

    // supply custom generator
    forAll(
        [](int a, int b) {
            PROP_STAT(a > 0);
            PROP_STAT(b > 0);
        },
        GenSmallInt(), GenSmallInt());

    //
    forAll(
        [](int a, int b) {
            PROP_STAT(a > 0);
            PROP_STAT(b > 0);
        },
        GenSmallInt());

    GenSmallInt genSmallInt;

    forAll(
        [](int a, int b) {
            PROP_STAT(a > 0);
            PROP_STAT(b > 0);
        },
        genSmallInt, genSmallInt);

    forAll(
        [](int a, std::string b) {
            PROP_STAT(a > 0);
            PROP_STAT(b.size() > 0);
        },
        genSmallInt);
}

TEST(PropTest, TestStringCheckFail)
{
    forAll([](std::string a) {
        PROP_STAT(a.size() > 3);
        PROP_ASSERT(a.size() < 5);
    });
}

TEST(PropTest, TestUnicodeStringCheckFail)
{
    forAll([](UTF8String a) {
        PROP_STAT(a.size() > 3);
        PROP_ASSERT(a.size() < 100);
    });

    forAll([](CESU8String a) {
        PROP_STAT(a.size() > 3);
        PROP_ASSERT(a.size() < 100);
    });

    forAll([](UTF16BEString a) {
        PROP_STAT(a.size() > 3);
        PROP_ASSERT(a.size() < 100);
    });

    forAll([](UTF16LEString a) {
        PROP_STAT(a.size() > 3);
        PROP_ASSERT(a.size() < 100);
    });
}

TEST(PropTest, TestStringCheckFail2)
{
    forAll([](std::string a) {
        PROP_STAT(a.size() > 3);
        PROP_EXPECT(a.size() < 5);
    });

    forAll([](std::string a) {
        PROP_STAT(a.size() > 3);
        return a.size() < 5;
    });

    forAll([](std::string a) {
        PROP_STAT(a.size() > 3);
        PROP_EXPECT(a.size() < 5);
        PROP_EXPECT_LT(a.size(), 6);
    });
}

TEST(PropTest, TestVectorCheckFail)
{
    std::vector<int> vec;
    vec.push_back(5);
    auto tup = std::make_tuple(vec);
    // std::cout << "tuple: ";
    show(std::cout, tup);
    std::cout << std::endl;

    auto vecGen = Arbi<std::vector<int>>();
    vecGen.setMaxSize(32);

    forAll(
        [](std::vector<int> a) {
            PROP_STAT(a.size() > 3);
            show(std::cout, a);
            std::cout << std::endl;
            PROP_EXPECT_LT(a.size(), 5) << "synthesized failure1";
            PROP_EXPECT_LT(a.size(), 4) << "synthesized failure2";
        },
        vecGen);
}

TEST(PropTest, TestTupleCheckFail)
{
    forAll([](std::tuple<int, std::tuple<int>> tuple) {
        // std::cout << "tuple: ";
        // show(std::cout, tuple);
        // std::cout << std::endl;
        int a = std::get<0>(tuple);
        std::tuple<int> subtup = std::get<1>(tuple);
        int b = std::get<0>(subtup);
        PROP_ASSERT((-10 < a && a < 100) || (-20 < b && b < 200));
    });
}

bool propertyAsFunc(std::string, int, std::vector<int>)
{
    return true;
}

class PropertyAsClass {
public:
    bool operator()(std::string, int, std::vector<int>) { return true; }

    static bool propertyAsMethod(std::string, int, std::vector<int>) { return true; }
};

TEST(PropTest, TestPropertyFunctionLambdaMethod)
{
    property(propertyAsFunc).forAll();
    forAll(propertyAsFunc);

    PropertyAsClass propertyAsClass;
    property(propertyAsClass).forAll();
    forAll(propertyAsClass);

    property(PropertyAsClass::propertyAsMethod).forAll();
    forAll(PropertyAsClass::propertyAsMethod);
}

namespace proptest {
namespace util {

template <>
struct ShowDefault<Animal>
{
    static std::ostream& show(std::ostream& os, const Animal& a)
    {
        os << "numFeet: " << a.numFeet << ", name: " << a.name << ", measures: ";
        if (!a.measures.empty()) {
            os << a.measures[0];
            for (auto measure = a.measures.begin() + 1; measure != a.measures.end(); ++measure) {
                os << ", " << *measure;
            }
        }
        return os;
    }
};

}  // namespace util

}  // namespace proptest

TEST(PropTest, TestCheckArbitraryWithConstruct)
{
    int64_t seed = getCurrentTime();
    Random rand(seed);

    auto vecGen = Arbi<std::vector<int>>();
    vecGen.setMaxSize(20);
    auto animal = construct<Animal, int, std::string, std::vector<int>&>(Arbi<int>(), Arbi<std::string>(), vecGen);
    auto animalVecGen = Arbi<std::vector<Animal>>(animal);
    animalVecGen.setMaxSize(20);

    EXPECT_FOR_ALL(
        [](std::vector<Animal> animals) {
            // std::cout << "animal " << i++ << std::endl;
            if (!animals.empty()) {
                for (auto animal : animals) {
                    if (animal.name.size() < 5 && animal.measures.size() < 5)
                        show(std::cout, animal);
                }
                PROP_STAT(animals.size() > 3);
            }
        },
        animalVecGen);
}

decltype(auto) dummyProperty()
{
    using Type = std::function<int()>;
    std::shared_ptr<Type> modelPtr = std::make_shared<Type>([]() { return 0; });
    return property([modelPtr](int) {
        auto model = *modelPtr;
        PROP_STAT(model() > 2);
    });
}

TEST(StateTest, PropertyCapture)
{
    auto prop = dummyProperty();
    prop.forAll();
}

TEST(PropTest, DISABLED_TestExpectDeath)
{
    forAll([](std::vector<int> vec, uint64_t n) {
        auto dangerous = [&vec, n]() { vec[vec.size() - 1 + n] = 100; };
        dangerous();
        // EXPECT_DEATH(, ".*") << "vector: " << vec.size() << ", n: " << n;
    });
}
