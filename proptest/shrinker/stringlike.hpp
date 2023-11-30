#pragma once
#include "../Shrinkable.hpp"
#include "../generator/util.hpp"

namespace proptest {

template <typename StringLike>
Shrinkable<StringLike> shrinkStringLike(const StringLike& str, size_t minSize, size_t size, const vector<int>& bytePositions) {
    auto shrinkRear =
        util::binarySearchShrinkable(size - minSize)
            .template map<StringLike>([str, minSize, bytePositions](const uint64_t& _size) -> StringLike {
                if (bytePositions.empty())
                    return StringLike();
                else
                    return StringLike(str.substr(0, bytePositions[_size + minSize]));
            });

    return shrinkRear.concat([minSize, bytePositions](const Shrinkable<StringLike>& shr) {
        auto& str = shr.getRef();
        size_t maxSizeCopy = str.charsize();
        if (maxSizeCopy == minSize)
            return Stream::empty();
        auto newShrinkable =
            util::binarySearchShrinkableU(maxSizeCopy - minSize)
                .map<StringLike>([str, minSize, maxSizeCopy, bytePositions](const uint64_t& value) {
                    if (bytePositions.empty())
                        return StringLike();
                    else
                        return StringLike(str.substr(bytePositions[minSize + value],
                                                      bytePositions[maxSizeCopy] - bytePositions[minSize + value]));
                });
        return newShrinkable.shrinks();
    });
}

}
