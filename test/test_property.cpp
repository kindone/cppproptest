#include "testbase.hpp"

using namespace PropertyBasedTesting;

TEST(PropTest, TestCheckAssert)
{
    check([](std::string a, int i, std::string b) -> bool {
        if (i % 2 == 0)
            PROP_DISCARD();
        return true;
    });

    check([](std::string a, int i, std::string b) -> bool {
        if (i % 2 == 0)
            PROP_SUCCESS();
        PROP_DISCARD();
        return true;
    });
}

TEST(PropTest, TestPropertyBasic)
{
    property([](std::vector<int> a) -> bool {
        PROP_STAT(a.size() > 5);
        return true;
    }).check();

    auto func = [](std::vector<int> a) -> bool { return true; };

    property(func).check();
    check(func);

    auto prop = property([](std::string a, int i, std::string b) -> bool {
        PROP_STAT(i > 0);
        PROP_ASSERT(false);
        return true;
    });

    // chaining
    prop.setSeed(0).check();
    // with specific arguments
    prop.example(std::string("hello"), 10, std::string("world"));
}

TEST(PropTest, TestPropertyExample)
{
    auto func = [](std::string a, int i, std::string b) -> bool {
        PROP_STAT(i > 0);
        return false;
    };
    auto prop = property(func);
    // with specific arguments
    EXPECT_FALSE(prop.example(std::string("hello"), 10, std::string("world")));
}

TYPED_TEST(SignedNumericTest, TestCheckFail)
{
    check([](TypeParam a, TypeParam b /*,std::string str, std::vector<int> vec*/) -> bool {
        PROP_ASSERT(-10 < a && a < 100 && -20 < b && b < 200);
        return true;
    });
}

TEST(PropTest, TestCheckBasic)
{
    check([](const int& a, const int& b) -> bool {
        EXPECT_EQ(a + b, b + a);
        PROP_STAT(a + b > 0);
        return true;
    });

    check([](int a) -> bool {
        PROP_STAT(a > 0);
        return true;
    });

    check([](std::string a, std::string b) -> bool {
        std::string c /*(allocator())*/, d /*(allocator())*/;
        c = a + b;
        d = b + a;
        EXPECT_EQ(c.size(), d.size());
        return true;
    });

    check([=](UTF8String a, UTF8String b) -> bool {
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

namespace PropertyBasedTesting {

template <>
class Arbitrary<Bit> : public Gen<Bit> {
public:
    Shrinkable<Bit> operator()(Random& rand)
    {
        static auto gen_v =
            transform<uint8_t, uint8_t>(Arbitrary<uint8_t>(), [](const uint8_t& vbit) { return (1 << 0) & vbit; });
        static auto gen_bit = construct<Bit, uint8_t, bool>(gen_v, Arbitrary<bool>());
        return gen_bit(rand);
    }
};
}  // namespace PropertyBasedTesting

TEST(PropTest, TestCheckBit)
{
    check([](Bit bit) {
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
    check(
        [](int a, int b) {
            PROP_STAT(a > 0);
            PROP_STAT(b > 0);
        },
        GenSmallInt(), GenSmallInt());

    //
    check(
        [](int a, int b) {
            PROP_STAT(a > 0);
            PROP_STAT(b > 0);
        },
        GenSmallInt());

    GenSmallInt genSmallInt;

    check(
        [](int a, int b) {
            PROP_STAT(a > 0);
            PROP_STAT(b > 0);
        },
        genSmallInt, genSmallInt);

    check(
        [](int a, std::string b) {
            PROP_STAT(a > 0);
            PROP_STAT(b.size() > 0);
        },
        genSmallInt);
}

TEST(PropTest, TestStringCheckFail)
{
    check([](std::string a) {
        PROP_STAT(a.size() > 3);
        PROP_ASSERT(a.size() < 5);
    });
}

TEST(PropTest, TestStringCheckFail2)
{
    check([](std::string a) {
        PROP_STAT(a.size() > 3);
        PROP_EXPECT_TRUE(a.size() < 5);
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

    auto vecGen = Arbitrary<std::vector<int>>();
    vecGen.setMaxSize(32);

    check(
        [](std::vector<int> a) {
            PROP_STAT(a.size() > 3);
            show(std::cout, a);
            std::cout << std::endl;
            PROP_EXPECT_LT(a.size(), 5);
        },
        vecGen);
}

TEST(PropTest, TestTupleCheckFail)
{
    check([](std::tuple<int, std::tuple<int>> tuple) {
        // std::cout << "tuple: ";
        // show(std::cout, tuple);
        // std::cout << std::endl;
        int a = std::get<0>(tuple);
        std::tuple<int> subtup = std::get<1>(tuple);
        int b = std::get<0>(subtup);
        PROP_ASSERT((-10 < a && a < 100) || (-20 < b && b < 200));
    });
}

bool propertyAsFunc(std::string a, int i, std::vector<int> v)
{
    return true;
}

class PropertyAsClass {
public:
    bool operator()(std::string a, int i, std::vector<int> v) { return true; }

    static bool propertyAsMethod(std::string a, int i, std::vector<int> v) { return true; }
};

TEST(PropTest, TestPropertyFunctionLambdaMethod)
{
    property(propertyAsFunc).check();
    check(propertyAsFunc);

    PropertyAsClass propertyAsClass;
    property(propertyAsClass).check();
    check(propertyAsClass);

    property(PropertyAsClass::propertyAsMethod).check();
    check(PropertyAsClass::propertyAsMethod);
}

TEST(PropTest, TestCheckArbitraryWithConstruct)
{
    int64_t seed = getCurrentTime();
    Random rand(seed);

    int i = 0;

    auto vecGen = Arbitrary<std::vector<int>>();
    vecGen.setMaxSize(20);
    auto animal =
        construct<Animal, int, std::string, std::vector<int>&>(Arbitrary<int>(), Arbitrary<std::string>(), vecGen);
    auto animalVecGen = Arbitrary<std::vector<Animal>>(animal);
    animalVecGen.setMaxSize(20);

    check(
        [&i](std::vector<Animal> animals) {
            // std::cout << "animal " << i++ << std::endl;
            if (!animals.empty()) {
                PROP_STAT(animals.size() > 3);
            }
        },
        animalVecGen);
}

decltype(auto) dummyProperty()
{
    using Type = std::function<int()>;
    std::shared_ptr<Type> modelPtr = std::make_shared<Type>([]() { return 0; });
    return property([modelPtr](int dummy) {
        auto model = *modelPtr;
        PROP_STAT(model() > 2);
    });
}

TEST(StateTest, PropertyCapture)
{
    auto prop = dummyProperty();
    prop.check();
}
