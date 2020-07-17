#pragma once

namespace proptest {

struct MinMax
{
    MinMax(int min, int max) : min(min), max(max) {}
    uint64_t size() const { return static_cast<uint64_t>(max) - static_cast<uint64_t>(min) + 1; }

    int min;
    int max;
};

decltype(auto) ranges(MinMax&&... minmax)
{
    uint64_t sum;

    return 0;
}

}  // namespace proptest
