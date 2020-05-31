#include "../gen.hpp"
#include "../util/utf8string.hpp"
#include "../util/cesu8string.hpp"
#include "string.hpp"
#include "util.hpp"
#include "numeric.hpp"
#include <vector>
#include <iostream>
#include <ios>
#include <iomanip>
#include <sstream>

namespace PropertyBasedTesting {

size_t Arbitrary<std::string>::defaultMinSize = 0;
size_t Arbitrary<std::string>::defaultMaxSize = 200;

// defaults to ascii characters
Arbitrary<std::string>::Arbitrary() : elemGen(fromTo<char>(0x1, 0x7f)), minSize(defaultMinSize), maxSize(defaultMaxSize)
{
}

Arbitrary<std::string>::Arbitrary(Arbitrary<char>& _elemGen)
    : elemGen([_elemGen](Random& rand) mutable { return _elemGen(rand); }),
      minSize(defaultMinSize),
      maxSize(defaultMaxSize)
{
}

Arbitrary<std::string>::Arbitrary(std::function<Shrinkable<char>(Random&)> _elemGen)
    : elemGen(_elemGen), minSize(defaultMinSize), maxSize(defaultMaxSize)
{
}

Arbitrary<std::string> Arbitrary<std::string>::setMinSize(size_t size)
{
    minSize = size;
    return *this;
}

Arbitrary<std::string> Arbitrary<std::string>::setMaxSize(size_t size)
{
    maxSize = size;
    return *this;
}

Arbitrary<std::string> Arbitrary<std::string>::setSize(size_t size)
{
    minSize = size;
    maxSize = size;
    return *this;
}

Shrinkable<std::string> Arbitrary<std::string>::operator()(Random& rand)
{
    size_t size = rand.getRandomSize(minSize, maxSize + 1);
    std::string str(size, ' ' /*, allocator()*/);
    for (size_t i = 0; i < size; i++)
        str[i] = elemGen(rand).get();

    size_t minSizeCopy = minSize;
    return binarySearchShrinkable<size_t>(size - minSizeCopy)
        .transform<std::string>([str, minSizeCopy](const size_t& size) { return str.substr(0, size + minSizeCopy); });

    /*
    return make_shrinkable<std::string>(str).with([str]() -> stream_t {
        return stream_t::empty();
    });
    */
}

size_t Arbitrary<UTF8String>::defaultMinSize = 0;
size_t Arbitrary<UTF8String>::defaultMaxSize = 200;

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
 */
Shrinkable<UTF8String> Arbitrary<UTF8String>::operator()(Random& rand)
{
    size_t len = rand.getRandomSize(minSize, maxSize + 1);
    std::vector<uint8_t> chars /*, allocator()*/;
    std::vector<int> positions /*, allocator()*/;
    std::vector<uint32_t> codes;

    chars.reserve(len * 4);
    codes.reserve(len);
    positions.reserve(len);

    // std::cout << "utf8 gen, len = " << len << std::endl;

    for (size_t i = 0; i < len; i++) {
        // U+D800..U+DFFF is forbidden for surrogate use
        uint32_t code = rand.getRandomSize(1, 0x10FFFF - (0xDFFF - 0xD800 + 1) + 1);
        if (0xd800 <= code && code <= 0xdfff) {
            code = code + (0xdfff - 0xd800 + 1);
        }
        positions.push_back(chars.size());
        codes.push_back(code);

        if (code <= 0x7f) {
            chars.push_back(static_cast<uint8_t>(code));
        } else if (code <= 0x07FF) {
            code -= 0x80;
            uint8_t c0 = (code >> 6) + 0xc2;
            uint8_t c1 = (code & 0x3f) + 0x80;
            chars.push_back(c0);
            chars.push_back(c1);
        } else if (code <= 0x0FFF) {
            code -= 0x800;
            uint8_t c0 = 0xe0;
            uint8_t c1 = (code >> 6) + 0xa0;
            uint8_t c2 = (code & 0x3f) + 0x80;
            chars.push_back(c0);
            chars.push_back(c1);
            chars.push_back(c2);
        } else if (code <= 0xCFFF) {
            code -= 0x1000;
            uint8_t c0 = (code >> 12) + 0xe1;
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
            throw std::runtime_error("should not reach here. surrogate region");
        } else if (code <= 0xFFFF) {
            code -= 0xE000;
            uint8_t c0 = (code >> 12) + 0xee;
            uint8_t c1 = ((code >> 6) & 0x3f) + 0x80;
            uint8_t c2 = (code & 0x3f) + 0x80;
            chars.push_back(c0);
            chars.push_back(c1);
            chars.push_back(c2);
        } else if (code <= 0x3FFFF) {
            code -= 0x10000;
            uint8_t c0 = 0xf0;
            uint8_t c1 = (code >> 12) + 0x90;
            uint8_t c2 = ((code >> 6) & 0x3f) + 0x80;
            uint8_t c3 = (code & 0x3f) + 0x80;
            chars.push_back(c0);
            chars.push_back(c1);
            chars.push_back(c2);
            chars.push_back(c3);
        } else if (code <= 0xFFFFF) {
            code -= 0x40000;
            uint8_t c0 = (code >> 18) + 0xf1;
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
            uint8_t c1 = (code >> 12) + 0x80;
            uint8_t c2 = ((code >> 6) & 0x3f) + 0x80;
            uint8_t c3 = (code & 0x3f) + 0x80;
            chars.push_back(c0);
            chars.push_back(c1);
            chars.push_back(c2);
            chars.push_back(c3);

        } else {
            throw std::runtime_error("should not reach here. code too big");
        }
    }
    positions.push_back(chars.size());

    if (!util::isValidUTF8(chars)) {
        std::stringstream os;
        os << "not a valid UTF-8 string: ";
        printf("not a valid UTF-8 string: ");
        for (size_t i = 0; i < chars.size(); i++) {
            os << static_cast<int>(chars[i]) << " ";
            printf("%x ", chars[i]);
        }
        printf("\n");

        throw std::runtime_error(os.str());
    }

    // std::cout << "hex = {";
    // util::UTF8ToHex(std::cout, chars);
    // std::cout << "}, decoded = \"";
    // util::decodeUTF8(std::cout, chars);
    // std::cout << "\"" << std::endl;

    UTF8String str(chars.size(), ' ' /*, allocator()*/);
    for (size_t i = 0; i < chars.size(); i++) {
        str[i] = chars[i];
    }

    // substring shrinking
    size_t minSizeCopy = minSize;
    return binarySearchShrinkable<size_t>(len - minSizeCopy)
        .template transform<UTF8String>([str, minSizeCopy, positions](const size_t& size) -> UTF8String {
            if (positions.empty())
                return UTF8String();
            else
                return UTF8String(str.substr(0, positions[size + minSizeCopy]));
        });
}

size_t Arbitrary<UTF16BEString>::defaultMinSize = 0;
size_t Arbitrary<UTF16BEString>::defaultMaxSize = 200;

/*
 * legal UTF-16 byte sequence
 *
 *  Code Points        1st       2s       3s       4s
 * U+0000..U+D7FF     (as code point in LE/BE)
 *[U+D800..U+DBFF]    (high surrogates)
 *[U+DC00..U+DFFF]    (low surrogates)
 * U+E000..U+FFFF     (as code point in LE/BE)
 * U+10000..U+10FFFF: 4-byte surrogate pairs (U+D800..U+DBFF + U+DC00..U+DFFF)
 */
Shrinkable<UTF16BEString> Arbitrary<UTF16BEString>::operator()(Random& rand)
{
    size_t len = rand.getRandomSize(minSize, maxSize + 1);
    std::vector<uint8_t> chars /*, allocator()*/;
    std::vector<int> positions /*, allocator()*/;
    std::vector<uint32_t> codes;

    chars.reserve(len * 4 + 2);
    codes.reserve(len);
    positions.reserve(len);

    // std::cout << "UTF16 BE gen, len = " << len << std::endl;

    for (size_t i = 0; i < len; i++) {
        // U+D800..U+DFFF is forbidden for surrogate use
        uint32_t code = rand.getRandomSize(1, 0x10FFFF - (0xDFFF - 0xD800 + 1) + 1);
        if (0xd800 <= code && code <= 0xdfff) {
            code = code + (0xDFFF - 0xD800 + 1);
        }
        positions.push_back(chars.size());
        codes.push_back(code);

        if (code <= 0xd7FF || (0xE000 <= code && code <= 0xFFFF)) {
            uint8_t c0 = (code >> 8);
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
                std::stringstream os;
                os << "invalid surrogate pairs: ";
                os << std::setfill('0') << std::setw(2) << std::hex << static_cast<int>(c0) << " ";
                os << std::setfill('0') << std::setw(2) << std::hex << static_cast<int>(c1) << " ";
                os << std::setfill('0') << std::setw(2) << std::hex << static_cast<int>(c2) << " ";
                os << std::setfill('0') << std::setw(2) << std::hex << static_cast<int>(c3) << " ";
                throw std::runtime_error(os.str());
            }

            chars.push_back(c0);
            chars.push_back(c1);
            chars.push_back(c2);
            chars.push_back(c3);
        }
    }
    positions.push_back(chars.size());
    chars.push_back(0);
    chars.push_back(0);

