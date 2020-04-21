#include "proptest.hpp"
#include "googletest/googletest/include/gtest/gtest.h"
#include "googletest/googlemock/include/gmock/gmock.h"
#include "Random.hpp"
#include <iostream>

class UtilTestCase : public ::testing::Test {
};


using namespace PropertyBasedTesting;

extern std::ostream& operator<<(std::ostream& os, const std::vector<int> &input);

TEST(UtilTestCase, invokeTest) {
    int arg1 = 5;
    std::vector<int> arg2;
    std::string arg3("hello");

    auto func = [](int i, std::vector<int> v, std::string s) {
        std::cout << "i: " << i << " v: " << v << " s: " << s << std::endl;
    };

    invokeWithArgTuple(func, std::make_tuple(arg1, arg2, arg3));
    invokeWithArgs(func, arg1, arg2, arg3);
}

TEST(UtilTestCase, transformTest) {
    transformTuple(transformTuple(std::make_tuple(5,6,7), [](int i) {
        std::cout << "n: " << i << std::endl;
        return std::to_string(i);
    }), [](std::string s) {
        std::cout << "s: " << s << std::endl;
        return s.size();
    });
}


template <typename IN>
struct Transformer;

template <>
struct Transformer<int> {
    static std::string transform(int&& v) {
        std::cout << "Transformer<int> - " << v << std::endl;
        return std::to_string(v + 1);
    }

};

template <>
struct Transformer<std::string> {
    static int transform(std::string&& v) {
        std::cout << "Transformer<string> - " << v << std::endl;
        return v.size();
    }
};

TEST(UtilTestCase, transformHeteroTest) {
    transformHeteroTuple<Transformer>(std::make_tuple(5,6,7));
}


TEST(UtilTestCase, transformHeteroTest2) {
    transformHeteroTuple<Transformer>(transformHeteroTuple<Transformer>(std::make_tuple(5,6,7)));
}



TEST(UtilTestCase, transformHeteroTest3) {
    int a = 5;
    std::string b("a");
    transformHeteroTuple<Transformer>(transformHeteroTuple<Transformer>(std::make_tuple(a,b,7)));
}

template <typename T>
struct Bind {
    std::string to_string(int a) {
        return std::to_string(a);
    }
};

template <template <typename> class T, typename P>
decltype(auto) callToString(int value) {
    T<P> t;
    std::bind(&T<P>::to_string, &t, value);
}

TEST(UtilTestCase, stdbind) {
    int arg1 = 5;
    std::vector<int> arg2;
    std::string arg3("hello");

    auto func = [](int i, std::vector<int> v, std::string s) {
        std::cout << "i: " << i << " v: " << v << " s: " << s << std::endl;
    };

    std::bind(func, arg1, arg2, arg3);

    Bind<int> bind;
    std::bind(&Bind<int>::to_string, &bind, 5);

    callToString<Bind,int>(5);
}

template <typename ...ARGS>
decltype(auto) doTuple(std::tuple<ARGS...>& tup) {
    TypeList<typename std::remove_reference<ARGS>::type...> typeList;
    return typeList;
}

TEST(UtilTestCase, TypeList) {
    auto tup = std::make_tuple(1, 2.3, "abc");
    auto res = doTuple(tup);
    using type_tuple = typename decltype(res)::type_tuple;

}

