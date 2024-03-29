#include "../api.hpp"
#include "utf8string.hpp"
#include "unicode.hpp"
#include "std.hpp"

namespace proptest {

size_t UTF8String::charsize() const
{
    int size = util::UTF8CharSize(*this);
    if (size < 0)
        throw runtime_error("Not a valid UTF-8 string");

    return static_cast<size_t>(size);
}

namespace util {

IosFlagSaver::IosFlagSaver(ostream& _ios) : ios(_ios), f(_ios.flags()) {}
IosFlagSaver::~IosFlagSaver()
{
    ios.flags(f);
}

ostream& UTF8ToHex(ostream& os, vector<uint8_t>& chars)
{
    util::IosFlagSaver iosFlagSaver(os);

    if (chars.size() > 0)
        os << /*"\\x" <<*/ setfill('0') << setw(2) << util::hex << static_cast<int>(chars[0]);
    for (size_t i = 1; i < chars.size(); i++) {
        os << " " << setfill('0') << setw(2) << util::hex << static_cast<int>(chars[i]);
    }

    return os;
}

ostream& decodeUTF8(ostream& os, const string& str)
{
    vector<uint8_t> chars;
    chars.reserve(str.size());
    for (size_t i = 0; i < str.size(); i++) {
        chars.push_back(str[i]);
    }
    return decodeUTF8(os, chars);
}

ostream& decodeUTF8(ostream& os, const UTF8String& str)
{
    vector<uint8_t> chars;
    chars.reserve(str.size());
    for (size_t i = 0; i < str.size(); i++) {
        chars.push_back(str[i]);
    }
    return decodeUTF8(os, chars);
}

/*
 * legal utf-8 byte sequence
 * http://www.unicode.org/versions/Unicode6.0.0/ch03.pdf
 *
 *  Code Points        1st       2s       3s       4s
 * U+0000..U+007F     00..7F
 * U+0080..U+07FF     C2..DF   80..BF
 * U+0800..U+0FFF     E0       A0..BF   80..BF
 * U+1000..U+CFFF     E1..EC   80..BF   80..BF
 * U+D000..U+D7FF     ED       80..9F   80..BF
 * U+E000..U+FFFF     EE..EF   80..BF   80..BF
 * U+10000..U+3FFFF   F0       90..BF   80..BF   80..BF
 * U+40000..U+FFFFF   F1..F3   80..BF   80..BF   80..BF
 * U+100000..U+10FFFF F4       80..8F   80..BF   80..BF
 *
 */
ostream& decodeUTF8(ostream& os, vector<uint8_t>& chars)
{
    for (size_t i = 0; i < chars.size(); i++) {
        // U+0000..U+007F
        if (chars[i] <= 0x7f) {
            validChar(os, chars[i]);
            // os << static_cast<char>(chars[i]);
        } else if (i + 2 > chars.size()) {
            charAsHex(os, chars[i]);
            // U+0080..U+07FF
        } else if (0xc2 <= chars[i] && chars[i] <= 0xdf) {
            if (0x80 <= chars[i + 1] && chars[i + 1] <= 0xbf) {
                // validChar(os, chars[i] - 0xc2, chars[i+1]);
                codepage(os, 0x80 + (chars[i] - 0xc2) * (0xbf - 0x80 + 1) + (chars[i + 1] - 0x80));
                // charAsHex(os, chars[i], chars[i+1]);
                i++;
            } else {
                charAsHex(os, chars[i]);
            }
        } else if (i + 3 > chars.size()) {
            charAsHex(os, chars[i]);
            // U+0800..U+0FFF
        } else if (0xe0 == chars[i]) {
            if (0xa0 <= chars[i + 1] && chars[i + 1] <= 0xbf && 0x80 <= chars[i + 2] && chars[i + 2] <= 0xbf) {
                // validChar(os, chars[i], chars[i+1], chars[i+2]);
                codepage(os, 0x0800 + (chars[i] - 0xe0) * (0xbf - 0xa0 + 1) * (0xbf - 0x80 + 1) +
                                 (chars[i + 1] - 0xa0) * (0xbf - 0x80 + 1) + (chars[i + 2] - 0x80));
                // charAsHex(os, chars[i], chars[i+1], chars[i+2]);
                i += 2;
            } else
                charAsHex(os, chars[i]);
            // U+1000..U+CFFF
        } else if (0xe1 <= chars[i] && chars[i] <= 0xec) {
            if (0x80 <= chars[i + 1] && chars[i + 1] <= 0xbf && 0x80 <= chars[i + 2] && chars[i + 2] <= 0xbf) {
                // validChar(os, chars[i], chars[i+1], chars[i+2]);
                codepage(os, 0x1000 + (chars[i] - 0xe1) * (0xbf - 0x80 + 1) * (0xbf - 0x80 + 1) +
                                 (chars[i + 1] - 0x80) * (0xbf - 0x80 + 1) + (chars[i + 2] - 0x80));
                // charAsHex(os, chars[i], chars[i+1], chars[i+2]);
                i += 2;
            } else
                charAsHex(os, chars[i]);
            // U+D000..U+D7FF
        } else if (0xed == chars[i]) {
            if (0x80 <= chars[i + 1] && chars[i + 1] <= 0x9f && 0x80 <= chars[i + 2] && chars[i + 2] <= 0xbf) {
                // validChar(os, chars[i], chars[i+1], chars[i+2]);
                codepage(os, 0xD000 + (chars[i] - 0xed) * (0x9f - 0x80 + 1) * (0xbf - 0x80 + 1) +
                                 (chars[i + 1] - 0x80) * (0xbf - 0x80 + 1) + (chars[i + 2] - 0x80));
                // charAsHex(os, chars[i], chars[i+1], chars[i+2]);
                i += 2;
            } else
                charAsHex(os, chars[i]);
            // U+E000..U+FFFF
        } else if (0xee <= chars[i] && chars[i] <= 0xef) {
            if (0x80 <= chars[i + 1] && chars[i + 1] <= 0xbf && 0x80 <= chars[i + 2] && chars[i + 2] <= 0xbf) {
                // validChar(os, chars[i], chars[i+1], chars[i+2]);
                codepage(os, 0xe000 + (chars[i] - 0xee) * (0xbf - 0x80 + 1) * (0xbf - 0x80 + 1) +
                                 (chars[i + 1] - 0x80) * (0xbf - 0x80 + 1) + (chars[i + 2] - 0x80));
                // charAsHex(os, chars[i], chars[i+1], chars[i+2]);
                i += 2;
            } else
                charAsHex(os, chars[i]);
        } else if (i + 4 > chars.size()) {
            charAsHex(os, chars[i]);
            // U+10000..U+3FFFF
        } else if (0xf0 == chars[i]) {
            if (0x90 <= chars[i + 1] && chars[i + 1] <= 0xbf && 0x80 <= chars[i + 2] && chars[i + 2] <= 0xbf &&
                0x80 <= chars[i + 3] && chars[i + 3] <= 0xbf) {
                // validChar(os, chars[i], chars[i+1], chars[i+2], chars[i+3]);
                codepage(os, 0x10000 + (chars[i] - 0xf0) * (0xbf - 0x90 + 1) * (0xbf - 0x80 + 1) * (0xbf - 0x80 + 1) +
                                 (chars[i + 1] - 0x90) * (0xbf - 0x80 + 1) * (0xbf - 0x80 + 1) +
                                 (chars[i + 2] - 0x80) * (0xbf - 0x80 + 1) + (chars[i + 3] - 0x80));
                // charAsHex(os, chars[i], chars[i+1], chars[i+2], chars[i+3]);
                i += 3;
            } else
                charAsHex(os, chars[i]);
            // U+40000..U+FFFFF
        } else if (0xf1 <= chars[i] && chars[i] <= 0xf3) {
            if (0x80 <= chars[i + 1] && chars[i + 1] <= 0xbf && 0x80 <= chars[i + 2] && chars[i + 2] <= 0xbf &&
                0x80 <= chars[i + 3] && chars[i + 3] <= 0xbf) {
                // validChar(os, chars[i], chars[i+1], chars[i+2], chars[i+3]);
                codepage(os, 0x40000 + (chars[i] - 0xf1) * (0xbf - 0x80 + 1) * (0xbf - 0x80 + 1) * (0xbf - 0x80 + 1) +
                                 (chars[i + 1] - 0x80) * (0xbf - 0x80 + 1) * (0xbf - 0x80 + 1) +
                                 (chars[i + 2] - 0x80) * (0xbf - 0x80 + 1) + (chars[i + 3] - 0x80));
                // charAsHex(os, chars[i], chars[i+1], chars[i+2], chars[i+3]);
                i += 3;
            } else
                charAsHex(os, chars[i]);
            // U+100000..U+10FFFF
        } else if (0xf4 == chars[i]) {
            if (0x80 <= chars[i + 1] && chars[i + 1] <= 0x8f && 0x80 <= chars[i + 2] && chars[i + 2] <= 0xbf &&
                0x80 <= chars[i + 3] && chars[i + 3] <= 0xbf) {
                // validChar(os, chars[i], chars[i+1], chars[i+2], chars[i+3]);
                codepage(os, 0x100000 + (chars[i] - 0xf4) * (0x8f - 0x80 + 1) * (0xbf - 0x80 + 1) * (0xbf - 0x80 + 1) +
                                 (chars[i + 1] - 0x80) * (0xbf - 0x80 + 1) * (0xbf - 0x80 + 1) +
                                 (chars[i + 2] - 0x80) * (0xbf - 0x80 + 1) + (chars[i + 3] - 0x80));
                // charAsHex(os, chars[i], chars[i+1], chars[i+2], chars[i+3]);
                i += 3;
            } else {
                charAsHex(os, chars[i]);
            }
        } else {
            charAsHex(os, chars[i]);
        }
    }
    return os;
}

uint32_t decodeUTF8(vector<uint8_t>& chars)
{
    if (0 < chars.size()) {
        // U+0000..U+007F
        if (chars[0] <= 0x7f) {
            return static_cast<uint32_t>(chars[0]);
        } else if (2 > chars.size()) {
            throw runtime_error("invalid UTF-8 sequence");
            // U+0080..U+07FF
        } else if (0xc2 <= chars[0] && chars[0] <= 0xdf) {
            if (0x80 <= chars[1] && chars[1] <= 0xbf) {
                return static_cast<uint32_t>(0x80 + (chars[0] - 0xc2) * (0xbf - 0x80 + 1) + (chars[1] - 0x80));
            } else {
                throw runtime_error("invalid UTF-8 sequence");
            }
        } else if (3 > chars.size()) {
            throw runtime_error("invalid UTF-8 sequence");
            // U+0800..U+0FFF
        } else if (0xe0 == chars[0]) {
            if (0xa0 <= chars[1] && chars[1] <= 0xbf && 0x80 <= chars[2] && chars[2] <= 0xbf) {
                return static_cast<uint32_t>(0x0800 + (chars[0] - 0xe0) * (0xbf - 0xa0 + 1) * (0xbf - 0x80 + 1) +
                                 (chars[1] - 0xa0) * (0xbf - 0x80 + 1) + (chars[2] - 0x80));
            } else
                throw runtime_error("invalid UTF-8 sequence");
            // U+1000..U+CFFF
        } else if (0xe1 <= chars[0] && chars[0] <= 0xec) {
            if (0x80 <= chars[1] && chars[1] <= 0xbf && 0x80 <= chars[2] && chars[2] <= 0xbf) {
                return static_cast<uint32_t>(0x1000 + (chars[0] - 0xe1) * (0xbf - 0x80 + 1) * (0xbf - 0x80 + 1) +
                                 (chars[1] - 0x80) * (0xbf - 0x80 + 1) + (chars[2] - 0x80));
            } else
                throw runtime_error("invalid UTF-8 sequence");
            // U+D000..U+D7FF
        } else if (0xed == chars[0]) {
            if (0x80 <= chars[1] && chars[1] <= 0x9f && 0x80 <= chars[2] && chars[2] <= 0xbf) {
                return static_cast<uint32_t>(0xD000 + (chars[0] - 0xed) * (0x9f - 0x80 + 1) * (0xbf - 0x80 + 1) +
                                 (chars[1] - 0x80) * (0xbf - 0x80 + 1) + (chars[2] - 0x80));
            } else
                throw runtime_error("invalid UTF-8 sequence");
            // U+E000..U+FFFF
        } else if (0xee <= chars[0] && chars[0] <= 0xef) {
            if (0x80 <= chars[1] && chars[1] <= 0xbf && 0x80 <= chars[2] && chars[2] <= 0xbf) {
                return static_cast<uint32_t>(0xe000 + (chars[0] - 0xee) * (0xbf - 0x80 + 1) * (0xbf - 0x80 + 1) +
                                 (chars[1] - 0x80) * (0xbf - 0x80 + 1) + (chars[2] - 0x80));
            } else
                throw runtime_error("invalid UTF-8 sequence");
        } else if (4 > chars.size()) {
            throw runtime_error("invalid UTF-8 sequence");
            // U+10000..U+3FFFF
        } else if (0xf0 == chars[0]) {
            if (0x90 <= chars[1] && chars[1] <= 0xbf && 0x80 <= chars[2] && chars[2] <= 0xbf &&
                0x80 <= chars[3] && chars[3] <= 0xbf) {
                return static_cast<uint32_t>(0x10000 + (chars[0] - 0xf0) * (0xbf - 0x90 + 1) * (0xbf - 0x80 + 1) * (0xbf - 0x80 + 1) +
                                 (chars[1] - 0x90) * (0xbf - 0x80 + 1) * (0xbf - 0x80 + 1) +
                                 (chars[2] - 0x80) * (0xbf - 0x80 + 1) + (chars[3] - 0x80));
            } else
                throw runtime_error("invalid UTF-8 sequence");
            // U+40000..U+FFFFF
        } else if (0xf1 <= chars[0] && chars[0] <= 0xf3) {
            if (0x80 <= chars[1] && chars[1] <= 0xbf && 0x80 <= chars[2] && chars[2] <= 0xbf &&
                0x80 <= chars[3] && chars[3] <= 0xbf) {
                return static_cast<uint32_t>(0x40000 + (chars[0] - 0xf1) * (0xbf - 0x80 + 1) * (0xbf - 0x80 + 1) * (0xbf - 0x80 + 1) +
                                 (chars[1] - 0x80) * (0xbf - 0x80 + 1) * (0xbf - 0x80 + 1) +
                                 (chars[2] - 0x80) * (0xbf - 0x80 + 1) + (chars[3] - 0x80));
            } else
                throw runtime_error("invalid UTF-8 sequence");
            // U+100000..U+10FFFF
        } else if (0xf4 == chars[0]) {
            if (0x80 <= chars[1] && chars[1] <= 0x8f && 0x80 <= chars[2] && chars[2] <= 0xbf &&
                0x80 <= chars[3] && chars[3] <= 0xbf) {
                return static_cast<uint32_t>(0x100000 + (chars[0] - 0xf4) * (0x8f - 0x80 + 1) * (0xbf - 0x80 + 1) * (0xbf - 0x80 + 1) +
                                 (chars[1] - 0x80) * (0xbf - 0x80 + 1) * (0xbf - 0x80 + 1) +
                                 (chars[2] - 0x80) * (0xbf - 0x80 + 1) + (chars[3] - 0x80));
            } else {
                throw runtime_error("invalid UTF-8 sequence");
            }
        } else {
            throw runtime_error("invalid UTF-8 sequence");
        }
    }
    throw runtime_error("invalid UTF-8 sequence");
}

void encodeUTF8(uint32_t code, vector<uint8_t>& chars)
{
    if (code <= 0x7f) {
        chars.push_back(static_cast<uint8_t>(code));
    } else if (code <= 0x07FF) {
        code -= 0x80;
        uint8_t c0 = static_cast<uint8_t>((code >> 6) + 0xc2);
        uint8_t c1 = (code & 0x3f) + 0x80;
        chars.push_back(c0);
        chars.push_back(c1);
    } else if (code <= 0x0FFF) {
        code -= 0x800;
        uint8_t c0 = 0xe0;
        uint8_t c1 = static_cast<uint8_t>((code >> 6) + 0xa0);
        uint8_t c2 = (code & 0x3f) + 0x80;
        chars.push_back(c0);
        chars.push_back(c1);
        chars.push_back(c2);
    } else if (code <= 0xCFFF) {
        code -= 0x1000;
        uint8_t c0 = static_cast<uint8_t>((code >> 12) + 0xe1);
        uint8_t c1 = ((code >> 6) & 0x3f) + 0x80;
        uint8_t c2 = (code & 0x3f) + 0x80;
        chars.push_back(c0);
        chars.push_back(c1);
        chars.push_back(c2);
    } else if (code <= 0xD7FF) {
        code -= 0xD000;
        uint8_t c0 = 0xed;
        uint8_t c1 = ((code >> 6) & 0x3f) + 0x80;
        uint8_t c2 = (code & 0x3f) + 0x80;
        chars.push_back(c0);
        chars.push_back(c1);
        chars.push_back(c2);
    } else if (code <= 0xDFFF) {
        throw runtime_error("should not reach here. surrogate region");
    } else if (code <= 0xFFFF) {
        code -= 0xE000;
        uint8_t c0 = static_cast<uint8_t>((code >> 12) + 0xee);
        uint8_t c1 = ((code >> 6) & 0x3f) + 0x80;
        uint8_t c2 = (code & 0x3f) + 0x80;
        chars.push_back(c0);
        chars.push_back(c1);
        chars.push_back(c2);
    } else if (code <= 0x3FFFF) {
        code -= 0x10000;
        uint8_t c0 = 0xf0;
        uint8_t c1 = static_cast<uint8_t>((code >> 12) + 0x90);
        uint8_t c2 = ((code >> 6) & 0x3f) + 0x80;
        uint8_t c3 = (code & 0x3f) + 0x80;
        chars.push_back(c0);
        chars.push_back(c1);
        chars.push_back(c2);
        chars.push_back(c3);
    } else if (code <= 0xFFFFF) {
        code -= 0x40000;
        uint8_t c0 = static_cast<uint8_t>((code >> 18) + 0xf1);
        uint8_t c1 = ((code >> 12) & 0x3f) + 0x80;
        uint8_t c2 = ((code >> 6) & 0x3f) + 0x80;
        uint8_t c3 = (code & 0x3f) + 0x80;
        chars.push_back(c0);
        chars.push_back(c1);
        chars.push_back(c2);
        chars.push_back(c3);
    } else if (code <= 0x10FFFF) {
        code -= 0x100000;
        uint8_t c0 = 0xf4;
        uint8_t c1 = static_cast<uint8_t>((code >> 12) + 0x80);
        uint8_t c2 = ((code >> 6) & 0x3f) + 0x80;
        uint8_t c3 = (code & 0x3f) + 0x80;
        chars.push_back(c0);
        chars.push_back(c1);
        chars.push_back(c2);
        chars.push_back(c3);

    } else {
        throw runtime_error("should not reach here. code too big");
    }
}

int UTF8CharSize(const string& str)
{
    vector<uint8_t> chars(str.size());
    for (size_t i = 0; i < str.size(); i++) {
        chars[i] = str[i];
    }
    int numChars = 0;
    if (isValidUTF8(chars, numChars)) {
        return numChars;
    } else
        return -1;
}

bool isValidUTF8(vector<uint8_t>& chars)
{
    int numChars = 0;
    return isValidUTF8(chars, numChars);
}

bool isValidUTF8(vector<uint8_t>& chars, int& numChars)
{
    numChars = 0;
    for (size_t i = 0; i < chars.size(); i++, numChars++) {
        if (chars[i] <= 0x7f) {
            continue;
        } else if (i + 2 > chars.size()) {
            return false;
        } else if (0xc2 <= chars[i] && chars[i] <= 0xdf) {
            if (0x80 <= chars[i + 1] && chars[i + 1] <= 0xbf) {
                i++;
            } else
                return false;
        } else if (i + 3 > chars.size()) {
            return false;
        } else if (0xe0 == chars[i]) {
            if (0xa0 <= chars[i + 1] && chars[i + 1] <= 0xbf && 0x80 <= chars[i + 2] && chars[i + 2] <= 0xbf) {
                i += 2;
            } else
                return false;
        } else if (0xe1 <= chars[i] && chars[i] <= 0xec) {
            if (0x80 <= chars[i + 1] && chars[i + 1] <= 0xbf && 0x80 <= chars[i + 2] && chars[i + 2] <= 0xbf) {
                i += 2;
            } else
                return false;
        } else if (0xed == chars[i]) {
            if (0x80 <= chars[i + 1] && chars[i + 1] <= 0x9f && 0x80 <= chars[i + 2] && chars[i + 2] <= 0xbf) {
                i += 2;
            } else
                return false;
        } else if (0xee <= chars[i] && chars[i] <= 0xef) {
            if (0x80 <= chars[i + 1] && chars[i + 1] <= 0xbf && 0x80 <= chars[i + 2] && chars[i + 2] <= 0xbf) {
                i += 2;
            } else
                return false;
        } else if (i + 4 > chars.size()) {
            return false;
        } else if (0xf0 == chars[i]) {
            if (0x90 <= chars[i + 1] && chars[i + 1] <= 0xbf && 0x80 <= chars[i + 2] && chars[i + 2] <= 0xbf &&
                0x80 <= chars[i + 3] && chars[i + 3] <= 0xbf) {
                i += 3;
            } else
                return false;
        } else if (0xf1 <= chars[i] && chars[i] <= 0xf3) {
            if (0x80 <= chars[i + 1] && chars[i + 1] <= 0xbf && 0x80 <= chars[i + 2] && chars[i + 2] <= 0xbf &&
                0x80 <= chars[i + 3] && chars[i + 3] <= 0xbf) {
                i += 3;
            } else
                return false;

        } else if (0xf4 == chars[i]) {
            if (0x80 <= chars[i + 1] && chars[i + 1] <= 0x8f && 0x80 <= chars[i + 2] && chars[i + 2] <= 0xbf &&
                0x80 <= chars[i + 3] && chars[i + 3] <= 0xbf) {
                i += 3;
            } else
                return false;

        } else
            return false;
    }
    return true;
}

}  // namespace util

}  // namespace proptest
