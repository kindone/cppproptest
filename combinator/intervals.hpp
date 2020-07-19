#pragma once

#include "oneof.hpp"
#include "../generator/integral.hpp"
#include "../api.hpp"
#include <initializer_list>

namespace proptest {

struct Interval
{
    Interval(int64_t min, int64_t max) : min(min), max(max) {}
    int64_t size() const { return max - min + 1; }

    int64_t min;
    int64_t max;
};

struct UInterval
{
    UInterval(uint64_t min, uint64_t max) : min(min), max(max) {}
    uint64_t size() const { return max - min + 1; }

    uint64_t min;
    uint64_t max;
};

PROPTEST_API CustomGen<int64_t> intervals(std::initializer_list<Interval> interval_list);
PROPTEST_API CustomGen<uint64_t> uintervals(std::initializer_list<UInterval> interval_list);

}  // namespace proptest
