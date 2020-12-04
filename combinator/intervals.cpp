#include "intervals.hpp"

namespace proptest {

Generator<int64_t> intervals(initializer_list<Interval> intervals)
{
    using WeightedVec = vector<util::Weighted<int64_t>>;

    uint64_t sum = 0;
    for (auto interval : intervals) {
        if (interval.size() == 0)
            throw runtime_error("invalid empty interval: [" + to_string(interval.min) + ", " +
                                     to_string(interval.max) + "]");
        sum += interval.size();
    }

    WeightedVec* genVec = new WeightedVec();
    for (auto interval : intervals) {
        genVec->push_back(weightedGen<int64_t>(proptest::interval<int64_t>(interval.min, interval.max),
                                               static_cast<double>(interval.size()) / sum));
    }

    shared_ptr<WeightedVec> genVecPtr(genVec);

    return util::oneOfHelper<int64_t>(genVecPtr);
}

Generator<uint64_t> uintervals(initializer_list<UInterval> intervals)
{
    using WeightedVec = vector<util::Weighted<uint64_t>>;

    uint64_t sum = 0;

    for (auto interval : intervals) {
        if (interval.size() == 0)
            throw runtime_error("invalid empty interval: [" + to_string(interval.min) + ", " +
                                     to_string(interval.max) + "]");
        sum += interval.size();
    }

    WeightedVec* genVec = new WeightedVec();
    for (auto interval : intervals) {
        genVec->push_back(weightedGen<uint64_t>(proptest::interval<uint64_t>(interval.min, interval.max),
                                                static_cast<double>(interval.size()) / sum));
    }

    shared_ptr<WeightedVec> genVecPtr(genVec);

    return util::oneOfHelper<uint64_t>(genVecPtr);
}

}  // namespace proptest
