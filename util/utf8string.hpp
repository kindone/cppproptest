#pragma once
#include "../api.hpp"
#include "misc.hpp"
#include <iostream>
#include <ios>
#include <iomanip>
#include <vector>

namespace PropertyBasedTesting {

class PROPTEST_API UTF8String : public std::string {
public:
    explicit UTF8String(std::string&& other) : std::string(other) {}
    explicit UTF8String(std::string& other) : std::string(other) {}
    using std::string::string;

    size_t charsize() const;
};

namespace util {

PROPTEST_API std::ostream& UTF8ToHex(std::ostream& os, std::vector<uint8_t>& chars);
PROPTEST_API std::ostream& decodeUTF8(std::ostream& os, std::vector<uint8_t>& chars);
PROPTEST_API std::ostream& decodeUTF8(std::ostream& os, const std::string& str);
PROPTEST_API std::ostream& decodeUTF8(std::ostream& os, const UTF8String& str);
PROPTEST_API bool isValidUTF8(std::vector<uint8_t>& chars);
PROPTEST_API bool isValidUTF8(std::vector<uint8_t>& chars, int& numChars);
PROPTEST_API int UTF8CharSize(const std::string& str);

}  // namespace util

}  // namespace PropertyBasedTesting