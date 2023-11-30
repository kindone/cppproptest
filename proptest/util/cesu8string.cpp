#include "../api.hpp"
#include "cesu8string.hpp"
#include "unicode.hpp"
#include "../util/std.hpp"

namespace proptest {

size_t CESU8String::charsize() const
{
    int size = util::CESU8CharSize(*this);
    if (size < 0)
        throw runtime_error("Not a valid CESU-8 string");

    return static_cast<size_t>(size);
}

namespace util {

ostream& validCESU8Char(ostream& os, uint8_t c)
{
    if (static_cast<char>(c) == '\\')
        os << "\\\\";
    else if (c < 0x20 || c == 0x7f) {
        util::IosFlagSaver iosFlagSaver(os);
        os << "\\x" << setfill('0') << setw(2) << util::hex << static_cast<int>(c);
    } else
        os << static_cast<char>(c);

    return os;
}

ostream& validCESU8Char(ostream& os, uint8_t c1, uint8_t c2)
{
    util::IosFlagSaver iosFlagSaver(os);
    os << "\\u" << setfill('0') << setw(2) << util::hex << static_cast<int>(c1) << static_cast<int>(c2);
    return os;
}

ostream& validCESU8Char(ostream& os, uint8_t c1, uint8_t c2, uint8_t c3)
{
    util::IosFlagSaver iosFlagSaver(os);
    os << "\\U" << setfill('0') << setw(2) << util::hex << static_cast<int>(0) << static_cast<int>(c1)
       << static_cast<int>(c2) << static_cast<int>(c3);
    return os;
}

ostream& validCESU8Char(ostream& os, uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4)
{
    util::IosFlagSaver iosFlagSaver(os);
    os << "\\U" << setfill('0') << setw(2) << util::hex << static_cast<int>(c1) << static_cast<int>(c2)
       << static_cast<int>(c3) << static_cast<int>(c4);
    return os;
}

ostream& cesu8AsHex(ostream& os, uint8_t c)
{
    util::IosFlagSaver iosFlagSaver(os);
    os << "\\x" << setfill('0') << setw(2) << util::hex << static_cast<int>(c);
    return os;
}

ostream& cesu8AsHex(ostream& os, uint8_t c1, uint8_t c2)
{
    util::IosFlagSaver iosFlagSaver(os);
    os << "\\x" << setfill('0') << setw(2) << util::hex << static_cast<int>(c1);
    os << "\\x" << setfill('0') << setw(2) << util::hex << static_cast<int>(c2);
    return os;
}

ostream& cesu8AsHex(ostream& os, uint8_t c1, uint8_t c2, uint8_t c3)
{
    util::IosFlagSaver iosFlagSaver(os);
    os << "\\x" << setfill('0') << setw(2) << util::hex << static_cast<int>(c1);
    os << "\\x" << setfill('0') << setw(2) << util::hex << static_cast<int>(c2);
    os << "\\x" << setfill('0') << setw(2) << util::hex << static_cast<int>(c3);
    return os;
}

ostream& cesu8AsHex(ostream& os, uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4)
{
    util::IosFlagSaver iosFlagSaver(os);
    os << "\\x" << setfill('0') << setw(2) << util::hex << static_cast<int>(c1);
    os << "\\x" << setfill('0') << setw(2) << util::hex << static_cast<int>(c2);
    os << "\\x" << setfill('0') << setw(2) << util::hex << static_cast<int>(c3);
    os << "\\x" << setfill('0') << setw(2) << util::hex << static_cast<int>(c4);
    return os;
}

ostream& CESU8ToHex(ostream& os, vector<uint8_t>& chars)
{
    util::IosFlagSaver iosFlagSaver(os);

    if (chars.size() > 0)
        os << /*"\\x" <<*/ setfill('0') << setw(2) << util::hex << static_cast<int>(chars[0]);
    for (size_t i = 1; i < chars.size(); i++) {
        os << " " << setfill('0') << setw(2) << util::hex << static_cast<int>(chars[i]);
    }

    return os;
}

ostream& decodeCESU8(ostream& os, const string& str)
{
    vector<uint8_t> chars;
    chars.reserve(str.size());
    for (size_t i = 0; i < str.size(); i++) {
        chars.push_back(str[i]);
    }
    return decodeCESU8(os, chars);
}

ostream& decodeCESU8(ostream& os, const CESU8String& str)
{
    vector<uint8_t> chars;
    chars.reserve(str.size());
    for (size_t i = 0; i < str.size(); i++) {
        chars.push_back(str[i]);
    }
    return decodeCESU8(os, chars);
}

/*
 * legal CESU-8 byte sequence
 *
 *  Code Points        1st       2s       3s       4s
 * U+0000..U+007F     00..7F
 * U+0080..U+07FF     C2..DF   80..BF
 * U+0800..U+0FFF     E0       A0..BF   80..BF
 * U+1000..U+CFFF     E1..EC   80..BF   80..BF
 * U+D000..U+D7FF     ED       80..9F   80..BF
 *[U+D800..U+DBFF]    ED       A0..AF   80..BF (high surrogates)
 *[U+DC00..U+DFFF]    ED       B0..BF   80..BF (low surrogates)
 * U+E000..U+FFFF     EE..EF   80..BF   80..BF
 * U+10000..: 6-byte surrogate pairs (U+D800..U+DBFF + U+DC00..U+DFFF)
 */
ostream& decodeCESU8(ostream& os, vector<uint8_t>& chars)
{
    for (size_t i = 0; i < chars.size(); i++) {
        // U+0000..U+007F
        if (chars[i] <= 0x7f) {
            validCESU8Char(os, chars[i]);
            // os << static_cast<char>(chars[i]);
        } else if (i + 2 > chars.size()) {
            cesu8AsHex(os, chars[i]);
            // U+0080..U+07FF
        } else if (0xc2 <= chars[i] && chars[i] <= 0xdf) {
            if (0x80 <= chars[i + 1] && chars[i + 1] <= 0xbf) {
                // validChar(os, chars[i] - 0xc2, chars[i+1]);
                codepage(os, 0x80 + (chars[i] - 0xc2) * (0xbf - 0x80 + 1) + (chars[i + 1] - 0x80));
                // charAsHex(os, chars[i], chars[i+1]);
                i++;
            } else {
                cesu8AsHex(os, chars[i]);
            }
        } else if (i + 3 > chars.size()) {
            cesu8AsHex(os, chars[i]);
            // U+0800..U+0FFF
        } else if (0xe0 == chars[i]) {
            if (0xa0 <= chars[i + 1] && chars[i + 1] <= 0xbf && 0x80 <= chars[i + 2] && chars[i + 2] <= 0xbf) {
                // validChar(os, chars[i], chars[i+1], chars[i+2]);
                codepage(os, 0x0800 + (chars[i] - 0xe0) * (0xbf - 0xa0 + 1) * (0xbf - 0x80 + 1) +
                                 (chars[i + 1] - 0xa0) * (0xbf - 0x80 + 1) + (chars[i + 2] - 0x80));
                // charAsHex(os, chars[i], chars[i+1], chars[i+2]);
                i += 2;
            } else
                cesu8AsHex(os, chars[i]);
            // U+1000..U+CFFF
        } else if (0xe1 <= chars[i] && chars[i] <= 0xec) {
            if (0x80 <= chars[i + 1] && chars[i + 1] <= 0xbf && 0x80 <= chars[i + 2] && chars[i + 2] <= 0xbf) {
                // validChar(os, chars[i], chars[i+1], chars[i+2]);
                codepage(os, 0x1000 + (chars[i] - 0xe1) * (0xbf - 0x80 + 1) * (0xbf - 0x80 + 1) +
                                 (chars[i + 1] - 0x80) * (0xbf - 0x80 + 1) + (chars[i + 2] - 0x80));
                // charAsHex(os, chars[i], chars[i+1], chars[i+2]);
                i += 2;
            } else
                cesu8AsHex(os, chars[i]);
            // U+D000..U+D7FF
        } else if (0xed == chars[i]) {
            if (0x80 <= chars[i + 1] && chars[i + 1] <= 0x9f && 0x80 <= chars[i + 2] && chars[i + 2] <= 0xbf) {
                // validChar(os, chars[i], chars[i+1], chars[i+2]);
                codepage(os, 0xD000 + (chars[i] - 0xed) * (0x9f - 0x80 + 1) * (0xbf - 0x80 + 1) +
                                 (chars[i + 1] - 0x80) * (0xbf - 0x80 + 1) + (chars[i + 2] - 0x80));
                // charAsHex(os, chars[i], chars[i+1], chars[i+2]);
                i += 2;
            } else {
                if (i + 6 > chars.size()) {
                    cesu8AsHex(os, chars[i]);
                } else if (0xa0 <= chars[i + 1] && chars[i + 1] <= 0xaf && 0x80 <= chars[i + 2] &&
                           chars[i + 2] <= 0xbf && 0xed == chars[i + 3] && 0xb0 <= chars[i + 4] &&
                           chars[i + 4] <= 0xbf && 0x80 <= chars[i + 5] && chars[i + 5] <= 0xbf) {
                    // surrogate pairs
                    uint16_t high = 0xD800 + (chars[i] - 0xed) * (0xaf - 0xa0 + 1) * (0xbf - 0x80 + 1) +
                                    (chars[i + 1] - 0xa0) * (0xbf - 0x80 + 1) + (chars[i + 2] - 0x80);
                    uint16_t low = 0xDC00 + (chars[i + 3] - 0xed) * (0xbf - 0xb0 + 1) * (0xbf - 0x80 + 1) +
                                   (chars[i + 4] - 0xb0) * (0xbf - 0x80 + 1) + (chars[i + 5] - 0x80);
                    uint32_t code = 0x10000 + ((high & 0x03FF) << 10) + (low & 0x03FF);
                    codepage(os, code);
                    i += 5;
                } else
                    cesu8AsHex(os, chars[i]);
            }
            // U+E000..U+FFFF
        } else if (0xee <= chars[i] && chars[i] <= 0xef) {
            if (0x80 <= chars[i + 1] && chars[i + 1] <= 0xbf && 0x80 <= chars[i + 2] && chars[i + 2] <= 0xbf) {
                // validChar(os, chars[i], chars[i+1], chars[i+2]);
                codepage(os, 0xe000 + (chars[i] - 0xee) * (0xbf - 0x80 + 1) * (0xbf - 0x80 + 1) +
                                 (chars[i + 1] - 0x80) * (0xbf - 0x80 + 1) + (chars[i + 2] - 0x80));
                // charAsHex(os, chars[i], chars[i+1], chars[i+2]);
                i += 2;
            } else
                cesu8AsHex(os, chars[i]);
        } else {
            cesu8AsHex(os, chars[i]);
        }
    }
    return os;
}

uint32_t decodeCESU8(vector<uint8_t>& chars)
{
    if (0 < chars.size()) {
        // U+0000..U+007F
        if (chars[0] <= 0x7f) {
            return static_cast<uint32_t>(chars[0]);
        } else if (2 > chars.size()) {
            throw runtime_error("invalid CESU8 sequence");
            // U+0080..U+07FF
        } else if (0xc2 <= chars[0] && chars[0] <= 0xdf) {
            if (0x80 <= chars[1] && chars[1] <= 0xbf) {
                return static_cast<uint32_t>(0x80 + (chars[0] - 0xc2) * (0xbf - 0x80 + 1) + (chars[1] - 0x80));
            } else {
                throw runtime_error("invalid CESU8 sequence");
            }
        } else if (3 > chars.size()) {
            throw runtime_error("invalid CESU8 sequence");
            // U+0800..U+0FFF
        } else if (0xe0 == chars[0]) {
            if (0xa0 <= chars[1] && chars[1] <= 0xbf && 0x80 <= chars[2] && chars[2] <= 0xbf) {
                return static_cast<uint32_t>(0x0800 + (chars[0] - 0xe0) * (0xbf - 0xa0 + 1) * (0xbf - 0x80 + 1) +
                                 (chars[1] - 0xa0) * (0xbf - 0x80 + 1) + (chars[2] - 0x80));
            } else
                throw runtime_error("invalid CESU8 sequence");
            // U+1000..U+CFFF
        } else if (0xe1 <= chars[0] && chars[0] <= 0xec) {
            if (0x80 <= chars[1] && chars[1] <= 0xbf && 0x80 <= chars[2] && chars[2] <= 0xbf) {
                // validChar(os, chars[0], chars[1], chars[2]);
                return static_cast<uint32_t>(0x1000 + (chars[0] - 0xe1) * (0xbf - 0x80 + 1) * (0xbf - 0x80 + 1) +
                                 (chars[1] - 0x80) * (0xbf - 0x80 + 1) + (chars[2] - 0x80));
            } else
                throw runtime_error("invalid CESU8 sequence");
            // U+D000..U+D7FF
        } else if (0xed == chars[0]) {
            if (0x80 <= chars[1] && chars[1] <= 0x9f && 0x80 <= chars[2] && chars[2] <= 0xbf) {
                return static_cast<uint32_t>(0xD000 + (chars[0] - 0xed) * (0x9f - 0x80 + 1) * (0xbf - 0x80 + 1) +
                                 (chars[1] - 0x80) * (0xbf - 0x80 + 1) + (chars[2] - 0x80));
            } else {
                if (6 > chars.size()) {
                    throw runtime_error("invalid CESU8 sequence");
                } else if (0xa0 <= chars[1] && chars[1] <= 0xaf && 0x80 <= chars[2] &&
                           chars[2] <= 0xbf && 0xed == chars[3] && 0xb0 <= chars[4] &&
                           chars[4] <= 0xbf && 0x80 <= chars[5] && chars[5] <= 0xbf) {
                    // surrogate pairs
                    uint16_t high = 0xD800 + (chars[0] - 0xed) * (0xaf - 0xa0 + 1) * (0xbf - 0x80 + 1) +
                                    (chars[1] - 0xa0) * (0xbf - 0x80 + 1) + (chars[2] - 0x80);
                    uint16_t low = 0xDC00 + (chars[3] - 0xed) * (0xbf - 0xb0 + 1) * (0xbf - 0x80 + 1) +
                                   (chars[4] - 0xb0) * (0xbf - 0x80 + 1) + (chars[5] - 0x80);
                    uint32_t code = 0x10000 + ((high & 0x03FF) << 10) + (low & 0x03FF);
                    return static_cast<uint32_t>(code);
                } else
                    throw runtime_error("invalid CESU8 sequence");
            }
            // U+E000..U+FFFF
        } else if (0xee <= chars[0] && chars[0] <= 0xef) {
            if (0x80 <= chars[1] && chars[1] <= 0xbf && 0x80 <= chars[2] && chars[2] <= 0xbf) {
                // validChar(os, chars[0], chars[1], chars[2]);
                return static_cast<uint32_t>(0xe000 + (chars[0] - 0xee) * (0xbf - 0x80 + 1) * (0xbf - 0x80 + 1) +
                                 (chars[1] - 0x80) * (0xbf - 0x80 + 1) + (chars[2] - 0x80));
            } else
                throw runtime_error("invalid CESU8 sequence");
        } else {
            throw runtime_error("invalid CESU8 sequence");
        }
    }
    throw runtime_error("invalid CESU8 sequence");
}

void encodeCESU8(uint32_t code, vector<uint8_t>& chars)
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
    } else if (code <= 0x10FFFF) {
        code -= 0x10000;
        uint16_t surrogates[2] = {static_cast<uint16_t>(0xD800 + (code >> 10)),
                                    static_cast<uint16_t>(0xDC00 + (code & 0x3FF))};
        for (int j = 0; j < 2; j++) {
            code = surrogates[j];
            code -= (j == 0 ? 0xd800 : 0xdc00);
            uint8_t c0 = 0xed;
            uint8_t c1 = ((code >> 6) & 0x3f) + (j == 0 ? 0xa0 : 0xb0);
            uint8_t c2 = (code & 0x3f) + 0x80;
            chars.push_back(c0);
            chars.push_back(c1);
            chars.push_back(c2);
        }
    } else {
        throw runtime_error("should not reach here. code too big");
    }
}