    if (!util::isValidUTF16BE(chars)) {
        std::stringstream os;
        os << "not a valid UTF-16 BE string: ";
        for (size_t i = 0; i < chars.size(); i++) {
            os << static_cast<int>(chars[i]) << " ";
        }
        throw std::runtime_error(os.str());
    }

    UTF16BEString str(chars.size(), ' ' /*, allocator()*/);
    for (size_t i = 0; i < chars.size(); i++) {
        str[i] = chars[i];
    }

    // substring shrinking
    size_t minSizeCopy = minSize;
    return binarySearchShrinkable<size_t>(len - minSizeCopy)
        .template transform<UTF16BEString>([str, minSizeCopy, positions](const size_t& size) -> UTF16BEString {
            if (positions.empty())
                return UTF16BEString();
            else
                return UTF16BEString(str.substr(0, positions[size + minSizeCopy]));
        });
}

size_t Arbitrary<UTF16LEString>::defaultMinSize = 0;
size_t Arbitrary<UTF16LEString>::defaultMaxSize = 200;

/*
 * legal UTF-16 byte sequence
 *
 *  Code Points        1st       2s       3s       4s
 * U+0000..U+D7FF     (as code point in LE/BE)
 *[U+D800..U+DBFF]    (high surrogates)
 *[U+DC00..U+DFFF]    (low surrogates)
 * U+E000..U+FFFF     (as code point in LE/BE)
 * U+10000..U+10FFFF: 4-byte surrogate pairs (U+D800..U+DBFF + U+DC00..U+DFFF)
 */
