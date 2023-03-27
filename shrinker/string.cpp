#include "string.hpp"
#include "../generator/util.hpp"

namespace proptest {

Shrinkable<string> shrinkString(const string& str, size_t minSize) {
    size_t size = str.size();
    auto shrinkRear =
        util::binarySearchShrinkableU(size - minSize).map<string>([str, minSize](const uint64_t& size) {
            return str.substr(0, size + minSize);
        });

    // shrink front
    return shrinkRear.concat([minSize](const Shrinkable<string>& shr) {
        auto& str = shr.getRef();
        size_t maxSizeCopy = str.size();
        if (maxSizeCopy == minSize)
            return Stream::empty();
        auto newShrinkable = util::binarySearchShrinkableU(maxSizeCopy - minSize)
                                 .map<string>([str, minSize = minSize, maxSizeCopy](const uint64_t& value) {
                                     return str.substr(minSize + value, maxSizeCopy - (minSize + value));
                                 });
        return newShrinkable.shrinks();
    });
}

}
