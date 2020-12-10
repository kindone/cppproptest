#include "proptest.hpp"
#include "googletest/googletest/include/gtest/gtest.h"
#include "googletest/googlemock/include/gmock/gmock.h"
#include "Random.hpp"
#include "../util/std.hpp"

class UtilTestCase : public ::testing::Test {
};

using namespace proptest;
using namespace proptest::util;

extern ostream& operator<<(ostream& os, const vector<int>& input);

TEST(UtilTestCase, invokeTest)
{
    int arg1 = 5;
    vector<int> arg2;
    string arg3("hello");

    auto func = [](int i, vector<int> v, string s) {
        cout << "i: " << i << " v: " << v << " s: " << s << endl;
    };

    invokeWithArgTuple(func, util::make_tuple(arg1, arg2, arg3));
    invokeWithArgs(func, util::make_tuple(arg1, arg2, arg3));
}

TEST(UtilTestCase, transformTest)
{
    transformTuple(transformTuple(util::make_tuple(5, 6, 7),
                                  [](int i) {
                                      cout << "n: " << i << endl;
                                      return to_string(i);
                                  }),
                   [](string s) {
                       cout << "s: " << s << endl;
                       return s.size();
                   });
}

template <typename IN>
struct Transformer;

template <>
struct Transformer<int>
{
    static string transform(int&& v)
    {
        cout << "Transformer<int> - " << v << endl;
        return to_string(v + 1);
    }
};

template <>
struct Transformer<string>
{
    static int transform(string&& v)
    {
        cout << "Transformer<string> - " << v << endl;
        return v.size();
    }
};

TEST(UtilTestCase, transformHeteroTest)
{
    transformHeteroTuple<Transformer>(util::make_tuple(5, 6, 7));
}

TEST(UtilTestCase, transformHeteroTest2)
{
    transformHeteroTuple<Transformer>(transformHeteroTuple<Transformer>(util::make_tuple(5, 6, 7)));
}

TEST(UtilTestCase, transformHeteroTest3)
{
    int a = 5;
    string b("a");
    transformHeteroTuple<Transformer>(transformHeteroTuple<Transformer>(util::make_tuple(a, b, 7)));
}

template <typename T>
struct Bind
{
    string to_string(int a) { return to_string(a); }
};

template <template <typename> class T, typename P>
decltype(auto) callToString(int value)
{
    T<P> t;
    bind(&T<P>::to_string, &t, value);
}

TEST(UtilTestCase, stdbind)
{
    int arg1 = 5;
    vector<int> arg2;
    string arg3("hello");

    auto func = [](int i, vector<int> v, string s) {
        cout << "i: " << i << " v: " << v << " s: " << s << endl;
    };

    bind(func, arg1, arg2, arg3);

    Bind<int> b;
    bind(&Bind<int>::to_string, &b, 5);

    callToString<Bind, int>(5);
}

template <typename... ARGS>
decltype(auto) doTuple(tuple<ARGS...>&)
{
    util::TypeList<typename remove_reference<ARGS>::type...> typeList;
    return typeList;
}

TEST(UtilTestCase, TypeList)
{
    auto tup = util::make_tuple(1, 2.3, "abc");
    auto res = doTuple(tup);
    using type_tuple = typename decltype(res)::type_tuple;
    auto tup2 = static_cast<type_tuple>(tup);
    EXPECT_EQ(tup, tup2);
}