Shrinkable<UTF16LEString> Arbitrary<UTF16LEString>::operator()(Random& rand)
{
    size_t len = rand.getRandomSize(minSize, maxSize + 1);
    std::vector<uint8_t> chars /*, allocator()*/;
    std::vector<int> positions /*, allocator()*/;
    std::vector<uint32_t> codes;

    chars.reserve(len * 4);
    codes.reserve(len);
    positions.reserve(len);

    // std::cout << "UTF16 LE gen, len = " << len << std::endl;

    for (size_t i = 0; i < len; i++) {
        // U+D800..U+DFFF is forbidden for surrogate use
        uint32_t code = rand.getRandomSize(1, 0x10FFFF - (0xDFFF - 0xD800 + 1) + 1);
        if (0xd800 <= code && code <= 0xdfff) {
            code = code + (0xdfff - 0xd800 + 1);
        }
        positions.push_back(chars.size());
        codes.push_back(code);

        if (code <= 0xd7ff || (0xE000 <= code && code <= 0xFFFF)) {
            uint8_t c0 = (code >> 8);
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
                std::stringstream os;
                os << "invalid surrogate pairs: ";
                os << std::setfill('0') << std::setw(2) << std::hex << static_cast<int>(c1) << " ";
                os << std::setfill('0') << std::setw(2) << std::hex << static_cast<int>(c0) << " ";
                os << std::setfill('0') << std::setw(2) << std::hex << static_cast<int>(c3) << " ";
                os << std::setfill('0') << std::setw(2) << std::hex << static_cast<int>(c2) << " ";
                throw std::runtime_error(os.str());
                // throw std::runtime_error("invalid surrogate pairs: ");
            }
            chars.push_back(c1);
            chars.push_back(c0);
            chars.push_back(c3);
            chars.push_back(c2);
        }
    }
    positions.push_back(chars.size());
    chars.push_back(0);
    chars.push_back(0);

    if (!util::isValidUTF16LE(chars)) {
        std::stringstream os;
        os << "not a valid UTF-16 LE string: ";
        printf("not a valid UTF-16 LE string: ");
        for (size_t i = 0; i < chars.size(); i++) {
            os << static_cast<int>(chars[i]) << " ";
            printf("%x ", chars[i]);
        }
        printf("\n");

        throw std::runtime_error(os.str());
    }

    // std::cout << "hex = {";
    // util::UTF16LEToHex(std::cout, chars);
    // std::cout << "}, decoded = \"";
    // util::decodeUTF16LE(std::cout, chars);
    // std::cout << "\"" << std::endl;

    UTF16LEString str(chars.size(), ' ' /*, allocator()*/);
    for (size_t i = 0; i < chars.size(); i++) {
        str[i] = chars[i];
    }

    // substring shrinking
    size_t minSizeCopy = minSize;
    return binarySearchShrinkable<size_t>(len - minSizeCopy)
        .template transform<UTF16LEString>([str, minSizeCopy, positions](const size_t& size) -> UTF16LEString {
            if (positions.empty())
                return UTF16LEString();
            else
                return UTF16LEString(str.substr(0, positions[size + minSizeCopy]));
        });
}

