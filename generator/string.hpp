#pragma once
#include "../gen.hpp"
#include "../util/utf8string.hpp"
#include "../util/utf16string.hpp"
#include "../util/cesu8string.hpp"
#include <string>

namespace PropertyBasedTesting {

template <>
class PROPTEST_API Arbitrary<std::string> final : public Gen<std::string> {
public:
    static size_t defaultMinSize;
    static size_t defaultMaxSize;

    Arbitrary();
    Arbitrary(Arbitrary<char>& _elemGen);
    Arbitrary(std::function<Shrinkable<char>(Random&)> _elemGen);

    Shrinkable<std::string> operator()(Random& rand) override;

    Arbitrary setMinSize(size_t size);
    Arbitrary setMaxSize(size_t size);
    Arbitrary setSize(size_t size);

    // FIXME: turn to shared_ptr
    std::function<Shrinkable<char>(Random&)> elemGen;
    size_t minSize;
    size_t maxSize;
};

template <>
class PROPTEST_API Arbitrary<UTF8String> final : public Gen<UTF8String> {
public:
    static size_t defaultMinSize;
    static size_t defaultMaxSize;

    Arbitrary() : minSize(defaultMinSize), maxSize(defaultMaxSize) {}

    Shrinkable<UTF8String> operator()(Random& rand) override;

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

template <>
class PROPTEST_API Arbitrary<CESU8String> final : public Gen<CESU8String> {
public:
    static size_t defaultMinSize;
    static size_t defaultMaxSize;

    Arbitrary() : minSize(defaultMinSize), maxSize(defaultMaxSize) {}

    Shrinkable<CESU8String> operator()(Random& rand) override;

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
