#include "testing/proptest.hpp"
#include "googletest/googletest/include/gtest/gtest.h"
#include "googletest/googlemock/include/gmock/gmock.h"
#include "testing/Random.hpp"
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

TEST(UtilTestCase, mapTest) {
    mapTuple(mapTuple(std::make_tuple(5,6,7), [](int i) {
        std::cout << "n: " << i << std::endl;
        return std::to_string(i);
    }), [](std::string s) {
        std::cout << "s: " << s << std::endl;
        return s.size();
    });
}


template <typename IN>
struct Mapper;

template <>
struct Mapper<int> {
    static std::string map(int&& v) {
        std::cout << "Mapper<int> - " << v << std::endl;
        return std::to_string(v + 1);
    }

};

template <>
struct Mapper<std::string> {
    static int map(std::string&& v) {
        std::cout << "Mapper<string> - " << v << std::endl;
        return v.size();
    }

};

TEST(UtilTestCase, mapHeteroTest) {
    mapHeteroTuple<Mapper>(std::make_tuple(5,6,7));
}


TEST(UtilTestCase, mapHeteroTest2) {
    mapHeteroTuple<Mapper>(mapHeteroTuple<Mapper>(std::make_tuple(5,6,7)));
}



TEST(UtilTestCase, mapHeteroTest3) {
    int a = 5;
    std::string b("a");
    mapHeteroTuple<Mapper>(mapHeteroTuple<Mapper>(std::make_tuple(a,b,7)));
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

TEST(UtilTestCase, Stream) {
    int val = -50; // 0, -25, ... -49
    // recursive
    auto stream = Stream<int>(0, [val]() {
        static std::function<Stream<int>(int,int)> genpos = [](int min, int val) {
            if(val <= 0 || (val-min) <= 1)
                return Stream<int>::empty();
            else
                return Stream<int>((val + min)/2, [min, val]() { return genpos((val+min)/2, val); });
        };
        static std::function<Stream<int>(int,int)> genneg = [](int max, int val) {
            if(val >= 0 || (max-val) <= 1)
                return Stream<int>::empty();
            else
                return Stream<int>((val + max)/2, [max, val]() { return genneg((val+max)/2, val); });
        };
        if(val >= 0)
            return genpos(0, val);
        else
            return genneg(0, val);
    });

    for(auto itr = stream.iterator(); itr.hasNext(); ) {
        std::cout << "stream:" << itr.next() << std::endl;
    }
}

TEST(UtilTestCase, StreamShrink) {
    int val = -50; // 0, -25, ... -49
    // recursive
    auto stream = Stream<Shrinkable<int>>(make_shrinkable<int>(0), [val]() {
        static std::function<Stream<Shrinkable<int>>(int,int)> genpos = [](int min, int val) {
            if(val <= 0 || (val-min) <= 1)
                return Stream<Shrinkable<int>>::empty();
            else
                return Stream<Shrinkable<int>>(make_shrinkable<int>((val + min)/2), [min, val]() { return genpos((val+min)/2, val); });
        };
        static std::function<Stream<Shrinkable<int>>(int,int)> genneg = [](int max, int val) {
            if(val >= 0 || (max-val) <= 1)
                return Stream<Shrinkable<int>>::empty();
            else
                return Stream<Shrinkable<int>>(make_shrinkable<int>((val + max)/2), [max, val]() { return genneg((val+max)/2, val); });
        };
        if(val >= 0)
            return genpos(0, val);
        else
            return genneg(0, val);
    });

    for(auto itr = stream.iterator(); itr.hasNext(); ) {
        std::cout << "streamshrink:" << itr.next() << std::endl;
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
        std::cout << "stream:" << itr.next() << std::endl;
    }

    auto shrinkable = make_shrinkable<int>(5).with([=]() {
        return stream;
    });

    auto stream2 = shrinkable.shrinks();
    for(auto itr = stream.iterator(); itr.hasNext(); ) {
        std::cout << "stream2:" << itr.next() << std::endl;
    }

}

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


TEST(UtilTestCase, Random) {
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