TEST(UtilTestCase, StreamShrink) {
    int value = -50; // 0, -25, ... -49
    // recursive
    using shrinkable_t = Shrinkable<int>;
    using stream_t = Stream<shrinkable_t>;
    using func_t = typename std::function<stream_t()>;
    using genfunc_t = typename std::function<stream_t(int, int)>;

    // given min, val, generate stream
    static genfunc_t genpos = [](int min, int val) {
        int mid = val/2 + min/2;
        if(val <= 0 || (val-min) <= 1 || mid == val || mid == min)
            return stream_t::empty();
        else
            return stream_t(
                make_shrinkable<int>(mid).with([=]() { return genpos(0, mid);}),
                [=]() { return genpos(mid, val); }
            );
    };

    static genfunc_t genneg = [](int max, int val) {
        int mid = val/2 + max/2;
        //std::cout << "      val: " << val << ", mid: " << mid << ", max: " << max << std::endl;
        if(val >= 0 || (max-val) <= 1 || mid == val || mid == max)
            return stream_t::empty();
        else
            return stream_t(
                make_shrinkable<int>(mid).with([=]() { return genneg(0, mid);}),
                [=]() { return genneg(mid, val); }
            );
    };

    //std::cout << "      val0: " << value << std::endl;
    auto shr = make_shrinkable<int>(value).with([value]() {
        //std::cout << "      val1: " << value << std::endl;
        return  stream_t(make_shrinkable<int>(0), [value]() {
            //std::cout << "      val2: " << value << std::endl;
            if(value >= 0)
                return genpos(0, value);
            else
                return genneg(0, value);
        });
    });


    for(auto itr = shr.shrinks().iterator(); itr.hasNext(); ) {
        auto shrinkable = itr.next();
        std::cout << "streamshrink:" << shrinkable.get() << std::endl;
        for(auto itr2 = shrinkable.shrinks().iterator(); itr2.hasNext(); ) {
            std::cout << "  shrink: " << itr2.next().get() << std::endl;
        }

    }

    Stream<Shrinkable<std::string>> strstream = shr.shrinks().transform<Shrinkable<std::string>>([](const Shrinkable<int>& value) {
        auto shrinksPtr = value.shrinksPtr;
        return make_shrinkable<std::string>(std::to_string(value.get())).with([shrinksPtr]() {
            return (*shrinksPtr)().transform<Shrinkable<std::string>>([](const Shrinkable<int>& v) {
                return make_shrinkable<std::string>(std::to_string(v.get()));
            });
        });
    });


    for(auto itr = strstream.iterator(); itr.hasNext(); ) {
        auto shrinkable = itr.next();
        std::cout << "strstreamshrink:" << shrinkable.get() << std::endl;
        for(auto itr2 = shrinkable.shrinks().iterator(); itr2.hasNext(); ) {
            std::cout << "  shrink: " << itr2.next().get() << std::endl;
        }
    }



}

TEST(UtilTestCase, Shrinkable) {
    // generates 50,49,...,0
    int val = 50;
    // recursive
    auto stream = Stream<Shrinkable<int>>(make_shrinkable<int>(val), [val]() {
        static std::function<Stream<Shrinkable<int>>(int)> gen = [](int val) {
            if(val <= 0)
                return Stream<Shrinkable<int>>::empty();
            else
                return Stream<Shrinkable<int>>(make_shrinkable<int>(val-1), [val]() { return gen(val-1); });
        };
        return gen(val);
    });

    for(auto itr = stream.iterator(); itr.hasNext(); ) {
        std::cout << "stream:" << itr.next().get() << std::endl;
    }

    auto shrinkable = make_shrinkable<int>(5).with([=]() {
        return stream;
    });

    auto stream2 = shrinkable.shrinks();
    for(auto itr = stream.iterator(); itr.hasNext(); ) {
        std::cout << "stream2:" << itr.next().get() << std::endl;
    }
}

TEST(UtilTestCase, ShrinkableNumeric) {
    // should show 0~7, -7~0
    int values[] = {8,-8};
    for(size_t i = 0; i < 2; i++) {
        int value = values[i];
        auto shrinkable =  binarySearchShrinkable<int>(value);

        for(auto itr = shrinkable.shrinks().iterator(); itr.hasNext(); ) {
            auto shrinkable = itr.next();
            std::cout << "strstreamshrink:" << shrinkable.get() << std::endl;
            for(auto itr2 = shrinkable.shrinks().iterator(); itr2.hasNext(); ) {
                auto shrinkable2 = itr2.next();
                std::cout << "  shrink: " << shrinkable2.get() << std::endl;
                for(auto itr3 = shrinkable2.shrinks().iterator(); itr3.hasNext();) {
                    auto shrinkable3 = itr3.next();
                    std::cout << "    shrink: " << shrinkable3.get() << std::endl;
                    for(auto itr4 = shrinkable3.shrinks().iterator(); itr4.hasNext();) {
                        auto shrinkable4 = itr4.next();
                        std::cout << "      shrink: " << shrinkable4.get() << std::endl;
                    }

                }
            }
        }
    }
}

