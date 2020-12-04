#pragma once
#include "../gen.hpp"
#include "../util/utf16string.hpp"
#include "../util/std.hpp"

namespace proptest {

template <>
class PROPTEST_API Arbi<UTF16BEString> final : public ArbiContainer<UTF16BEString> {
public:
    using ArbiContainer<UTF16BEString>::minSize;
    using ArbiContainer<UTF16BEString>::maxSize;
    static size_t defaultMinSize;
    static size_t defaultMaxSize;

    Arbi();
    Arbi(Arbi<uint32_t>& _elemGen);
    Arbi(GenFunction<uint32_t> _elemGen);

    Shrinkable<UTF16BEString> operator()(Random& rand) override;

    GenFunction<uint32_t> elemGen;
};

template <>
class PROPTEST_API Arbi<UTF16LEString> final : public ArbiContainer<UTF16LEString> {
public:
    using ArbiContainer<UTF16LEString>::minSize;
    using ArbiContainer<UTF16LEString>::maxSize;
    static size_t defaultMinSize;
    static size_t defaultMaxSize;

    Arbi();
    Arbi(Arbi<uint32_t>& _elemGen);
    Arbi(GenFunction<uint32_t> _elemGen);

    Shrinkable<UTF16LEString> operator()(Random& rand) override;
    GenFunction<uint32_t> elemGen;
};

}  // namespace proptest
