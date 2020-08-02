#pragma once
#include "../gen.hpp"
#include "../util/cesu8string.hpp"
#include <string>

namespace proptest {

template <>
class PROPTEST_API Arbitrary<CESU8String> final : public ArbitraryContainer<CESU8String> {
public:
    using ArbitraryContainer<CESU8String>::minSize;
    using ArbitraryContainer<CESU8String>::maxSize;
    static size_t defaultMinSize;
    static size_t defaultMaxSize;

    Arbitrary();
    Arbitrary(Arbitrary<uint32_t>& _elemGen);
    Arbitrary(GenFunction<uint32_t> _elemGen);

    Shrinkable<CESU8String> operator()(Random& rand) override;

    GenFunction<uint32_t> elemGen;
};

}  // namespace proptest
