#pragma once
#include "proptest/api.hpp"
#include "proptest/util/misc.hpp"
#include "proptest/util/std.hpp"

namespace proptest {

class PROPTEST_API UTF8String : public string {
public:
    explicit UTF8String(string&& other) : string(other) {}
    explicit UTF8String(string& other) : string(other) {}
    using string::string;

    size_t charsize() const;
};

namespace util {

PROPTEST_API ostream& UTF8ToHex(ostream& os, vector<uint8_t>& chars);
PROPTEST_API ostream& decodeUTF8(ostream& os, const string& str);
PROPTEST_API ostream& decodeUTF8(ostream& os, const UTF8String& str);

PROPTEST_API uint32_t decodeUTF8(vector<uint8_t>& chars);
void encodeUTF8(uint32_t code, vector<uint8_t>& chars);

struct PROPTEST_API DecodeUTF8
{
    DecodeUTF8(const string& _str) : str(_str) {}
    DecodeUTF8(const UTF8String& _str) : str(_str) {}

    friend ostream& operator<<(ostream& os, const DecodeUTF8& obj) { return decodeUTF8(os, obj.str); }
    string str;
};

PROPTEST_API ostream& decodeUTF8(ostream& os, vector<uint8_t>& chars);

PROPTEST_API bool isValidUTF8(vector<uint8_t>& chars);
PROPTEST_API bool isValidUTF8(vector<uint8_t>& chars, int& numChars);
PROPTEST_API int UTF8CharSize(const string& str);

}  // namespace util

}  // namespace proptest
