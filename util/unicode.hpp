#pragma once

#include "../api.hpp"
#include <iostream>
#include <vector>
#include <string>

namespace proptest {
namespace util {

PROPTEST_API std::ostream& codepage(std::ostream& os, uint32_t code);
std::ostream& charAsHex(std::ostream& os, uint8_t c);
std::ostream& charAsHex(std::ostream& os, uint8_t c1, uint8_t c2);
std::ostream& charAsHex(std::ostream& os, uint8_t c1, uint8_t c2, uint8_t c3);
std::ostream& charAsHex(std::ostream& os, uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4);
PROPTEST_API std::ostream& charAsHex(std::ostream& os, std::vector<uint8_t>& chars);
PROPTEST_API std::ostream& charAsHex(std::ostream& os, const std::string& str);

struct PROPTEST_API StringAsHex
{
    StringAsHex(const std::string& str) : str(str) {}

    friend std::ostream& operator<<(std::ostream& os, const StringAsHex& obj) { return charAsHex(os, obj.str); }

    const std::string& str;
};

std::ostream& validChar(std::ostream& os, uint8_t c);
std::ostream& validChar2(std::ostream& os, uint8_t c);
// std::ostream& validChar(std::ostream& os, uint8_t c1, uint8_t c2);
// std::ostream& validChar(std::ostream& os, uint8_t c1, uint8_t c2, uint8_t c3);
// std::ostream& validChar(std::ostream& os, uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4);

}  // namespace util
}  // namespace proptest
