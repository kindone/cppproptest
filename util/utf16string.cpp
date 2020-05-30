#include "../api.hpp"
#include "utf16string.hpp"
#include "unicode.hpp"
#include <stdexcept>

namespace PropertyBasedTesting {

size_t UTF16BEString::charsize() const
{
    int size = util::UTF16BECharSize(*this);
    if (size < 0)
        throw std::runtime_error("Not a valid UTF-16 BE string");

    return static_cast<size_t>(size);
}

size_t UTF16LEString::charsize() const
{
    int size = util::UTF16LECharSize(*this);
    if (size < 0)
        throw std::runtime_error("Not a valid UTF-16 LE string");

    return static_cast<size_t>(size);
}

namespace util {

std::ostream& decodeUTF16BE(std::ostream& os, const std::string& str)
{
    std::vector<uint8_t> chars;
    chars.reserve(str.size());
    for (size_t i = 0; i < str.size(); i++) {
        chars.push_back(str[i]);
    }
    return decodeUTF16BE(os, chars);
}

std::ostream& decodeUTF16BE(std::ostream& os, const UTF16BEString& str)
{
    std::vector<uint8_t> chars;
    chars.reserve(str.size());
    for (size_t i = 0; i < str.size(); i++) {
        chars.push_back(str[i]);
    }
    return decodeUTF16BE(os, chars);
}

std::ostream& decodeUTF16LE(std::ostream& os, const std::string& str)
{
    std::vector<uint8_t> chars;
    chars.reserve(str.size());
    for (size_t i = 0; i < str.size(); i++) {
        chars.push_back(str[i]);
    }
    return decodeUTF16BE(os, chars);
}

std::ostream& decodeUTF16LE(std::ostream& os, const UTF16LEString& str)
{
    std::vector<uint8_t> chars;
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
std::ostream& decodeUTF16BE(std::ostream& os, std::vector<uint8_t>& chars)
{
    for (size_t i = 0; i < chars.size(); i++) {
        // U+0000..U+007F
        if (i + 2 > chars.size()) {
            charAsHex(os, chars[i]);
            // U+0000..U+D7FF or U+E000..U+FFFF
        } else if (chars[i] <= 0xD7 || 0xE0 <= chars[i]) {
            codepage(os, (chars[i] << 8) + chars[i + 1]);
        }
        // U+10000.. use surrogate pairs
        // chars[i] in D8..DF
        else {
            codepage(os, 0x10000 + ((chars[i] - 0xD8) << 8) + chars[i + 1]);
        }
    }
    return os;
}

std::ostream& decodeUTF16LE(std::ostream& os, std::vector<uint8_t>& chars)
{
    for (size_t i = 0; i < chars.size(); i++) {
        // U+0000..U+007F
        if (i + 2 > chars.size()) {
            charAsHex(os, chars[i], chars[i + 1]);
            // U+0000..U+D7FF or U+E000..U+FFFF
        } else if (chars[i + 1] <= 0xD7 || 0xE0 <= chars[i + 1]) {
            codepage(os, (chars[i + 1] << 8) + chars[i]);
        }
        // U+10000.. use surrogate pairs
        // chars[i] in D8..DF
        else {
            codepage(os, 0x10000 + ((chars[i + 1] - 0xD8) << 8) + chars[i]);
        }
    }
    return os;
}

int UTF16BECharSize(const std::string& str)
{
    std::vector<uint8_t> chars(str.size());
    for (size_t i = 0; i < str.size(); i++) {
        chars[i] = str[i];
    }
    int numChars = 0;
    if (isValidUTF16BE(chars, numChars)) {
        return numChars;
    } else
        return -1;
}

int UTF16LECharSize(const std::string& str)
{
    std::vector<uint8_t> chars(str.size());
    for (size_t i = 0; i < str.size(); i++) {
        chars[i] = str[i];
    }
    int numChars = 0;
    if (isValidUTF16LE(chars, numChars)) {
        return numChars;
    } else
        return -1;
}

bool isValidUTF16BE(std::vector<uint8_t>& chars)
{
    int numChars = 0;
    return isValidUTF16BE(chars, numChars);
}

bool isValidUTF16BE(std::vector<uint8_t>& chars, int& numChars)
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
        else if (chars[i] <= 0xDB && 0xDC <= chars[i + 2] && chars[i + 2] <= 0xDF) {
            i += 3;
        }
    }
    return true;
}

bool isValidUTF16LE(std::vector<uint8_t>& chars)
{
    int numChars = 0;
    return isValidUTF16LE(chars, numChars);
}

bool isValidUTF16LE(std::vector<uint8_t>& chars, int& numChars)
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
        else if (chars[i + 1] <= 0xDB && 0xDC <= chars[i + 3] && chars[i + 3] <= 0xDF) {
            i += 3;
        }
    }
    return true;
}

}  // namespace util
}  // namespace PropertyBasedTesting
