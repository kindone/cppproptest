#pragma once

#include "../Shrinkable.hpp"
#include "../util/std.hpp"
#include "../generator/util.hpp"

namespace proptest {

template <typename Key, typename T>
Shrinkable<map<Key, T>> shrinkMap(const shared_ptr<map<Shrinkable<Key>, Shrinkable<T>>>& shrinkableMap, size_t minSize) {
    size_t size = shrinkableMap->size();
    // shrink map size with submap using binary numeric shrink of sizes
    size_t minSizeCopy = minSize;
    auto rangeShrinkable =
        util::binarySearchShrinkable(size - minSizeCopy).template map<size_t>([minSizeCopy](const uint64_t& _size) {
            return _size + minSizeCopy;
        });

    // this make sure shrinking is possible towards minSize
    Shrinkable<map<Shrinkable<Key>, Shrinkable<T>>> shrinkable =
        rangeShrinkable.template flatMap<map<Shrinkable<Key>, Shrinkable<T>>>([shrinkableMap](const size_t& _size) {
            if (_size == 0)
                return make_shrinkable<map<Shrinkable<Key>, Shrinkable<T>>>();  // empty map

            size_t i = 0;
            auto begin = shrinkableMap->begin();
            auto last = shrinkableMap->begin();
            for (; last != shrinkableMap->end() && i < _size; ++last, ++i) {}
            return make_shrinkable<map<Shrinkable<Key>, Shrinkable<T>>>(begin, last);
        });

    return shrinkable.template flatMap<map<Key, T>>(+[](const map<Shrinkable<Key>, Shrinkable<T>>& shrMap) {
        auto value = make_shrinkable<map<Key, T>>();
        map<Key, T>& valueMap = value.getRef();

        for (auto itr = shrMap.begin(); itr != shrMap.end(); ++itr) {
            auto& shrPair = *itr;
            valueMap.insert(pair<Key, T>(shrPair.first.getRef(), shrPair.second.getRef()));
        }

        return value;
    });

}

} // namespace proptes
