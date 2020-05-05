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
};

namespace util {

std::ostream& validChar(std::ostream& os, uint8_t c);
std::ostream& validChar(std::ostream& os, uint8_t c1, uint8_t c2);
std::ostream& validChar(std::ostream& os, uint8_t c1, uint8_t c2, uint8_t c3);
std::ostream& validChar(std::ostream& os, uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4);
std::ostream& charAsHex(std::ostream& os, uint8_t c);
PROPTEST_API std::ostream& UTF8ToHex(std::ostream& os, std::vector<uint8_t>& chars);
PROPTEST_API std::ostream& decodeUTF8(std::ostream& os, std::vector<uint8_t>& chars);
PROPTEST_API std::ostream& decodeUTF8(std::ostream& os, const std::string& str);
PROPTEST_API std::ostream& decodeUTF8(std::ostream& os, const UTF8String& str);
PROPTEST_API bool isValidUTF8(std::vector<uint8_t>& chars);

}  // namespace util

}  // namespace PropertyBasedTesting