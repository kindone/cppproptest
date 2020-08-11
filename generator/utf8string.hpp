#pragma once
#include "../gen.hpp"
#include "../util/utf8string.hpp"
#include <string>

namespace proptest {

template <>
class PROPTEST_API Arbi<UTF8String> final : public ArbiContainer<UTF8String> {
public:
    using ArbiContainer<UTF8String>::minSize;
    using ArbiContainer<UTF8String>::maxSize;
    static size_t defaultMinSize;
    static size_t defaultMaxSize;

    Arbi();
    Arbi(Arbi<uint32_t>& _elemGen);
    Arbi(GenFunction<uint32_t> _elemGen);

    Shrinkable<UTF8String> operator()(Random& rand) override;

    GenFunction<uint32_t> elemGen;
};

}  // namespace proptest