TEST(UtilTestCase, StreamShrink)
{
    int value = -50;  // 0, -25, ... -49
    // recursive
    using shrinkable_t = Shrinkable<int>;
    using stream_t = Stream<shrinkable_t>;
    // using func_t = typename function<stream_t()>;
    using genfunc_t = function<stream_t(int, int)>;

    // given min, val, generate stream
    static genfunc_t genpos = [](int min, int val) {
        int mid = val / 2 + min / 2;
        if (val <= 0 || (val - min) <= 1 || mid == val || mid == min)
            return stream_t::empty();
        else
            return stream_t(make_shrinkable<int>(mid).with([=]() { return genpos(0, mid); }),
                            [=]() { return genpos(mid, val); });
    };

    static genfunc_t genneg = [](int max, int val) {
        int mid = val / 2 + max / 2;
        // cout << "      val: " << val << ", mid: " << mid << ", max: " << max << endl;
        if (val >= 0 || (max - val) <= 1 || mid == val || mid == max)
            return stream_t::empty();
        else
            return stream_t(make_shrinkable<int>(mid).with([=]() { return genneg(0, mid); }),
                            [=]() { return genneg(mid, val); });
    };

    // cout << "      val0: " << value << endl;
    auto shr = make_shrinkable<int>(value).with([value]() {
        // cout << "      val1: " << value << endl;
        return stream_t(make_shrinkable<int>(0), [value]() {
            // cout << "      val2: " << value << endl;
            if (value >= 0)
                return genpos(0, value);
            else
                return genneg(0, value);
        });
    });

    for (auto itr = shr.shrinks().iterator(); itr.hasNext();) {
        auto shrinkable = itr.next();
        cout << "streamshrink:" << shrinkable.get() << endl;
        for (auto itr2 = shrinkable.shrinks().iterator(); itr2.hasNext();) {
            cout << "  shrink: " << itr2.next().get() << endl;
        }
    }

    Stream<Shrinkable<string>> strstream =
        shr.shrinks().transform<Shrinkable<string>>([](const Shrinkable<int>& value) {
            auto shrinksPtr = value.shrinksPtr;
            return make_shrinkable<string>(to_string(value.get())).with([shrinksPtr]() {
                return (*shrinksPtr)().transform<Shrinkable<string>>(
                    [](const Shrinkable<int>& v) { return make_shrinkable<string>(to_string(v.get())); });
            });
        });

    for (auto itr = strstream.iterator(); itr.hasNext();) {
        auto shrinkable = itr.next();
        cout << "strstreamshrink:" << shrinkable.get() << endl;
        for (auto itr2 = shrinkable.shrinks().iterator(); itr2.hasNext();) {
            cout << "  shrink: " << itr2.next().get() << endl;
        }
    }
}

TEST(UtilTestCase, Shrinkable)
{
    // generates 50,49,...,0
    int val = 50;
    // recursive
    auto stream = Stream<Shrinkable<int>>(make_shrinkable<int>(val), [val]() {
        static function<Stream<Shrinkable<int>>(int)> gen = [](int val) {
            if (val <= 0)
                return Stream<Shrinkable<int>>::empty();
            else
                return Stream<Shrinkable<int>>(make_shrinkable<int>(val - 1), [val]() { return gen(val - 1); });
        };
        return gen(val);
    });

    for (auto itr = stream.iterator(); itr.hasNext();) {
        cout << "stream:" << itr.next().get() << endl;
    }

    auto shrinkable = make_shrinkable<int>(5).with([=]() { return stream; });

    auto stream2 = shrinkable.shrinks();
    for (auto itr = stream.iterator(); itr.hasNext();) {
        cout << "stream2:" << itr.next().get() << endl;
    }
}

TEST(UtilTestCase, ShrinkableNumeric)
{
    // should show 0~7, -7~0
    int values[] = {8, -8};
    for (size_t i = 0; i < 2; i++) {
        int value = values[i];
        auto shrinkable = util::binarySearchShrinkable(value);

        for (auto itr = shrinkable.shrinks().iterator(); itr.hasNext();) {
            auto shrinkable1 = itr.next();
            cout << "strstreamshrink:" << shrinkable1.get() << endl;
            for (auto itr2 = shrinkable1.shrinks().iterator(); itr2.hasNext();) {
                auto shrinkable2 = itr2.next();
                cout << "  shrink: " << shrinkable2.get() << endl;
                for (auto itr3 = shrinkable2.shrinks().iterator(); itr3.hasNext();) {
                    auto shrinkable3 = itr3.next();
                    cout << "    shrink: " << shrinkable3.get() << endl;
                    for (auto itr4 = shrinkable3.shrinks().iterator(); itr4.hasNext();) {
                        auto shrinkable4 = itr4.next();
                        cout << "      shrink: " << shrinkable4.get() << endl;
                    }
                }
            }
        }
    }
}

