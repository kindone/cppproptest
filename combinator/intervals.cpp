#include "intervals.hpp"

namespace proptest {

CustomGen<int64_t> intervals(std::initializer_list<Interval> intervals)
{
    using WeightedVec = std::vector<util::Weighted<int64_t>>;

    uint64_t sum = 0;
    for (auto interval : intervals) {
        if (interval.size() == 0)
            throw std::runtime_error("invalid empty interval: [" + std::to_string(interval.min) + ", " +
                                     std::to_string(interval.max) + "]");
        sum += interval.size();
    }

    WeightedVec* genVec = new WeightedVec();
    for (auto interval : intervals) {
        genVec->push_back(
            weighted<int64_t>(fromTo<int64_t>(interval.min, interval.max), static_cast<double>(interval.size()) / sum));
    }

    std::shared_ptr<WeightedVec> genVecPtr(genVec);

    return util::oneOfHelper<int64_t>(genVecPtr);
}

CustomGen<uint64_t> uintervals(std::initializer_list<UInterval> intervals)
{
    using WeightedVec = std::vector<util::Weighted<uint64_t>>;

    uint64_t sum = 0;

    for (auto interval : intervals) {
        if (interval.size() == 0)
            throw std::runtime_error("invalid empty interval: [" + std::to_string(interval.min) + ", " +
                                     std::to_string(interval.max) + "]");
        sum += interval.size();
    }

    WeightedVec* genVec = new WeightedVec();
    for (auto interval : intervals) {
        genVec->push_back(weighted<uint64_t>(fromTo<uint64_t>(interval.min, interval.max),
                                             static_cast<double>(interval.size()) / sum));
    }

    std::shared_ptr<WeightedVec> genVecPtr(genVec);

    return util::oneOfHelper<uint64_t>(genVecPtr);
}

}  // namespace proptest
