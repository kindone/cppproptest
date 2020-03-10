#include "testing/proptest.hpp"
#include "googletest/googletest/include/gtest/gtest.h"
#include "googletest/googlemock/include/gmock/gmock.h"
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
    int val = -50;
    // recursive
    auto stream = Stream<int>(val, [val]() {
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

TEST(UtilTestCase, Shrinkable) {
    // generates 50,49,...,0
    int val = 50;
    // recursive
    auto stream = Stream<Shrinkable<int>>(Shrinkable<int>(val), [val]() {
        static std::function<Stream<Shrinkable<int>>(int)> gen = [](int val) {
            if(val <= 0)
                return Stream<Shrinkable<int>>::empty();
            else
                return Stream<Shrinkable<int>>(Shrinkable<int>(val-1), [val]() { return gen(val-1); });
        };
        return gen(val);
    });

    for(auto itr = stream.iterator(); itr.hasNext(); ) {
        std::cout << "stream:" << itr.next() << std::endl;
    }

    auto shrinkable = Shrinkable<int>(5, [=]() {
        return stream;
    });

    auto stream2 = shrinkable.shrinks();
    for(auto itr = stream.iterator(); itr.hasNext(); ) {
        std::cout << "stream2:" << itr.next() << std::endl;
    }
   
}