TEST(UtilTestCase, ShrinkableString)
{
    auto str = string("hello world");
    int len = str.size();
    auto shrinkable = util::binarySearchShrinkable(len).template map<string>(
        [str](const int64_t& len) { return str.substr(0, len); });

    for (auto itr = shrinkable.shrinks().iterator(); itr.hasNext();) {
        auto shrinkable1 = itr.next();
        cout << "strstreamshrink:" << shrinkable1.get() << endl;
        for (auto itr2 = shrinkable1.shrinks().iterator(); itr2.hasNext();) {
            cout << "  shrink: " << itr2.next().get() << endl;
        }
    }
}

struct NoBlank
{
    NoBlank() = delete;
    NoBlank(int) {}
};

struct NoCopy
{
    NoCopy(int) : id(nextId()) { cout << "nocopy create" << id << endl; }
    NoCopy(const NoCopy&) = delete;
    NoCopy& operator=(const NoCopy&) = delete;
    NoCopy(NoCopy&&)
    {
        id = nextId();
        cout << "nocopy move" << id << endl;
    }
    ~NoCopy() { cout << "~nocopy destroy" << id << endl; }

    int id;
    static int maxId;
    static int nextId() { return maxId++; }
};

int NoCopy::maxId = 1;

struct NoMove
{
    NoMove(int) : id(nextId()) { cout << "nomove create" << id << endl; }
    NoMove(const NoMove&)
    {
        id = nextId();
        cout << "nomove copy" << id << endl;
    }
    NoMove(NoMove&& a) = delete;
    ~NoMove() { cout << "~nomove destroy" << id << endl; }

    int id;
    static int maxId;
    static int nextId() { return maxId++; }
};

int NoMove::maxId = 1;

struct NoDelete
{
    ~NoDelete() = delete;
};

TEST(UtilTestCase, ObjectsWithConstraints)
{
    // tuple<NoBlank> t1(NoBlank(5));
    {
        tuple<NoCopy> t2(NoCopy(5));
        tuple<NoCopy> t2_2(util::move(get<0>(t2)));
        tuple<NoCopy> t2_3(util::move(t2_2));
    }

    {
        tuple<NoMove> t3(util::move(NoMove(5)));
        tuple<NoMove> t3_2 = t3;
        // tuple<NoMove> t3_3(util::move(t3));
    }

    // tuple<NoBlank> t1_2  = t1;
    // tuple<NoCopy> t2_2  = t2;
    // get<0>(t1) = NoBlank(5);
    // get<0>(t2) = NoCopy(5);
    // get<0>(t3) = NoMove(5);

    // tuple<NoDelete>(NoDelete(5));
}

TEST(UtilTestCase, RandomBasic)
{
    int64_t seed = getCurrentTime();
    Random rand(seed);
    for (int i = 0; i < 5; i++) {
        cout << "rng: " << rand.getRandomInt32() << endl;
    }

    // reusuability
    Random rand2 = rand;
    Random rand3 = rand;
    for (int i = 0; i < 5; i++) {
        auto r1 = rand.getRandomInt32();
        auto r2 = rand2.getRandomInt32();
        auto r3 = rand3.getRandomInt32();
        EXPECT_EQ(r1, r2);
        EXPECT_EQ(r2, r3);
    }

    for (int i = 0; i < 10; i++) {
        auto r = rand.getRandomSize(0, 2);
        cout << "randomSize(0, 1) : " << r << endl;
    }
}

