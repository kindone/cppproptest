#include "string.hpp"
#include "../generator/util.hpp"

namespace proptest {

Shrinkable<std::string> shrinkString(const std::string& str, size_t minSize) {
    size_t size = str.size();
    auto shrinkRear =
        util::binarySearchShrinkableU(size - minSize).map<std::string>([str, minSize](const uint64_t& size) {
            return str.substr(0, size + minSize);
        });

    // shrink front
    return shrinkRear.concat([minSize](const Shrinkable<std::string>& shr) {
        auto& str = shr.getRef();
        size_t maxSizeCopy = str.size();
        if (maxSizeCopy == minSize)
            return Stream<Shrinkable<std::string>>::empty();
        auto newShrinkable = util::binarySearchShrinkableU(maxSizeCopy - minSize)
                                 .map<std::string>([str, minSize = minSize, maxSizeCopy](const uint64_t& value) {
                                     return str.substr(minSize + value, maxSizeCopy - (minSize + value));
                                 });
        return newShrinkable.shrinks();
    });
}

}
