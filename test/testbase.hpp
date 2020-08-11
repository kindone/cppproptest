#pragma once
#include "proptest.hpp"
#include "googletest/googletest/include/gtest/gtest.h"
#include "googletest/googlemock/include/gmock/gmock.h"
#include "Random.hpp"
#include "util/utf8string.hpp"
#include "util/utf16string.hpp"
#include "util/cesu8string.hpp"

#include <chrono>
#include <iostream>

#include <time.h>
#include <sys/time.h>

double getTime();

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

std::ostream& operator<<(std::ostream& os, const std::vector<int>& input);
std::ostream& operator<<(std::ostream& os, const std::vector<int8_t>& input);

template <typename ARG1, typename ARG2>
std::ostream& operator<<(std::ostream& os, const std::tuple<ARG1, ARG2>& tuple)
{
    os << proptest::Show<std::tuple<ARG1, ARG2>>(tuple);
    return os;
}

template <typename ARG1, typename ARG2, typename ARG3>
std::ostream& operator<<(std::ostream& os, const std::tuple<ARG1, ARG2, ARG3>& tuple)
{
    os << proptest::Show<std::tuple<ARG1, ARG2, ARG3>>(tuple);
    return os;
}

template <typename ARG1, typename ARG2>
std::ostream& operator<<(std::ostream& os, const std::pair<ARG1, ARG2>& pair)
{
    os << proptest::Show<std::pair<ARG1, ARG2>>(pair);
    return os;
}

struct TableData
{
    int num_rows;
    uint16_t num_elements;
    std::vector<std::pair<uint16_t, bool>> indexes;
};

struct Animal
{
    Animal(int f, std::string n, std::vector<int>& m)
        : numFeet(f), name(n /*, allocator()*/), measures(m /*, allocator()*/)
    {
    }
    int numFeet;
    std::string name;
    std::vector<int> measures;
};

struct Animal2
{
    Animal2(int f, std::string n) : numFeet(f), name(n /*, allocator()*/) {}
    int numFeet;
    std::string name;
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
class Arbi<Animal> : public Construct<Animal, int, std::string, std::vector<int>&> {
};

}  // namespace proptest

std::ostream& operator<<(std::ostream& os, const proptest::UTF8String&);
std::ostream& operator<<(std::ostream& os, const proptest::UTF16BEString&);
std::ostream& operator<<(std::ostream& os, const proptest::UTF16LEString&);
std::ostream& operator<<(std::ostream& os, const proptest::CESU8String&);
std::ostream& operator<<(std::ostream& os, const std::vector<Foo>& vec);
std::ostream& operator<<(std::ostream& os, const TableData& td);
std::ostream& operator<<(std::ostream& os, const std::vector<std::tuple<uint16_t, bool>>& indexVec);
std::ostream& operator<<(std::ostream& os, const Animal& input);
std::ostream& operator<<(std::ostream& os,
                         const std::pair<std::tuple<int, uint16_t>, std::vector<std::tuple<uint16_t, bool>>>& input);
std::ostream& operator<<(std::ostream& os, const std::set<int>& input);
std::ostream& operator<<(std::ostream& os, const std::map<int, int>& input);
std::ostream& operator<<(std::ostream& os, const std::list<int>& input);

template <typename T>
std::ostream& operator<<(std::ostream& os, const std::shared_ptr<T>& ptr)
{
    if (static_cast<bool>(ptr))
        os << *ptr;
    else
        os << "(null)";
    return os;
    // return proptest::show(os, ptr);
}

template <typename T>
std::ostream& operator<<(std::ostream& os, const proptest::Nullable<T>& nullable)
{
    os << nullable.ptr;
    return os;
    // return proptest::show(os, nullable);
}

template <typename T>
void exhaustive(const proptest::Shrinkable<T>& shrinkable, int level, bool print = true)
{
    // using namespace proptest;
    if (print) {
        for (int i = 0; i < level; i++)
            std::cout << "  ";

        std::cout << "shrinkable: " << proptest::Show<T>(shrinkable.get()) << std::endl;
    }

    auto shrinks = shrinkable.shrinks();
    for (auto itr = shrinks.iterator(); itr.hasNext();) {
        auto shrinkable2 = itr.next();
        exhaustive(shrinkable2, level + 1, print);
    }
}
