#pragma once
#include "../Shrinkable.hpp"
#include <set>

namespace proptest {

template <typename T>
Shrinkable<std::set<T>> shrinkSet(const std::shared_ptr<std::set<Shrinkable<T>>>& shrinkableSet, size_t minSize) {
    // shrink set size with subset using binary numeric shrink of sizes
    size_t size = shrinkableSet->size();
    auto rangeShrinkable =
        util::binarySearchShrinkableU(size - minSize)
            .template map<uint64_t>([minSize](const uint64_t& _size) { return _size + minSize; });
    // this make sure shrinking is possible towards minSize
    Shrinkable<std::set<Shrinkable<T>>> shrinkable =
        rangeShrinkable.template flatMap<std::set<Shrinkable<T>>>([shrinkableSet](const size_t& _size) {
            if (_size == 0)
                return make_shrinkable<std::set<Shrinkable<T>>>();

            size_t i = 0;
            auto begin = shrinkableSet->begin();
            auto last = shrinkableSet->begin();
            for (; last != shrinkableSet->end() && i < _size; ++last, ++i) {}
            return make_shrinkable<std::set<Shrinkable<T>>>(begin, last);
        });

    return shrinkable.template flatMap<std::set<T>>(+[](const std::set<Shrinkable<T>>& shr) {
        auto value = make_shrinkable<std::set<T>>();
        std::set<T>& valueSet = value.getRef();

        for (auto itr = shr.begin(); itr != shr.end(); ++itr) {
            valueSet.insert(std::move(itr->getRef()));
        }

        return value;
    });
}

}
