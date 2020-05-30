#pragma once
#include "../api.hpp"
#include "misc.hpp"
#include <iostream>
#include <ios>
#include <iomanip>
#include <vector>
#include <string>

namespace PropertyBasedTesting {

class PROPTEST_API UTF16BEString : public std::string {
public:
    explicit UTF16BEString(std::string&& other) : std::string(other) {}
    explicit UTF16BEString(std::string& other) : std::string(other) {}
    using std::string::string;

    size_t charsize() const;
};

class PROPTEST_API UTF16LEString : public std::string {
public:
    explicit UTF16LEString(std::string&& other) : std::string(other) {}
    explicit UTF16LEString(std::string& other) : std::string(other) {}
    using std::string::string;

    size_t charsize() const;
};

namespace util {
std::ostream& validUTF16Char(std::ostream& os, uint8_t c);
std::ostream& validUTF16Char(std::ostream& os, uint8_t c1, uint8_t c2);
std::ostream& validUTF16Char(std::ostream& os, uint8_t c1, uint8_t c2, uint8_t c3);
std::ostream& validUTF16Char(std::ostream& os, uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4);

PROPTEST_API std::ostream& decodeUTF16BE(std::ostream& os, std::vector<uint8_t>& chars);
PROPTEST_API std::ostream& decodeUTF16BE(std::ostream& os, const std::string& str);
PROPTEST_API std::ostream& decodeUTF16BE(std::ostream& os, const UTF16BEString& str);
PROPTEST_API bool isValidUTF16BE(std::vector<uint8_t>& chars);
PROPTEST_API bool isValidUTF16BE(std::vector<uint8_t>& chars, int& numChars);
PROPTEST_API int UTF16BECharSize(const std::string& str);

PROPTEST_API std::ostream& decodeUTF16LE(std::ostream& os, std::vector<uint8_t>& chars);
PROPTEST_API std::ostream& decodeUTF16LE(std::ostream& os, const std::string& str);
PROPTEST_API std::ostream& decodeUTF16LE(std::ostream& os, const UTF16LEString& str);
PROPTEST_API bool isValidUTF16LE(std::vector<uint8_t>& chars);
PROPTEST_API bool isValidUTF16LE(std::vector<uint8_t>& chars, int& numChars);
PROPTEST_API int UTF16LECharSize(const std::string& str);
}  // namespace util

}  // namespace PropertyBasedTesting
