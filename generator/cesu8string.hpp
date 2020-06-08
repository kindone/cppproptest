#pragma once
#include "../gen.hpp"
#include "../util/cesu8string.hpp"
#include <string>

namespace PropertyBasedTesting {

template <>
class PROPTEST_API Arbitrary<CESU8String> final : public ArbitraryContainer<CESU8String> {
public:
    using ArbitraryContainer<CESU8String>::minSize;
    using ArbitraryContainer<CESU8String>::maxSize;
    static size_t defaultMinSize;
    static size_t defaultMaxSize;

    Arbitrary() : ArbitraryContainer<CESU8String>(defaultMinSize, defaultMaxSize) {}

    Shrinkable<CESU8String> operator()(Random& rand) override;
};

}  // namespace PropertyBasedTesting