TEST(UtilTestCase, ShrinkableString) {
    auto str = std::string("hello world");
    int len = str.size();
    auto shrinkable =  binarySearchShrinkable<int>(len).transform<std::string>([str](const int& len) {
        return str.substr(0, len);
    });

    for(auto itr = shrinkable.shrinks().iterator(); itr.hasNext(); ) {
        auto shrinkable = itr.next();
        std::cout << "strstreamshrink:" << shrinkable.get() << std::endl;
        for(auto itr2 = shrinkable.shrinks().iterator(); itr2.hasNext(); ) {
            std::cout << "  shrink: " << itr2.next().get() << std::endl;
        }
    }

}

/*
TEST(UtilTestCase, ShrinkableVector) {

    int len = 8;
    std::vector<int> vec;
    for(int i = 0; i < len; i++)
        vec.push_back(i);

    auto shrinkable =  binarySearchShrinkable<int>(value).transform<std::vector<int>>([vec](const int& len) {
        auto begin = vec.begin();
        auto last = vec.begin() + len;
        return std::vector<T>(begin, last);;
    });

    for(auto itr = shrinkable.shrinks().iterator(); itr.hasNext(); ) {
        auto shrinkable = itr.next();
        std::cout << "vecstreamshrink:" << shrinkable.get() << std::endl;
        for(auto itr2 = shrinkable.shrinks().iterator(); itr2.hasNext(); ) {
            auto shrinkable2 = itr2.next();
            std::cout << "  shrink: " << shrinkable2.get() << std::endl;
            for(auto itr3 = shrinkable2.shrinks().iterator(); itr3.hasNext();) {
                auto shrinkable3 = itr3.next();
                std::cout << "    shrink: " << shrinkable3.get() << std::endl;
                for(auto itr4 = shrinkable3.shrinks().iterator(); itr4.hasNext();) {
                    auto shrinkable4 = itr4.next();
                    std::cout << "      shrink: " << shrinkable4.get() << std::endl;
                }

            }
        }
    }
}
*/

struct NoBlank {
    NoBlank() = delete;
    NoBlank(int a) {}
};

struct NoCopy {
    NoCopy(int a) : id(nextId()){
        std::cout << "nocopy create" << id << std::endl;
    }
    NoCopy(const NoCopy&) = delete;
    NoCopy& operator=(const NoCopy&) = delete;
    NoCopy(NoCopy&& a) {
        id = nextId();
        std::cout << "nocopy move" << id << std::endl;
    }
    ~NoCopy() {
        std::cout << "~nocopy destroy" << id << std::endl;
    }

    int id;
    static int maxId;
    static int nextId() {
        return maxId++;
    }
};

int NoCopy::maxId = 1;

struct NoMove {
    NoMove(int a) :id(nextId()){
        std::cout << "nomove create" << id << std::endl;
    }
    NoMove(const NoMove& other) {
        id = nextId();
        std::cout << "nomove copy" << id << std::endl;
    };
    NoMove(NoMove&& a) = delete;
    ~NoMove() {
        std::cout << "~nomove destroy" << id << std::endl;
    }

    int id;
    static int maxId ;
    static int nextId() {
        return maxId++;
    }
};

int NoMove::maxId = 1;

struct NoDelete {
    ~NoDelete() = delete;
};

TEST(UtilTestCase, ObjectsWithConstraints) {
    //std::tuple<NoBlank> t1(NoBlank(5));
    {
    std::tuple<NoCopy> t2(NoCopy(5));
    std::tuple<NoCopy> t2_2(std::move(std::get<0>(t2)));
    std::tuple<NoCopy> t2_3(std::move(t2_2));
    }

    {
    std::tuple<NoMove> t3(std::move(NoMove(5)));
    std::tuple<NoMove> t3_2  = t3;
    //std::tuple<NoMove> t3_3(std::move(t3));
    }

    //std::tuple<NoBlank> t1_2  = t1;
    //std::tuple<NoCopy> t2_2  = t2;
    //std::get<0>(t1) = NoBlank(5);
    //std::get<0>(t2) = NoCopy(5);
    //std::get<0>(t3) = NoMove(5);

    //std::tuple<NoDelete>(NoDelete(5));
}


