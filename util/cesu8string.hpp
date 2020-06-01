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

    size_t charsize() const;
};

namespace util {
PROPTEST_API std::ostream& CESU8ToHex(std::ostream& os, std::vector<uint8_t>& chars);
PROPTEST_API std::ostream& decodeCESU8(std::ostream& os, std::vector<uint8_t>& chars);
PROPTEST_API std::ostream& decodeCESU8(std::ostream& os, const std::string& str);
PROPTEST_API std::ostream& decodeCESU8(std::ostream& os, const CESU8String& str);

struct PROPTEST_API DecodeCESU8
{
    DecodeCESU8(const std::string& str) : str(str) {}
    DecodeCESU8(const CESU8String& str) : str(str) {}
    friend std::ostream& operator<<(std::ostream& os, const DecodeCESU8& obj) { return decodeCESU8(os, obj.str); }

    const std::string& str;
};

PROPTEST_API bool isValidCESU8(std::vector<uint8_t>& chars);
PROPTEST_API bool isValidCESU8(std::vector<uint8_t>& chars, int& numChars);
PROPTEST_API int CESU8CharSize(const std::string& str);
}  // namespace util

}  // namespace PropertyBasedTesting
