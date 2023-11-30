#pragma once
#include "../gen.hpp"
#include "../util/utf16string.hpp"
#include "../util/std.hpp"

/**
 * @file utf16string.hpp
 * @brief Arbitrary for UTF16BEString and UTF16LEString
 */

namespace proptest {

/**
 * @ingroup Generators
 * @brief Arbitrary for UTF-16 big endian string with configurable code generator and min/max sizes
 */
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

/**
 * @ingroup Generators
 * @brief Arbitrary for UTF-16 little endian string with configurable code generator and min/max sizes
 */
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