TEST(UtilTestCase, RandomBasic) {
    int64_t seed = getCurrentTime();
    Random rand(seed);
    for(int i = 0; i < 5; i++) {
        std::cout << "rng: " << rand.getRandomInt32() << std::endl;
    }

    // reusuability
    Random rand2 = rand;
    Random rand3 = rand;
    for(int i = 0; i < 5; i++) {
        auto r1 = rand.getRandomInt32();
        auto r2 = rand2.getRandomInt32();
        auto r3 = rand3.getRandomInt32();
        EXPECT_EQ(r1, r2);
        EXPECT_EQ(r2, r2);
    }
}

TEST(UtilTestCase, Random8) {
    int64_t seed = getCurrentTime();
    Random rand(seed);

    static auto getUInt8 = [](uint64_t num, uint8_t min, uint8_t max) -> uint8_t {
        uint64_t span = max - min + 1;
        return static_cast<uint8_t>((num % span) + min);
    };

    {
    int i = 0;
    // 0
    std::cout << "num" << i++ << ": " << static_cast<uint64_t>(getUInt8(0, 0, UINT8_MAX)) << std::endl;
    // UINT8_MAX
    std::cout << "num" << i++ << ": " << static_cast<uint64_t>(getUInt8(UINT8_MAX, 0, UINT8_MAX)) << std::endl;
    // UINT8_MAX
    std::cout << "num" << i++ << ": " << static_cast<uint64_t>(getUInt8(0, UINT8_MAX, UINT8_MAX)) << std::endl;
    std::cout << "num" << i++ << ": " << static_cast<uint64_t>(getUInt8(UINT8_MAX, UINT8_MAX, UINT8_MAX)) << std::endl;
    // 0
    std::cout << "num" << i++ << ": " << static_cast<uint64_t>(getUInt8(0, 0, 0)) << std::endl;
    std::cout << "num" << i++ << ": " << static_cast<uint64_t>(getUInt8(UINT8_MAX, 0, 0)) << std::endl;

    }

    static auto getInt8 = [](uint64_t num, int8_t min, int8_t max) -> int8_t{
        uint64_t span = max - min + 1;
        uint8_t unsignedVal = (getUInt8(num, 0, UINT8_MAX) % span);
        return *reinterpret_cast<int8_t*>(&unsignedVal) + min;
    };

    {
    int i = 0;
    // 0
    std::cout << "num" << i++ << ": " << static_cast<int64_t>(getInt8(0, 0, INT8_MAX)) << std::endl;
    // INT8_MAX
    std::cout << "num" << i++ << ": " << static_cast<int64_t>(getInt8(INT8_MAX, 0, INT8_MAX)) << std::endl;

    // INT8_MIN
    std::cout << "num" << i++ << ": " << static_cast<int64_t>(getInt8(0, INT8_MIN, INT8_MAX)) << std::endl;
    // 0
    std::cout << "num" << i++ << ": " << static_cast<int64_t>(getInt8(INT8_MAX+1, INT8_MIN, INT8_MAX)) << std::endl;
    // INT8_MAX
    std::cout << "num" << i++ << ": " << static_cast<int64_t>(getInt8(UINT8_MAX, INT8_MIN, INT8_MAX)) << std::endl;

    // INT8_MAX
    std::cout << "num" << i++ << ": " << static_cast<int64_t>(getInt8(0, INT8_MAX, INT8_MAX)) << std::endl;
    std::cout << "num" << i++ << ": " << static_cast<int64_t>(getInt8(INT8_MAX, INT8_MAX, INT8_MAX)) << std::endl;
    std::cout << "num" << i++ << ": " << static_cast<int64_t>(getInt8(UINT8_MAX, INT8_MAX, INT8_MAX)) << std::endl;
    // 0
    std::cout << "num" << i++ << ": " << static_cast<int64_t>(getInt8(0, 0, 0)) << std::endl;
    std::cout << "num" << i++ << ": " << static_cast<int64_t>(getInt8(INT8_MAX, 0, 0)) << std::endl;
    std::cout << "num" << i++ << ": " << static_cast<int64_t>(getInt8(UINT8_MAX, 0, 0)) << std::endl;
    // INT8_MIN
    std::cout << "num" << i++ << ": " << static_cast<int64_t>(getInt8(0, INT8_MIN, INT8_MIN)) << std::endl;
    std::cout << "num" << i++ << ": " << static_cast<int64_t>(getInt8(INT8_MAX, INT8_MIN, INT8_MIN)) << std::endl;
    std::cout << "num" << i++ << ": " << static_cast<int64_t>(getInt8(UINT8_MAX, INT8_MIN, INT8_MIN)) << std::endl;

    }
}

