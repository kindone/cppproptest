#pragma once
#include "../api.hpp"
#include "misc.hpp"
#include "std.hpp"

namespace proptest {

class PROPTEST_API UTF16BEString : public string {
public:
    explicit UTF16BEString(string&& other) : string(other) {}
    explicit UTF16BEString(string& other) : string(other) {}
    using string::string;

    size_t charsize() const;
};

class PROPTEST_API UTF16LEString : public string {
public:
    explicit UTF16LEString(string&& other) : string(other) {}
    explicit UTF16LEString(string& other) : string(other) {}
    using string::string;

    size_t charsize() const;
};

namespace util {
ostream& validUTF16Char(ostream& os, uint8_t c);
ostream& validUTF16Char(ostream& os, uint8_t c1, uint8_t c2);
ostream& validUTF16Char(ostream& os, uint8_t c1, uint8_t c2, uint8_t c3);
ostream& validUTF16Char(ostream& os, uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4);

PROPTEST_API ostream& decodeUTF16BE(ostream& os, vector<uint8_t>& chars);
PROPTEST_API ostream& decodeUTF16BE(ostream& os, const string& str);
PROPTEST_API ostream& decodeUTF16BE(ostream& os, const UTF16BEString& str);

struct PROPTEST_API DecodeUTF16BE
{
    DecodeUTF16BE(const string& _str) : str(_str) {}
    DecodeUTF16BE(const UTF16BEString& _str) : str(_str) {}
    friend ostream& operator<<(ostream& os, const DecodeUTF16BE& obj) { return decodeUTF16BE(os, obj.str); }

    const string& str;
};

PROPTEST_API bool isValidUTF16BE(vector<uint8_t>& chars);
PROPTEST_API bool isValidUTF16BE(vector<uint8_t>& chars, int& numChars);
PROPTEST_API int UTF16BECharSize(const string& str);

PROPTEST_API ostream& decodeUTF16LE(ostream& os, vector<uint8_t>& chars);
PROPTEST_API ostream& decodeUTF16LE(ostream& os, const string& str);
PROPTEST_API ostream& decodeUTF16LE(ostream& os, const UTF16LEString& str);

struct PROPTEST_API DecodeUTF16LE
{
    DecodeUTF16LE(const string& _str) : str(_str) {}
    DecodeUTF16LE(const UTF16LEString& _str) : str(_str) {}
    friend ostream& operator<<(ostream& os, const DecodeUTF16LE& obj) { return decodeUTF16LE(os, obj.str); }

    const string& str;
};

PROPTEST_API bool isValidUTF16LE(vector<uint8_t>& chars);
PROPTEST_API bool isValidUTF16LE(vector<uint8_t>& chars, int& numChars);
PROPTEST_API int UTF16LECharSize(const string& str);
}  // namespace util

}  // namespace proptest
