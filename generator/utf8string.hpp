#pragma once
#include "../gen.hpp"
#include "../util/utf8string.hpp"
#include <string>

namespace PropertyBasedTesting {

template <>
class PROPTEST_API Arbitrary<UTF8String> final : public ArbitraryContainer<UTF8String> {
public:
    using ArbitraryContainer<UTF8String>::minSize;
    using ArbitraryContainer<UTF8String>::maxSize;
    static size_t defaultMinSize;
    static size_t defaultMaxSize;

    Arbitrary() : ArbitraryContainer<UTF8String>(defaultMinSize, defaultMaxSize) {}

    Shrinkable<UTF8String> operator()(Random& rand) override;
};

}  // namespace PropertyBasedTesting
