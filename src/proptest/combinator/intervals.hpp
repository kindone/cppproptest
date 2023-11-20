#pragma once

#include "proptest/combinator/oneof.hpp"
#include "proptest/generator/integral.hpp"
#include "proptest/api.hpp"
#include "proptest/util/std.hpp"

/**
 * @file intervals.hpp
 * @brief generator combinator for generating integers in multiple intervals
 */

namespace proptest {

struct Interval
{
    Interval(int64_t _min, int64_t _max) : min(_min), max(_max) {}
    int64_t size() const { return max - min + 1; }

    int64_t min;
    int64_t max;
};

struct UInterval
{
    UInterval(uint64_t _min, uint64_t _max) : min(_min), max(_max) {}
    uint64_t size() const { return max - min + 1; }

    uint64_t min;
    uint64_t max;
};

/**
 * @ingroup Combinators
 * @brief generates integer values in union of intervals
 * @details auto intGen = intervals({Interval(-100, 0), Interval(10, 100)}); // generates integers in [-100,0] or
 * [10,100]
 */
PROPTEST_API Generator<int64_t> intervals(initializer_list<Interval> interval_list);
/**
 * @brief generates unsigned integer values in union of intervals
 * @details auto intGen = uintervals({UInterval(0, 2), UInterval(5, 10)}); // generates integers in [0,2] or [5,10]
 */
PROPTEST_API Generator<uint64_t> uintervals(initializer_list<UInterval> interval_list);

}  // namespace proptest