size_t Arbitrary<CESU8String>::defaultMinSize = 0;
size_t Arbitrary<CESU8String>::defaultMaxSize = 200;

/*
 * legal CESU-8 byte sequence
 *
 *  Code Points        1st       2s       3s
 * U+0000..U+007F     00..7F
 * U+0080..U+07FF     C2..DF   80..BF
 * U+0800..U+0FFF     E0       A0..BF   80..BF
 * U+1000..U+CFFF     E1..EC   80..BF   80..BF
 * U+D000..U+D7FF     ED       80..9F   80..BF
 *[U+D800..U+DBFF]    ED       A0..AF   80..BF (high surrogates)
 *[U+DC00..U+DFFF]    ED       B0..BF   80..BF (low surrogates)
 * U+E000..U+FFFF     EE..EF   80..BF   80..BF
 * U+10000..U+10FFFF: 6-byte surrogate pairs (U+D800..U+DBFF + U+DC00..U+DFFF)
 */
Shrinkable<CESU8String> Arbitrary<CESU8String>::operator()(Random& rand)
{
    size_t len = rand.getRandomSize(minSize, maxSize + 1);
    std::vector<uint8_t> chars /*, allocator()*/;
    std::vector<int> positions /*, allocator()*/;
    std::vector<uint32_t> codes;

    chars.reserve(len * 6);
    codes.reserve(len);
    positions.reserve(len);

    // std::cout << "cesu8 gen, len = " << len << std::endl;

    for (size_t i = 0; i < len; i++) {
        // U+D800..U+DFFF is forbidden for surrogate use
        uint32_t code = rand.getRandomSize(1, 0x10FFFF - (0xDFFF - 0xD800 + 1) + 1);
        if (0xd800 <= code && code <= 0xdfff) {
            code = code + (0xdfff - 0xd800 + 1);
        }
        positions.push_back(chars.size());
        codes.push_back(code);

        if (code <= 0x7f) {
            chars.push_back(static_cast<uint8_t>(code));
        } else if (code <= 0x07FF) {
            code -= 0x80;
            uint8_t c0 = (code >> 6) + 0xc2;
            uint8_t c1 = (code & 0x3f) + 0x80;
            chars.push_back(c0);
            chars.push_back(c1);
        } else if (code <= 0x0FFF) {
            code -= 0x800;
            uint8_t c0 = 0xe0;
            uint8_t c1 = (code >> 6) + 0xa0;
            uint8_t c2 = (code & 0x3f) + 0x80;
            chars.push_back(c0);
            chars.push_back(c1);
            chars.push_back(c2);
        } else if (code <= 0xCFFF) {
            code -= 0x1000;
            uint8_t c0 = (code >> 12) + 0xe1;
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
            throw std::runtime_error("should not reach here. surrogate region");
        } else if (code <= 0xFFFF) {
            code -= 0xE000;
            uint8_t c0 = (code >> 12) + 0xee;
            uint8_t c1 = ((code >> 6) & 0x3f) + 0x80;
            uint8_t c2 = (code & 0x3f) + 0x80;
            chars.push_back(c0);
            chars.push_back(c1);
            chars.push_back(c2);
        } else if (code <= 0x10FFFF) {
            code -= 0x10000;
            uint16_t surrogates[2] = {static_cast<uint16_t>(0xD800 + (code >> 10)),
                                      static_cast<uint16_t>(0xDC00 + (code & 0x3FF))};
            for (int i = 0; i < 2; i++) {
                code = surrogates[i];
                code -= (i == 0 ? 0xd800 : 0xdc00);
                uint8_t c0 = 0xed;
                uint8_t c1 = ((code >> 6) & 0x3f) + (i == 0 ? 0xa0 : 0xb0);
                uint8_t c2 = (code & 0x3f) + 0x80;
                chars.push_back(c0);
                chars.push_back(c1);
                chars.push_back(c2);
            }
        } else {
            throw std::runtime_error("should not reach here. code too big");
        }
    }
    positions.push_back(chars.size());

    if (!util::isValidCESU8(chars)) {
        std::stringstream os;
        os << "not a valid cesu8 string: ";
        printf("not a valid cesu8 string: ");
        for (size_t i = 0; i < chars.size(); i++) {
            os << std::setfill('0') << std::setw(2) << std::hex << static_cast<int>(chars[i]) << " ";
            printf("%x", chars[i]);
        }
        printf("\n");

        throw std::runtime_error(os.str());
    }

    // std::cout << "hex = {";
    // util::CESU8ToHex(std::cout, chars);
    // std::cout << "}, decoded = \"";
    // util::decodeCESU8(std::cout, chars);
    // std::cout << "\"" << std::endl;

    CESU8String str(chars.size(), ' ' /*, allocator()*/);
    for (size_t i = 0; i < chars.size(); i++) {
        str[i] = chars[i];
    }

    // str.substr(0, positions[len]);

    // substring shrinking
    size_t minSizeCopy = minSize;
    return binarySearchShrinkable<size_t>(len - minSizeCopy)
        .template transform<CESU8String>([str, minSizeCopy, positions](const size_t& size) -> CESU8String {
            if (positions.empty())
                return CESU8String();
            else
                return CESU8String(str.substr(0, positions[size + minSizeCopy]));
        });
}

}  // namespace PropertyBasedTesting
