#pragma once
#include "../gen.hpp"
#include "../util/cesu8string.hpp"
#include <string>

namespace PropertyBasedTesting {

template <>
class PROPTEST_API Arbitrary<CESU8String> final : public Gen<CESU8String> {
public:
    static size_t defaultMinSize;
    static size_t defaultMaxSize;

    Arbitrary() : minSize(defaultMinSize), maxSize(defaultMaxSize) {}

    Shrinkable<CESU8String> operator()(Random& rand) override;

    Arbitrary setMinSize(size_t size)
    {
        minSize = size;
        return *this;
    }

    Arbitrary setMaxSize(size_t size)
    {
        maxSize = size;
        return *this;
    }

    Arbitrary setSize(size_t size)
    {
        minSize = size;
        maxSize = size;
        return *this;
    }

    size_t minSize;
    size_t maxSize;
};

}  // namespace PropertyBasedTesting