int CESU8CharSize(const string& str)
{
    vector<uint8_t> chars(str.size());
    for (size_t i = 0; i < str.size(); i++) {
        chars[i] = str[i];
    }
    int numChars = 0;
    if (isValidCESU8(chars, numChars)) {
        return numChars;
    } else
        return -1;
}

bool isValidCESU8(vector<uint8_t>& chars)
{
    int numChars = 0;
    return isValidCESU8(chars, numChars);
}

bool isValidCESU8(vector<uint8_t>& chars, int& numChars)
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
            } else {
                if (i + 6 > chars.size()) {
                    return false;
                } else if (0xa0 <= chars[i + 1] && chars[i + 1] <= 0xaf && 0x80 <= chars[i + 2] &&
                           chars[i + 2] <= 0xbf && 0xed == chars[i + 3] && 0xb0 <= chars[i + 4] &&
                           chars[i + 4] <= 0xbf && 0x80 <= chars[i + 5] && chars[i + 5] <= 0xbf) {
                    i += 5;
                } else
                    return false;
            }
            // U+E000..U+FFFF
        } else if (0xee <= chars[i] && chars[i] <= 0xef) {
            if (0x80 <= chars[i + 1] && chars[i + 1] <= 0xbf && 0x80 <= chars[i + 2] && chars[i + 2] <= 0xbf) {
                i += 2;
            } else
                return false;
        } else
            return false;
    }
    return true;
}

}  // namespace util
}  // namespace proptest
