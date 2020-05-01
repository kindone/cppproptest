#include "api.hpp"
#include "string.hpp"

namespace PropertyBasedTesting {

namespace util {

IosFlagSaver::IosFlagSaver(std::ostream& _ios):
    ios(_ios),
    f(_ios.flags()) {
}
IosFlagSaver::~IosFlagSaver() {
    ios.flags(f);
}

}


std::ostream& validChar(std::ostream& os, uint8_t c) {
    os << c;
    return os;
}

std::ostream& validChar(std::ostream& os, uint8_t c1, uint8_t c2) {
    util::IosFlagSaver iosFlagSaver(os);
    os << "\\u" <<  std::setfill('0') << std::setw(2) << std::hex << static_cast<int>(c1) << static_cast<int>(c2);
    return os;
}

std::ostream& validChar(std::ostream& os, uint8_t c1, uint8_t c2, uint8_t c3) {
    util::IosFlagSaver iosFlagSaver(os);
    os << "\\U" <<  std::setfill('0') << std::setw(2) << std::hex << static_cast<int>(0) << static_cast<int>(c1) << static_cast<int>(c2) << static_cast<int>(c3);
    return os;
}

std::ostream& validChar(std::ostream& os, uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4) {
    util::IosFlagSaver iosFlagSaver(os);
    os << "\\U" <<  std::setfill('0') << std::setw(2) << std::hex << static_cast<int>(c1) << static_cast<int>(c2) << static_cast<int>(c3) << static_cast<int>(c4);
    return os;
}

std::ostream& codepage(std::ostream& os, uint32_t code) {
    util::IosFlagSaver iosFlagSaver(os);
    if(code < 0x10000) {
        os << "\\u";
        os <<  std::setfill('0') << std::setw(2) << std::hex << ((code & 0xff00) >> 8);
        os <<  std::setfill('0') << std::setw(2) << std::hex << (code & 0xff);
    }
    else {
        os << "\\U";
        os << std::setfill('0') << std::setw(2) << std::hex << ((code & 0xff0000) >> 16);
        os << std::setfill('0') << std::setw(2) << std::hex << ((code & 0xff00) >> 8);
        os << std::setfill('0') << std::setw(2) << std::hex << (code & 0xff);
    }

    return os;
}

std::ostream& charAsHex(std::ostream& os, uint8_t c) {
    util::IosFlagSaver iosFlagSaver(os);
    os << "\\x" << std::setfill('0') << std::setw(2) << std::hex << static_cast<int>(c);
    return os;
}

std::ostream& UTF8ToHex(std::ostream& os, std::vector<uint8_t>& chars) {
    util::IosFlagSaver iosFlagSaver(os);

    for(size_t i = 0; i < chars.size(); i++) {
        os << /*"\\x" <<*/ std::setfill('0') << std::setw(2) << std::hex << static_cast<int>(chars[i]);
    }

    return os;
}

std::ostream& decodeUTF8(std::ostream& os, const std::string& str) {
    std::vector<uint8_t> chars;
    chars.reserve(str.size());
    for(size_t i = 0; i < str.size(); i++) {
        chars.push_back(str[i]);
    }
    return decodeUTF8(os, chars);
}

/*
 * legal utf-8 byte sequence
 * http://www.unicode.org/versions/Unicode6.0.0/ch03.pdf - page 94
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
std::ostream& decodeUTF8(std::ostream& os, std::vector<uint8_t>& chars)
{
    for (size_t i = 0; i < chars.size(); i++) {
        //U+0000..U+007F
        if (chars[i] <= 0x7f) {
            os << static_cast<char>(chars[i]);
        } else if (i + 1 >= chars.size()) {
            charAsHex(os, chars[i]);
        // U+0080..U+07FF
        } else if (0xc2 <= chars[i] && chars[i] <= 0xdf) {
            if (0x80 <= chars[i + 1] && chars[i + 1] <= 0xbf) {
                // validChar(os, chars[i] - 0xc2, chars[i+1]);
                codepage(os, 0x80 + (chars[i]-0xc2)*(0xbf-0x80+1) + (chars[i+1]-0x80));
                i++;
            } else {
                charAsHex(os, chars[i]);
            }
        } else if (i + 2 >= chars.size()) {
            charAsHex(os, chars[i]);
        // U+0800..U+0FFF
        } else if (0xe0 == chars[i]) {
            if (0xa0 <= chars[i + 1] && chars[i + 1] <= 0xbf && 0x80 <= chars[i + 2] && chars[i + 2] <= 0xbf) {
                // validChar(os, chars[i], chars[i+1], chars[i+2]);
                codepage(os, 0x0800 + (chars[i]-0xe0)*(0xbf-0xa0+1)*(0xbf-0x80+1)
                 + (chars[i+1]-0xa0)*(0xbf-0x80+1) + (chars[i+2]-0x80));
                i += 2;
            } else
                charAsHex(os, chars[i]);
        // U+1000..U+CFFF
        } else if (0xe1 <= chars[i] && chars[i] <= 0xec) {
            if (0x80 <= chars[i + 1] && chars[i + 1] <= 0xbf && 0x80 <= chars[i + 2] && chars[i + 2] <= 0xbf) {
                // validChar(os, chars[i], chars[i+1], chars[i+2]);
                codepage(os, 0x1000 + (chars[i]-0xe1)*(0xbf-0x80+1)*(0xbf-0x80+1)
                 + (chars[i+1]-0x80)*(0xbf-0x80+1) + (chars[i+2]-0x80));
                i += 2;
            } else
                charAsHex(os, chars[i]);
        // U+D000..U+D7FF
        } else if (0xed == chars[i]) {
            if (0x80 <= chars[i + 1] && chars[i + 1] <= 0x9f && 0x80 <= chars[i + 2] && chars[i + 2] <= 0xbf) {
                // validChar(os, chars[i], chars[i+1], chars[i+2]);
                codepage(os, 0xD000 + (chars[i]-0xed)*(0x9f-0x80+1)*(0xbf-0x80+1)
                 + (chars[i+1]-0x80)*(0xbf-0x80+1) + (chars[i+2]-0x80));
                i += 2;
            } else
                charAsHex(os, chars[i]);
        // U+E000..U+FFFF
        } else if (0xee <= chars[i] && chars[i] <= 0xef) {
            if (0x80 <= chars[i + 1] && chars[i + 1] <= 0xbf && 0x80 <= chars[i + 2] && chars[i + 2] <= 0xbf) {
                // validChar(os, chars[i], chars[i+1], chars[i+2]);
                codepage(os, 0xe000 + (chars[i]-0xee)*(0xbf-0x80+1)*(0xbf-0x80+1)
                 + (chars[i+1]-0x80)*(0xbf-0x80+1) + (chars[i+2]-0x80));
                i += 2;
            } else
                charAsHex(os, chars[i]);
        } else if (i + 3 >= chars.size()) {
            charAsHex(os, chars[i]);
        // U+10000..U+3FFFF
        } else if (0xf0 == chars[i]) {
            if (0x90 <= chars[i + 1] && chars[i + 1] <= 0xbf && 0x80 <= chars[i + 2] && chars[i + 2] <= 0xbf &&
                0x80 <= chars[i + 3] && chars[i + 3] <= 0xbf) {
                // validChar(os, chars[i], chars[i+1], chars[i+2], chars[i+3]);
                codepage(os, 0x10000 + (chars[i]-0xf0)*(0xbf-0x90+1)*(0xbf-0x80+1)*(0xbf-0x80+1)
                 + (chars[i+1]-0x90)*(0xbf-0x80+1)*(0xbf-0x80+1)
                 + (chars[i+2]-0x80)*(0xbf-0x80+1)
                 + (chars[i+3]-0x80));
                i += 3;
            } else
                charAsHex(os, chars[i]);
        // U+40000..U+FFFFF
        } else if (0xf1 <= chars[i] && chars[i] <= 0xf3) {
            if (0x80 <= chars[i + 1] && chars[i + 1] <= 0xbf && 0x80 <= chars[i + 2] && chars[i + 2] <= 0xbf &&
                0x80 <= chars[i + 3] && chars[i + 3] <= 0xbf) {
                // validChar(os, chars[i], chars[i+1], chars[i+2], chars[i+3]);
                codepage(os, 0x40000 + (chars[i]-0xf1)*(0xbf-0x80+1)*(0xbf-0x80+1)*(0xbf-0x80+1)
                 + (chars[i+1]-0x80)*(0xbf-0x80+1)*(0xbf-0x80+1)
                 + (chars[i+2]-0x80)*(0xbf-0x80+1)
                 + (chars[i+3]-0x80));
                i += 3;
            } else
                charAsHex(os, chars[i]);
        // U+100000..U+10FFFF
        } else if (0xf4 == chars[i]) {
            if (0x80 <= chars[i + 1] && chars[i + 1] <= 0x8f && 0x80 <= chars[i + 2] && chars[i + 2] <= 0xbf &&
                0x80 <= chars[i + 3] && chars[i + 3] <= 0xbf) {
                // validChar(os, chars[i], chars[i+1], chars[i+2], chars[i+3]);
                codepage(os, 0x100000 + (chars[i]-0xf4)*(0x8f-0x80+1)*(0xbf-0x80+1)*(0xbf-0x80+1)
                 + (chars[i+1]-0x80)*(0xbf-0x80+1)*(0xbf-0x80+1)
                 + (chars[i+2]-0x80)*(0xbf-0x80+1)
                 + (chars[i+3]-0x80));
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

bool isValidUTF8(std::vector<uint8_t>& chars)
{
    for (size_t i = 0; i < chars.size(); i++) {
        if (chars[i] <= 0x7f) {
            continue;
        } else if (i + 1 >= chars.size()) {
            return false;
        } else if (0xc2 <= chars[i] && chars[i] <= 0xdf) {
            if (0x80 <= chars[i + 1] && chars[i + 1] <= 0xbf) {
                i++;
            } else
                return false;
        } else if (i + 2 >= chars.size()) {
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
        } else if (i + 3 >= chars.size()) {
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

}