TEST(UtilTestCase, Random8)
{
    int64_t seed = getCurrentTime();
    Random rand(seed);

    static auto getUInt8 = [](uint64_t num, uint8_t min, uint8_t max) -> uint8_t {
        uint64_t span = max - min + 1;
        return static_cast<uint8_t>((num % span) + min);
    };

    {
        int i = 0;
        // 0
        cout << "num" << i++ << ": " << static_cast<uint64_t>(getUInt8(0, 0, UINT8_MAX)) << endl;
        // UINT8_MAX
        cout << "num" << i++ << ": " << static_cast<uint64_t>(getUInt8(UINT8_MAX, 0, UINT8_MAX)) << endl;
        // UINT8_MAX
        cout << "num" << i++ << ": " << static_cast<uint64_t>(getUInt8(0, UINT8_MAX, UINT8_MAX)) << endl;
        cout << "num" << i++ << ": " << static_cast<uint64_t>(getUInt8(UINT8_MAX, UINT8_MAX, UINT8_MAX))
                  << endl;
        // 0
        cout << "num" << i++ << ": " << static_cast<uint64_t>(getUInt8(0, 0, 0)) << endl;
        cout << "num" << i++ << ": " << static_cast<uint64_t>(getUInt8(UINT8_MAX, 0, 0)) << endl;
    }

    static auto getInt8 = [](uint64_t num, int8_t min, int8_t max) -> int8_t {
        uint64_t span = max - min + 1;
        uint8_t unsignedVal = (getUInt8(num, 0, UINT8_MAX) % span);
        return *reinterpret_cast<int8_t*>(&unsignedVal) + min;
    };

    {
        int i = 0;
        // 0
        cout << "num" << i++ << ": " << static_cast<int64_t>(getInt8(0, 0, INT8_MAX)) << endl;
        // INT8_MAX
        cout << "num" << i++ << ": " << static_cast<int64_t>(getInt8(INT8_MAX, 0, INT8_MAX)) << endl;

        // INT8_MIN
        cout << "num" << i++ << ": " << static_cast<int64_t>(getInt8(0, INT8_MIN, INT8_MAX)) << endl;
        // 0
        cout << "num" << i++ << ": " << static_cast<int64_t>(getInt8(INT8_MAX + 1, INT8_MIN, INT8_MAX))
                  << endl;
        // INT8_MAX
        cout << "num" << i++ << ": " << static_cast<int64_t>(getInt8(UINT8_MAX, INT8_MIN, INT8_MAX)) << endl;

        // INT8_MAX
        cout << "num" << i++ << ": " << static_cast<int64_t>(getInt8(0, INT8_MAX, INT8_MAX)) << endl;
        cout << "num" << i++ << ": " << static_cast<int64_t>(getInt8(INT8_MAX, INT8_MAX, INT8_MAX)) << endl;
        cout << "num" << i++ << ": " << static_cast<int64_t>(getInt8(UINT8_MAX, INT8_MAX, INT8_MAX)) << endl;
        // 0
        cout << "num" << i++ << ": " << static_cast<int64_t>(getInt8(0, 0, 0)) << endl;
        cout << "num" << i++ << ": " << static_cast<int64_t>(getInt8(INT8_MAX, 0, 0)) << endl;
        cout << "num" << i++ << ": " << static_cast<int64_t>(getInt8(UINT8_MAX, 0, 0)) << endl;
        // INT8_MIN
        cout << "num" << i++ << ": " << static_cast<int64_t>(getInt8(0, INT8_MIN, INT8_MIN)) << endl;
        cout << "num" << i++ << ": " << static_cast<int64_t>(getInt8(INT8_MAX, INT8_MIN, INT8_MIN)) << endl;
        cout << "num" << i++ << ": " << static_cast<int64_t>(getInt8(UINT8_MAX, INT8_MIN, INT8_MIN)) << endl;
    }
}

