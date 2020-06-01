#pragma once
#include "../gen.hpp"
#include "../util/utf16string.hpp"
#include <string>

namespace PropertyBasedTesting {

template <>
class PROPTEST_API Arbitrary<UTF16BEString> final : public Gen<UTF16BEString> {
public:
    static size_t defaultMinSize;
    static size_t defaultMaxSize;

    Arbitrary() : minSize(defaultMinSize), maxSize(defaultMaxSize) {}

    Shrinkable<UTF16BEString> operator()(Random& rand) override;

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

template <>
class PROPTEST_API Arbitrary<UTF16LEString> final : public Gen<UTF16LEString> {
public:
    static size_t defaultMinSize;
    static size_t defaultMaxSize;

    Arbitrary() : minSize(defaultMinSize), maxSize(defaultMaxSize) {}

    Shrinkable<UTF16LEString> operator()(Random& rand) override;

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
