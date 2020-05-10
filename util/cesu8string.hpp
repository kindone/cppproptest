#pragma once
#include "../api.hpp"
#include "misc.hpp"
#include <iostream>
#include <ios>
#include <iomanip>
#include <vector>
#include <string>

namespace PropertyBasedTesting {

class PROPTEST_API CESU8String : public std::string {
public:
    explicit CESU8String(std::string&& other) : std::string(other) {}
    explicit CESU8String(std::string& other) : std::string(other) {}
    using std::string::string;
};

namespace util {
std::ostream& validCESU8Char(std::ostream& os, uint8_t c);
std::ostream& validCESU8Char(std::ostream& os, uint8_t c1, uint8_t c2);
std::ostream& validCESU8Char(std::ostream& os, uint8_t c1, uint8_t c2, uint8_t c3);
std::ostream& validCESU8Char(std::ostream& os, uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4);
std::ostream& cesu8AsHex(std::ostream& os, uint8_t c);
PROPTEST_API std::ostream& CESU8ToHex(std::ostream& os, std::vector<uint8_t>& chars);
PROPTEST_API std::ostream& decodeCESU8(std::ostream& os, std::vector<uint8_t>& chars);
PROPTEST_API std::ostream& decodeCESU8(std::ostream& os, const std::string& str);
PROPTEST_API std::ostream& decodeCESU8(std::ostream& os, const CESU8String& str);
PROPTEST_API bool isValidCESU8(std::vector<uint8_t>& chars);
}  // namespace util

}  // namespace PropertyBasedTesting
