#pragma once
#include "../gen.hpp"
#include "../util/string.hpp"
#include <string>

namespace PropertyBasedTesting {

template <>
class PROPTEST_API Arbitrary<std::string> : public Gen<std::string> {
public:
    static size_t defaultMinSize;
    static size_t defaultMaxSize;

    Arbitrary();
    Arbitrary(Arbitrary<char>& _elemGen);
    Arbitrary(std::function<Shrinkable<char>(Random&)> _elemGen);

    Shrinkable<std::string> operator()(Random& rand);
    static std::string boundaryValues[1];

    Arbitrary setMinSize(int size);
    Arbitrary setMaxSize(int size);
    Arbitrary setSize(int size);

    // FIXME: turn to shared_ptr
    std::function<Shrinkable<char>(Random&)> elemGen;
    int minSize;
    int maxSize;
};

template <>
class PROPTEST_API Arbitrary<UTF8String> : public Gen<UTF8String> {
public:
    static size_t defaultMinSize;
    static size_t defaultMaxSize;

    Arbitrary() : minSize(defaultMinSize), maxSize(defaultMaxSize) {}

    Shrinkable<UTF8String> operator()(Random& rand);
    static std::string boundaryValues[1];

    Arbitrary setMinSize(int size)
    {
        minSize = size;
        return *this;
    }

    Arbitrary setMaxSize(int size)
    {
        maxSize = size;
        return *this;
    }

    Arbitrary setSize(int size)
    {
        minSize = size;
        maxSize = size;
        return *this;
    }

    int minSize;
    int maxSize;
};
}  // namespace PropertyBasedTesting
