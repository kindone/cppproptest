#pragma once
#include "../gen.hpp"
#include "../util/utf8string.hpp"
#include <string>

namespace pbt {

template <>
class PROPTEST_API Arbitrary<UTF8String> final : public ArbitraryContainer<UTF8String> {
public:
    using ArbitraryContainer<UTF8String>::minSize;
    using ArbitraryContainer<UTF8String>::maxSize;
    static size_t defaultMinSize;
    static size_t defaultMaxSize;

    Arbitrary();
    Arbitrary(Arbitrary<uint32_t>& _elemGen);
    Arbitrary(std::function<Shrinkable<uint32_t>(Random&)> _elemGen);

    Shrinkable<UTF8String> operator()(Random& rand) override;

    std::function<Shrinkable<uint32_t>(Random&)> elemGen;
};

}  // namespace pbt
