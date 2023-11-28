#include "../api.hpp"
#include "utf16string.hpp"
#include "unicode.hpp"
#include "std.hpp"

namespace proptest {

size_t UTF16BEString::charsize() const
{
    int size = util::UTF16BECharSize(*this);
    if (size < 0) {
        stringstream str;
        str << "Not a valid UTF-16 BE string: ";
        for (size_t i = 0; i < this->size(); i++) {
            str << setfill('0') << setw(2) << util::hex << static_cast<int>((*this)[i]) << " ";
        }
        throw runtime_error(str.str());
    }

    return static_cast<size_t>(size);
}

size_t UTF16LEString::charsize() const
{
    int size = util::UTF16LECharSize(*this);
    if (size < 0) {
        stringstream str;
        str << "Not a valid UTF-16 LE string: ";
        for (size_t i = 0; i < this->size(); i++) {
            str << setfill('0') << setw(2) << util::hex << static_cast<int>((*this)[i]) << " ";
        }
        throw runtime_error(str.str());
    }

    return static_cast<size_t>(size);
}

namespace util {

ostream& decodeUTF16BE(ostream& os, const string& str)
{
    vector<uint8_t> chars;
    chars.reserve(str.size());
    for (size_t i = 0; i < str.size(); i++) {
        chars.push_back(str[i]);
    }
    return decodeUTF16BE(os, chars);
}

ostream& decodeUTF16BE(ostream& os, const UTF16BEString& str)
{
    vector<uint8_t> chars;
    chars.reserve(str.size());
    for (size_t i = 0; i < str.size(); i++) {
        chars.push_back(str[i]);
    }
    return decodeUTF16BE(os, chars);
}

ostream& decodeUTF16LE(ostream& os, const string& str)
{
    vector<uint8_t> chars;
    chars.reserve(str.size());
    for (size_t i = 0; i < str.size(); i++) {
        chars.push_back(str[i]);
    }
    return decodeUTF16LE(os, chars);
}

ostream& decodeUTF16LE(ostream& os, const UTF16LEString& str)
{
    vector<uint8_t> chars;
    chars.reserve(str.size());
    for (size_t i = 0; i < str.size(); i++) {
        chars.push_back(str[i]);
    }
    return decodeUTF16LE(os, chars);
}

/*
 * legal UTF-16 byte sequence
 *
 *  Code Points        1st       2s       3s       4s
 * U+0000..U+D7FF     (as code point in LE/BE)
 *[U+D800..U+DBFF]    (high surrogates)
 *[U+DC00..U+DFFF]    (low surrogates)
 * U+E000..U+FFFF     (as code point in LE/BE)
 * U+10000..: 4-byte surrogate pairs (U+D800..U+DBFF + U+DC00..U+DFFF)
 */
ostream& decodeUTF16BE(ostream& os, vector<uint8_t>& chars)
{
    for (size_t i = 0; i < chars.size(); i++) {
        if (i + 2 > chars.size()) {
            charAsHex(os, chars[i]);
        }
        // ASCII: U+0000..U+007F
        if (chars[i] == 0 && chars[i + 1] <= 0x7f) {
            validChar2(os, chars[i + 1]);
            i++;
        }
        // U+0000..U+D7FF or U+E000..U+FFFF
        else if (chars[i] <= 0xD7 || 0xE0 <= chars[i]) {
            codepage(os, (chars[i] << 8) + chars[i + 1]);
            i++;
        } else if (i + 4 > chars.size()) {
            charAsHex(os, chars[i], chars[i + 1], chars[i + 2], chars[i + 3]);
            break;
        }
        // U+10000.. use surrogate pairs
        // U+0000..U+D7FF or U+E000..U+FFFF
        else {
            uint16_t s0 = static_cast<uint16_t>(((chars[i] << 8) + chars[i+1]));
            uint16_t s1 = static_cast<uint16_t>(((chars[i+2] << 8) + chars[i+3]));
            uint32_t p0 = (s0 - 0xD800) << 10;
            uint32_t p1 = (s1 - 0xDC00);
            codepage(os, 0x10000 + p0 + p1);
            // uint16_t c0 = ((chars[i] - 0xD8) << 8) + chars[i + 1];
            // uint16_t c1 = ((chars[i + 2] - 0xDC) << 8) + chars[i + 3];
            // codepage(os, 0x10000 + (c0 << 16) + c1);
            i += 3;
        }
    }
    return os;
}

uint32_t decodeUTF16BE(vector<uint8_t>& chars)
{
    if (2 > chars.size()) {
        throw runtime_error("invalid UTF-16 BE sequence");
    }
    // ASCII: U+0000..U+007F
    if (chars[0] == 0 && chars[1] <= 0x7f) {
        return static_cast<uint32_t>(chars[1]);
    }
    // U+0000..U+D7FF or U+E000..U+FFFF
    else if (chars[0] <= 0xD7 || 0xE0 <= chars[0]) {
        return static_cast<uint32_t>((chars[0] << 8) + chars[1]);
    } else if (4 > chars.size()) {
        throw runtime_error("invalid UTF-16 BE sequence");
    }
    // U+10000.. use surrogate pairs
    // U+0000..U+D7FF or U+E000..U+FFFF
    else {
        uint16_t s0 = static_cast<uint16_t>((chars[0] << 8) + chars[1]);
        uint16_t s1 = static_cast<uint16_t>((chars[2] << 8) + chars[3]);
        uint32_t p0 = (s0 - 0xD800) << 10;
        uint32_t p1 = (s1 - 0xDC00);
        return static_cast<uint32_t>(0x10000 + p0 + p1);
    }
    throw runtime_error("invalid UTF-16 BE sequence");
}

void encodeUTF16BE(uint32_t code, vector<uint8_t>& chars)
{
    if (code <= 0xd7FF || (0xE000 <= code && code <= 0xFFFF)) {
        uint8_t c0 = static_cast<uint8_t>(code >> 8);
        uint8_t c1 = (code & 0xff);
        chars.push_back(c0);
        chars.push_back(c1);
    }
    // code page U+10000..U+10FFFF
    else {
        code -= 0x10000;
        uint16_t surrogates[2] = {static_cast<uint16_t>(0xD800 + (code >> 10)),
                                    static_cast<uint16_t>(0xDC00 + (code & 0x3FF))};
        uint8_t c0 = surrogates[0] >> 8;
        uint8_t c1 = surrogates[0] & 0xFF;
        uint8_t c2 = surrogates[1] >> 8;
        uint8_t c3 = surrogates[1] & 0xFF;
        if (!(0xD8 <= c0 && c0 <= 0xDB && 0xDC <= c2 && c2 <= 0xDF)) {
            stringstream os;
            os << "invalid surrogate pairs: ";
            os << setfill('0') << setw(2) << util::hex << static_cast<int>(c0) << " ";
            os << setfill('0') << setw(2) << util::hex << static_cast<int>(c1) << " ";
            os << setfill('0') << setw(2) << util::hex << static_cast<int>(c2) << " ";
            os << setfill('0') << setw(2) << util::hex << static_cast<int>(c3) << " ";
            throw runtime_error(os.str());
        }

        chars.push_back(c0);
        chars.push_back(c1);
        chars.push_back(c2);
        chars.push_back(c3);
    }
}

ostream& decodeUTF16LE(ostream& os, vector<uint8_t>& chars)
{
    for (size_t i = 0; i < chars.size(); i++) {
        if (i + 2 > chars.size()) {
            charAsHex(os, chars[i]);
        }
        // ASCII: U+0000..U+007F
        if (chars[i + 1] == 0 && chars[i] <= 0x7f) {
            validChar2(os, chars[i]);
            i++;
        }
        // U+0000..U+D7FF or U+E000..U+FFFF
        else if (chars[i + 1] <= 0xD7 || 0xE0 <= chars[i + 1]) {
            codepage(os, (chars[i + 1] << 8) + chars[i]);
            i++;
        } else if (i + 4 > chars.size()) {
            charAsHex(os, chars[i], chars[i + 1], chars[i + 2], chars[i + 3]);
            break;
        }
        // U+10000.. use surrogate pairs
        // U+0000..U+D7FF or U+E000..U+FFFF
        else {
            uint16_t s0 = static_cast<uint16_t>((chars[i+1] << 8) + chars[i]);
            uint16_t s1 = static_cast<uint16_t>((chars[i+3] << 8) + chars[i+2]);
            uint32_t p0 = (s0 - 0xD800) << 10;
            uint32_t p1 = (s1 - 0xDC00);
            codepage(os, 0x10000 + p0 + p1);
            i += 3;
        }
    }
    return os;
}

uint32_t decodeUTF16LE(vector<uint8_t>& chars)
{
    if (2 > chars.size()) {
        throw runtime_error("invalid UTF-16 LE sequence");
    }
    // ASCII: U+0000..U+007F
    if (chars[1] == 0 && chars[0] <= 0x7f) {
        return static_cast<uint32_t>(chars[0]);
    }
    // U+0000..U+D7FF or U+E000..U+FFFF
    else if (chars[1] <= 0xD7 || 0xE0 <= chars[1]) {
        return static_cast<uint32_t>((chars[1] << 8) + chars[0]);
    } else if (4 > chars.size()) {
        throw runtime_error("invalid UTF-16 LE sequence");
    }
    // U+10000.. use surrogate pairs
    // U+0000..U+D7FF or U+E000..U+FFFF
    else {
        uint16_t s0 = static_cast<uint16_t>((chars[1] << 8) + chars[0]);
        uint16_t s1 = static_cast<uint16_t>((chars[3] << 8) + chars[2]);
        uint32_t p0 = (s0 - 0xD800) << 10;
        uint32_t p1 = (s1 - 0xDC00);
        return static_cast<uint32_t>(0x10000 + p0 + p1);
    }
    throw runtime_error("invalid UTF-16 LE sequence");
}

void encodeUTF16LE(uint32_t code, vector<uint8_t>& chars)
{
    if (code <= 0xd7ff || (0xE000 <= code && code <= 0xFFFF)) {
        uint8_t c0 = static_cast<uint8_t>(code >> 8);
        uint8_t c1 = (code & 0xff);
        chars.push_back(c1);
        chars.push_back(c0);
    }
    // code page U+10000..U+10FFFF
    else {
        code -= 0x10000;
        uint16_t surrogates[2] = {static_cast<uint16_t>(0xD800 + (code >> 10)),
                                    static_cast<uint16_t>(0xDC00 + (code & 0x3FF))};
        uint8_t c0 = surrogates[0] >> 8;
        uint8_t c1 = surrogates[0] & 0xFF;
        uint8_t c2 = surrogates[1] >> 8;
        uint8_t c3 = surrogates[1] & 0xFF;
        if (!(0xD8 <= c0 && c0 <= 0xDB && 0xDC <= c2 && c2 <= 0xDF)) {
            stringstream os;
            os << "invalid surrogate pairs: ";
            os << setfill('0') << setw(2) << util::hex << static_cast<int>(c1) << " ";
            os << setfill('0') << setw(2) << util::hex << static_cast<int>(c0) << " ";
            os << setfill('0') << setw(2) << util::hex << static_cast<int>(c3) << " ";
            os << setfill('0') << setw(2) << util::hex << static_cast<int>(c2) << " ";
            throw runtime_error(os.str());
            // throw runtime_error("invalid surrogate pairs: ");
        }
        chars.push_back(c1);
        chars.push_back(c0);
        chars.push_back(c3);
        chars.push_back(c2);
    }
}

int UTF16BECharSize(const string& str)
{
    vector<uint8_t> chars(str.size());
    for (size_t i = 0; i < str.size(); i++) {
        chars[i] = str[i];
    }
    int numChars = 0;
    if (isValidUTF16BE(chars, numChars)) {
        return numChars;
    } else
        return -1;
}

int UTF16LECharSize(const string& str)
{
    vector<uint8_t> chars(str.size());
    for (size_t i = 0; i < str.size(); i++) {
        chars[i] = str[i];
    }
    int numChars = 0;
    if (isValidUTF16LE(chars, numChars)) {
        return numChars;
    } else
        return -1;
}

bool isValidUTF16BE(vector<uint8_t>& chars)
{
    int numChars = 0;
    return isValidUTF16BE(chars, numChars);
}

bool isValidUTF16BE(vector<uint8_t>& chars, int& numChars)
{
    numChars = 0;
    for (size_t i = 0; i < chars.size(); i++, numChars++) {
        if (i + 2 > chars.size()) {
            return false;
        } else if (chars[i] <= 0xD7 || 0xE0 <= chars[i]) {
            i++;
        } else if (i + 4 > chars.size()) {
            return false;
        }
        // D800~DBFF + DC00~DF00
        else if (0xD8 <= chars[i] && chars[i] <= 0xDB && 0xDC <= chars[i + 2] && chars[i + 2] <= 0xDF) {
            i += 3;
        } else
            return false;
    }
    return true;
}

bool isValidUTF16LE(vector<uint8_t>& chars)
{
    int numChars = 0;
    return isValidUTF16LE(chars, numChars);
}

bool isValidUTF16LE(vector<uint8_t>& chars, int& numChars)
{
    numChars = 0;
    for (size_t i = 0; i < chars.size(); i++, numChars++) {
        if (i + 2 > chars.size()) {
            return false;
        } else if (chars[i + 1] <= 0xD7 || 0xE0 <= chars[i + 1]) {
            i++;
        } else if (i + 4 > chars.size()) {
            return false;
        }
        // D800~DBFF + DC00~DF00
        else if (0xD8 <= chars[i + 1] && chars[i + 1] <= 0xDB && 0xDC <= chars[i + 3] && chars[i + 3] <= 0xDF) {
            i += 3;
        } else
            return false;
    }
    return true;
}

}  // namespace util
}  // namespace proptest
