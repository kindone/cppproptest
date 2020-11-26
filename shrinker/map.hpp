#pragma once

#include "../Shrinkable.hpp"
#include <map>

namespace proptest {

template <typename Key, typename T>
Shrinkable<std::map<Key, T>> shrinkMap(const std::shared_ptr<std::map<Shrinkable<Key>, Shrinkable<T>>>& shrinkableMap, size_t minSize) {
    size_t size = shrinkableMap->size();
    // shrink map size with submap using binary numeric shrink of sizes
    size_t minSizeCopy = minSize;
    auto rangeShrinkable =
        util::binarySearchShrinkable(size - minSizeCopy).template map<size_t>([minSizeCopy](const uint64_t& _size) {
            return _size + minSizeCopy;
        });

    // this make sure shrinking is possible towards minSize
    Shrinkable<std::map<Shrinkable<Key>, Shrinkable<T>>> shrinkable =
        rangeShrinkable.template flatMap<std::map<Shrinkable<Key>, Shrinkable<T>>>([shrinkableMap](const size_t& _size) {
            if (_size == 0)
                return make_shrinkable<std::map<Shrinkable<Key>, Shrinkable<T>>>();  // empty map

            size_t i = 0;
            auto begin = shrinkableMap->begin();
            auto last = shrinkableMap->begin();
            for (; last != shrinkableMap->end() && i < _size; ++last, ++i) {}
            return make_shrinkable<std::map<Shrinkable<Key>, Shrinkable<T>>>(begin, last);
        });

    return shrinkable.template flatMap<std::map<Key, T>>(+[](const std::map<Shrinkable<Key>, Shrinkable<T>>& shr) {
        auto value = make_shrinkable<std::map<Key, T>>();
        std::map<Key, T>& valueMap = value.getRef();

        for (auto itr = shr.begin(); itr != shr.end(); ++itr) {
            auto& pair = *itr;
            valueMap.insert(std::pair<Key, T>(pair.first.getRef(), pair.second.getRef()));
        }

        return value;
    });

}

} // namespace proptes
