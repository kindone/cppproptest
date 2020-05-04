#pragma once
#include "api.hpp"
#include <iostream>
#include <ios>
#include <iomanip>
#include <vector>

namespace PropertyBasedTesting {

namespace util {

class IosFlagSaver {
public:
    explicit IosFlagSaver(std::ostream& _ios);

    ~IosFlagSaver();

    IosFlagSaver(const IosFlagSaver& rhs) = delete;
    IosFlagSaver& operator=(const IosFlagSaver& rhs) = delete;

private:
    std::ostream& ios;
    std::ios::fmtflags f;
};

}  // namespace util

class PROPTEST_API UTF8String : public std::string {
public:
    explicit UTF8String(std::string&& other) : std::string(other) {}
    explicit UTF8String(std::string& other) : std::string(other) {}
    using std::string::string;
};

std::ostream& validChar(std::ostream& os, uint8_t c);
std::ostream& validChar(std::ostream& os, uint8_t c1, uint8_t c2);
std::ostream& validChar(std::ostream& os, uint8_t c1, uint8_t c2, uint8_t c3);
std::ostream& validChar(std::ostream& os, uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4);
std::ostream& charAsHex(std::ostream& os, uint8_t c);
std::ostream& UTF8ToHex(std::ostream& os, std::vector<uint8_t>& chars);
PROPTEST_API std::ostream& decodeUTF8(std::ostream& os, std::vector<uint8_t>& chars);
PROPTEST_API std::ostream& decodeUTF8(std::ostream& os, const std::string& str);
PROPTEST_API std::ostream& decodeUTF8(std::ostream& os, const UTF8String& str);
PROPTEST_API bool isValidUTF8(std::vector<uint8_t>& chars);

}  // namespace PropertyBasedTesting