#pragma once
#include "../gen.hpp"
#include "../util/utf16string.hpp"
#include <string>

namespace PropertyBasedTesting {

template <>
class PROPTEST_API Arbitrary<UTF16BEString> final : public ArbitraryContainer<UTF16BEString> {
public:
    using ArbitraryContainer<UTF16BEString>::minSize;
    using ArbitraryContainer<UTF16BEString>::maxSize;
    static size_t defaultMinSize;
    static size_t defaultMaxSize;

    Arbitrary() : ArbitraryContainer<UTF16BEString>(defaultMinSize, defaultMaxSize) {}

    Shrinkable<UTF16BEString> operator()(Random& rand) override;
};

template <>
class PROPTEST_API Arbitrary<UTF16LEString> final : public ArbitraryContainer<UTF16LEString> {
public:
    using ArbitraryContainer<UTF16LEString>::minSize;
    using ArbitraryContainer<UTF16LEString>::maxSize;
    static size_t defaultMinSize;
    static size_t defaultMaxSize;

    Arbitrary() : ArbitraryContainer<UTF16LEString>(defaultMinSize, defaultMaxSize) {}

    Shrinkable<UTF16LEString> operator()(Random& rand) override;
};

}  // namespace PropertyBasedTesting
