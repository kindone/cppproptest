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

    Arbitrary();
    Arbitrary(Arbitrary<uint32_t>& _elemGen);
    Arbitrary(std::function<Shrinkable<uint32_t>(Random&)> _elemGen);

    Shrinkable<UTF16BEString> operator()(Random& rand) override;

    std::function<Shrinkable<uint32_t>(Random&)> elemGen;
};

template <>
class PROPTEST_API Arbitrary<UTF16LEString> final : public ArbitraryContainer<UTF16LEString> {
public:
    using ArbitraryContainer<UTF16LEString>::minSize;
    using ArbitraryContainer<UTF16LEString>::maxSize;
    static size_t defaultMinSize;
    static size_t defaultMaxSize;

    Arbitrary();
    Arbitrary(Arbitrary<uint32_t>& _elemGen);
    Arbitrary(std::function<Shrinkable<uint32_t>(Random&)> _elemGen);

    Shrinkable<UTF16LEString> operator()(Random& rand) override;
    std::function<Shrinkable<uint32_t>(Random&)> elemGen;
};

}  // namespace PropertyBasedTesting