TEST(UtilTestCase, Random64)
{
    int64_t seed = getCurrentTime();
    Random rand(seed);

    static auto getUInt64 = [](uint64_t num, uint64_t min, uint64_t max) -> uint64_t {
        if (min == 0 && max == UINT64_MAX)
            return num;
        uint64_t span = max - min + 1;
        return static_cast<uint64_t>((num % span) + min);
    };

    {
        int i = 0;
        // 0
        cout << "num" << i++ << ": " << static_cast<uint64_t>(getUInt64(0, 0, UINT64_MAX)) << endl;
        // UINT64_MAX
        cout << "num" << i++ << ": " << static_cast<uint64_t>(getUInt64(UINT64_MAX, 0, UINT64_MAX)) << endl;
        // UINT64_MAX
        cout << "num" << i++ << ": " << static_cast<uint64_t>(getUInt64(0, UINT64_MAX, UINT64_MAX)) << endl;
        cout << "num" << i++ << ": " << static_cast<uint64_t>(getUInt64(UINT64_MAX, UINT64_MAX, UINT64_MAX))
                  << endl;
        // 0
        cout << "num" << i++ << ": " << static_cast<uint64_t>(getUInt64(0, 0, 0)) << endl;
        cout << "num" << i++ << ": " << static_cast<uint64_t>(getUInt64(UINT64_MAX, 0, 0)) << endl;
    }

    static auto getInt64 = [](uint64_t num, int64_t min, int64_t max) -> int64_t {
        if (min == INT64_MIN && max == INT64_MAX) {
            uint64_t unsignedVal = getUInt64(num, 0, UINT64_MAX);
            return *reinterpret_cast<int64_t*>(&unsignedVal) + INT64_MIN;
        }

        uint64_t span = max - min + 1;
        uint64_t unsignedVal = (getUInt64(num, 0, UINT64_MAX) % span);
        return *reinterpret_cast<int64_t*>(&unsignedVal) + min;
    };

    {
        int i = 0;
        // 0
        cout << "num" << i++ << ": " << static_cast<int64_t>(getInt64(0, 0, INT64_MAX)) << endl;
        // INT64_MAX
        cout << "num" << i++ << ": " << static_cast<int64_t>(getInt64(INT64_MAX, 0, INT64_MAX)) << endl;

        // INT64_MIN
        cout << "num" << i++ << ": " << static_cast<int64_t>(getInt64(0, INT64_MIN, INT64_MAX)) << endl;
        // 0
        cout << "num" << i++ << ": "
                  << static_cast<int64_t>(getInt64(static_cast<uint64_t>(INT64_MAX) + 1, INT64_MIN, INT64_MAX))
                  << endl;
        // INT64_MAX
        cout << "num" << i++ << ": " << static_cast<int64_t>(getInt64(UINT64_MAX, INT64_MIN, INT64_MAX))
                  << endl;

        // INT64_MAX
        cout << "num" << i++ << ": " << static_cast<int64_t>(getInt64(0, INT64_MAX, INT64_MAX)) << endl;
        cout << "num" << i++ << ": " << static_cast<int64_t>(getInt64(INT64_MAX, INT64_MAX, INT64_MAX))
                  << endl;
        cout << "num" << i++ << ": " << static_cast<int64_t>(getInt64(UINT64_MAX, INT64_MAX, INT64_MAX))
                  << endl;
        // 0
        cout << "num" << i++ << ": " << static_cast<int64_t>(getInt64(0, 0, 0)) << endl;
        cout << "num" << i++ << ": " << static_cast<int64_t>(getInt64(INT64_MAX, 0, 0)) << endl;
        cout << "num" << i++ << ": " << static_cast<int64_t>(getInt64(UINT64_MAX, 0, 0)) << endl;
        // INT8_MIN
        cout << "num" << i++ << ": " << static_cast<int64_t>(getInt64(0, INT64_MIN, INT64_MIN)) << endl;
        cout << "num" << i++ << ": " << static_cast<int64_t>(getInt64(INT64_MAX, INT64_MIN, INT64_MIN))
                  << endl;
        cout << "num" << i++ << ": " << static_cast<int64_t>(getInt64(UINT64_MAX, INT64_MIN, INT64_MIN))
                  << endl;
    }
}

TEST(UtilTestCase, asserts)
{
    int a = 5;
    int b = 4;
    int c = 5;
    PropertyContext context;
    PROP_EXPECT_EQ(a, c);
    PROP_EXPECT_EQ(a, c) << " should not print";
    PROP_EXPECT_EQ(a, b);
    PROP_EXPECT_EQ(a, b) << " should print";
    PROP_EXPECT_LT(a, b);
    PROP_EXPECT_GT(a, b) << " should print";
}