TEST(UtilTestCase, Random64) {
    int64_t seed = getCurrentTime();
    Random rand(seed);

    static auto getUInt64 = [](uint64_t num, uint64_t min, uint64_t max) -> uint64_t {
        if(min == 0 && max == UINT64_MAX)
            return num;
        uint64_t span = max - min + 1;
        return static_cast<uint64_t>((num % span) + min);
    };

    {
    int i = 0;
    // 0
    std::cout << "num" << i++ << ": " << static_cast<uint64_t>(getUInt64(0, 0, UINT64_MAX)) << std::endl;
    // UINT64_MAX
    std::cout << "num" << i++ << ": " << static_cast<uint64_t>(getUInt64(UINT64_MAX, 0, UINT64_MAX)) << std::endl;
    // UINT64_MAX
    std::cout << "num" << i++ << ": " << static_cast<uint64_t>(getUInt64(0, UINT64_MAX, UINT64_MAX)) << std::endl;
    std::cout << "num" << i++ << ": " << static_cast<uint64_t>(getUInt64(UINT64_MAX, UINT64_MAX, UINT64_MAX)) << std::endl;
    // 0
    std::cout << "num" << i++ << ": " << static_cast<uint64_t>(getUInt64(0, 0, 0)) << std::endl;
    std::cout << "num" << i++ << ": " << static_cast<uint64_t>(getUInt64(UINT64_MAX, 0, 0)) << std::endl;

    }

    static auto getInt64 = [](uint64_t num, int64_t min, int64_t max) -> int64_t {
        if(min == INT64_MIN && max == INT64_MAX)
        {
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
    std::cout << "num" << i++ << ": " << static_cast<int64_t>(getInt64(0, 0, INT64_MAX)) << std::endl;
    // INT64_MAX
    std::cout << "num" << i++ << ": " << static_cast<int64_t>(getInt64(INT64_MAX, 0, INT64_MAX)) << std::endl;

    // INT64_MIN
    std::cout << "num" << i++ << ": " << static_cast<int64_t>(getInt64(0, INT64_MIN, INT64_MAX)) << std::endl;
    // 0
    std::cout << "num" << i++ << ": " << static_cast<int64_t>(getInt64(static_cast<uint64_t>(INT64_MAX)+1, INT64_MIN, INT64_MAX)) << std::endl;
    // INT64_MAX
    std::cout << "num" << i++ << ": " << static_cast<int64_t>(getInt64(UINT64_MAX, INT64_MIN, INT64_MAX)) << std::endl;

    // INT64_MAX
    std::cout << "num" << i++ << ": " << static_cast<int64_t>(getInt64(0, INT64_MAX, INT64_MAX)) << std::endl;
    std::cout << "num" << i++ << ": " << static_cast<int64_t>(getInt64(INT64_MAX, INT64_MAX, INT64_MAX)) << std::endl;
    std::cout << "num" << i++ << ": " << static_cast<int64_t>(getInt64(UINT64_MAX, INT64_MAX, INT64_MAX)) << std::endl;
    // 0
    std::cout << "num" << i++ << ": " << static_cast<int64_t>(getInt64(0, 0, 0)) << std::endl;
    std::cout << "num" << i++ << ": " << static_cast<int64_t>(getInt64(INT64_MAX, 0, 0)) << std::endl;
    std::cout << "num" << i++ << ": " << static_cast<int64_t>(getInt64(UINT64_MAX, 0, 0)) << std::endl;
    // INT8_MIN
    std::cout << "num" << i++ << ": " << static_cast<int64_t>(getInt64(0, INT64_MIN, INT64_MIN)) << std::endl;
    std::cout << "num" << i++ << ": " << static_cast<int64_t>(getInt64(INT64_MAX, INT64_MIN, INT64_MIN)) << std::endl;
    std::cout << "num" << i++ << ": " << static_cast<int64_t>(getInt64(UINT64_MAX, INT64_MIN, INT64_MIN)) << std::endl;

    }
}
