#include "../api.hpp"
#include "cesu8string.hpp"
#include "unicode.hpp"

namespace PropertyBasedTesting {

namespace util {

std::ostream& validCESU8Char(std::ostream& os, uint8_t c)
{
    if (static_cast<char>(c) == '\\')
        os << "\\\\";
    else if (c < 0x20) {
        util::IosFlagSaver iosFlagSaver(os);
        os << "\\x" << std::setfill('0') << std::setw(2) << std::hex << static_cast<int>(c);
    } else
        os << static_cast<char>(c);

    return os;
}

std::ostream& validCESU8Char(std::ostream& os, uint8_t c1, uint8_t c2)
{
    util::IosFlagSaver iosFlagSaver(os);
    os << "\\u" << std::setfill('0') << std::setw(2) << std::hex << static_cast<int>(c1) << static_cast<int>(c2);
    return os;
}

std::ostream& validCESU8Char(std::ostream& os, uint8_t c1, uint8_t c2, uint8_t c3)
{
    util::IosFlagSaver iosFlagSaver(os);
    os << "\\U" << std::setfill('0') << std::setw(2) << std::hex << static_cast<int>(0) << static_cast<int>(c1)
       << static_cast<int>(c2) << static_cast<int>(c3);
    return os;
}

std::ostream& validCESU8Char(std::ostream& os, uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4)
{
    util::IosFlagSaver iosFlagSaver(os);
    os << "\\U" << std::setfill('0') << std::setw(2) << std::hex << static_cast<int>(c1) << static_cast<int>(c2)
       << static_cast<int>(c3) << static_cast<int>(c4);
    return os;
}

std::ostream& cesu8AsHex(std::ostream& os, uint8_t c)
{
    util::IosFlagSaver iosFlagSaver(os);
    os << "\\x" << std::setfill('0') << std::setw(2) << std::hex << static_cast<int>(c);
    return os;
}

std::ostream& cesu8AsHex(std::ostream& os, uint8_t c1, uint8_t c2)
{
    util::IosFlagSaver iosFlagSaver(os);
    os << "\\x" << std::setfill('0') << std::setw(2) << std::hex << static_cast<int>(c1);
    os << "\\x" << std::setfill('0') << std::setw(2) << std::hex << static_cast<int>(c2);
    return os;
}

std::ostream& cesu8AsHex(std::ostream& os, uint8_t c1, uint8_t c2, uint8_t c3)
{
    util::IosFlagSaver iosFlagSaver(os);
    os << "\\x" << std::setfill('0') << std::setw(2) << std::hex << static_cast<int>(c1);
    os << "\\x" << std::setfill('0') << std::setw(2) << std::hex << static_cast<int>(c2);
    os << "\\x" << std::setfill('0') << std::setw(2) << std::hex << static_cast<int>(c3);
    return os;
}

std::ostream& cesu8AsHex(std::ostream& os, uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4)
{
    util::IosFlagSaver iosFlagSaver(os);
    os << "\\x" << std::setfill('0') << std::setw(2) << std::hex << static_cast<int>(c1);
    os << "\\x" << std::setfill('0') << std::setw(2) << std::hex << static_cast<int>(c2);
    os << "\\x" << std::setfill('0') << std::setw(2) << std::hex << static_cast<int>(c3);
    os << "\\x" << std::setfill('0') << std::setw(2) << std::hex << static_cast<int>(c4);
    return os;
}

std::ostream& CESU8ToHex(std::ostream& os, std::vector<uint8_t>& chars)
{
    util::IosFlagSaver iosFlagSaver(os);

    if (chars.size() > 0)
        os << /*"\\x" <<*/ std::setfill('0') << std::setw(2) << std::hex << static_cast<int>(chars[0]);
    for (size_t i = 1; i < chars.size(); i++) {
        os << " " << std::setfill('0') << std::setw(2) << std::hex << static_cast<int>(chars[i]);
    }

    return os;
}

std::ostream& decodeCESU8(std::ostream& os, const std::string& str)
{
    std::vector<uint8_t> chars;
    chars.reserve(str.size());
    for (size_t i = 0; i < str.size(); i++) {
        chars.push_back(str[i]);
    }
    return decodeCESU8(os, chars);
}

std::ostream& decodeCESU8(std::ostream& os, const CESU8String& str)
{
    std::vector<uint8_t> chars;
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
std::ostream& decodeCESU8(std::ostream& os, std::vector<uint8_t>& chars)
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

bool isValidCESU8(std::vector<uint8_t>& chars)
{
    for (size_t i = 0; i < chars.size(); i++) {
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
}  // namespace PropertyBasedTesting