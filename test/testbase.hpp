#pragma once
#include "proptest.hpp"
#include "googletest/googletest/include/gtest/gtest.h"
#include "googletest/googlemock/include/gmock/gmock.h"
#include "Random.hpp"
#include "util/utf8string.hpp"
#include "util/utf16string.hpp"
#include "util/cesu8string.hpp"

#include "../util/std.hpp"
#include <chrono>

#include <time.h>
#include <sys/time.h>

double getTime();


using std::list;
using std::vector;
using std::set;
using std::map;
using std::pair;
using std::tuple;
using std::get;
using std::string;
using std::ostream;
using std::cout;
using std::endl;
using std::shared_ptr;
using std::bind;



class PropTestCase : public ::testing::Test {
};

template <typename T>
struct NumericTest : public testing::Test
{
    using NumericType = T;
};

template <typename T>
struct SignedNumericTest : public testing::Test
{
    using NumericType = T;
};

template <typename T>
struct IntegralTest : public testing::Test
{
    using NumericType = T;
};

using NumericTypes =
    testing::Types<int8_t, int16_t, int32_t, int64_t, uint8_t, uint16_t, uint32_t, uint64_t, float, double>;
using SignedNumericTypes = testing::Types<int8_t, int16_t, int32_t, int64_t, float, double>;
using IntegralTypes = testing::Types<int8_t, int16_t, int32_t, int64_t, uint8_t, uint16_t, uint32_t, uint64_t>;
using FloatingTypes = testing::Types<float, double>;

TYPED_TEST_CASE(NumericTest, NumericTypes);
TYPED_TEST_CASE(SignedNumericTest, SignedNumericTypes);
TYPED_TEST_CASE(IntegralTest, IntegralTypes);
TYPED_TEST_CASE(FloatingTest, FloatingTypes);


ostream& operator<<(ostream& os, const vector<int>& input);
ostream& operator<<(ostream& os, const vector<int8_t>& input);

template <typename ARG1, typename ARG2>
ostream& operator<<(ostream& os, const tuple<ARG1, ARG2>& tup)
{
    os << proptest::Show<tuple<ARG1, ARG2>>(tup);
    return os;
}

template <typename ARG1, typename ARG2, typename ARG3>
ostream& operator<<(ostream& os, const tuple<ARG1, ARG2, ARG3>& tup)
{
    os << proptest::Show<tuple<ARG1, ARG2, ARG3>>(tup);
    return os;
}

template <typename ARG1, typename ARG2>
ostream& operator<<(ostream& os, const pair<ARG1, ARG2>& pr)
{
    os << proptest::Show<pair<ARG1, ARG2>>(pr);
    return os;
}

struct TableData
{
    int num_rows;
    uint16_t num_elements;
    vector<pair<uint16_t, bool>> indexes;
};

struct Animal
{
    Animal(int f, string n, vector<int>& m)
        : numFeet(f), name(n /*, allocator()*/), measures(m /*, allocator()*/)
    {
    }
    int numFeet;
    string name;
    vector<int> measures;
};

struct Animal2
{
    Animal2(int f, string n) : numFeet(f), name(n /*, allocator()*/) {}
    int numFeet;
    string name;
};

struct Foo
{
    Foo(int a) : a(a) {}
    int a;
};

struct GenSmallInt : public proptest::GenBase<int32_t>
{
    GenSmallInt() : step(0ULL) {}
    proptest::Shrinkable<int32_t> operator()(proptest::Random&)
    {
        constexpr size_t num = sizeof(boundaryValues) / sizeof(boundaryValues[0]);
        return proptest::make_shrinkable<int32_t>(boundaryValues[step++ % num]);
    }

    size_t step;
    static constexpr int32_t boundaryValues[13] = {
        INT32_MIN, 0,         INT32_MAX,     -1,           1, -2, 2, INT32_MIN + 1, INT32_MAX - 1,
        INT16_MIN, INT16_MAX, INT16_MIN + 1, INT16_MAX - 1};
};

namespace proptest {

// define Arbi of Animal using Construct
template <>
class Arbi<Animal> : public Construct<Animal, int, string, vector<int>&> {
};

}  // namespace proptest

ostream& operator<<(ostream& os, const proptest::UTF8String&);
ostream& operator<<(ostream& os, const proptest::UTF16BEString&);
ostream& operator<<(ostream& os, const proptest::UTF16LEString&);
ostream& operator<<(ostream& os, const proptest::CESU8String&);
ostream& operator<<(ostream& os, const vector<Foo>& vec);
ostream& operator<<(ostream& os, const TableData& td);
ostream& operator<<(ostream& os, const vector<tuple<uint16_t, bool>>& indexVec);
ostream& operator<<(ostream& os, const Animal& input);
ostream& operator<<(ostream& os,
                         const pair<tuple<int, uint16_t>, vector<tuple<uint16_t, bool>>>& input);
ostream& operator<<(ostream& os, const set<int>& input);
ostream& operator<<(ostream& os, const map<int, int>& input);
ostream& operator<<(ostream& os, const list<int>& input);

template <typename T>
ostream& operator<<(ostream& os, const shared_ptr<T>& ptr)
{
    if (static_cast<bool>(ptr))
        os << *ptr;
    else
        os << "(null)";
    return os;
    // return proptest::show(os, ptr);
}

template <typename T>
ostream& operator<<(ostream& os, const proptest::Nullable<T>& nullable)
{
    os << nullable.ptr;
    return os;
    // return proptest::show(os, nullable);
}

template <typename T>
void printShrinkable(const proptest::Shrinkable<T>& shrinkable, int level) {
    for (int i = 0; i < level; i++)
        cout << "  ";

    cout << "shrinkable: " << proptest::Show<T>(shrinkable.get()) << endl;
}

template <typename T>
void exhaustive(const proptest::Shrinkable<T>& shrinkable, int level)
{
    // using namespace proptest;
    printShrinkable(shrinkable, level);

    auto shrinks = shrinkable.shrinks();
    for (auto itr = shrinks.iterator(); itr.hasNext();) {
        auto shrinkable2 = itr.next();
        exhaustive(shrinkable2, level + 1);
    }
}

template <typename T>
void exhaustive(const proptest::Shrinkable<T>& shrinkable, int level, proptest::function<void(const proptest::Shrinkable<T>&, int)> func)
{
    // using namespace proptest;
    func(shrinkable, level);

    auto shrinks = shrinkable.shrinks();
    for (auto itr = shrinks.iterator(); itr.hasNext();) {
        auto shrinkable2 = itr.next();
        exhaustive(shrinkable2, level + 1, func);
    }
}
