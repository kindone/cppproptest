#ifndef __PROPTEST_STRING_HPP__
#define __PROPTEST_STRING_HPP__

#include "testing/gen.hpp"
#include <string>


namespace PropertyBasedTesting
{

template <>
class PROPTEST_API Arbitrary<std::string> : public Gen<std::string>
{
public:
    Arbitrary(int _maxLen = 300) : maxLen(_maxLen) {
    }

    Shrinkable<std::string> operator()(Random& rand);
    static std::string boundaryValues[1];

    int maxLen;
};

class PROPTEST_API UTF8String : public std::string
{
public:
    explicit UTF8String(std::string&& other) : std::string(other) {}
    explicit UTF8String(std::string& other) : std::string(other) {}
    using std::string::string;
};

template <>
class PROPTEST_API Arbitrary<UTF8String> : public Gen<UTF8String>
{
public:
    Arbitrary(int _maxLen = 300) : maxLen(_maxLen) {
    }

    Shrinkable<UTF8String> operator()(Random& rand);
    static std::string boundaryValues[1];

    int maxLen;
};
} // namespace PropertyBasedTesting

#endif
