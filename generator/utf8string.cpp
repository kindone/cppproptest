#include "../gen.hpp"
#include "../util/utf8string.hpp"
#include "utf8string.hpp"
#include "unicode.hpp"
#include "util.hpp"
#include "integral.hpp"
#include <vector>
#include <iostream>
#include <ios>
#include <iomanip>
#include <sstream>

namespace proptest {

size_t Arbitrary<UTF8String>::defaultMinSize = 0;
size_t Arbitrary<UTF8String>::defaultMaxSize = 200;

Arbitrary<UTF8String>::Arbitrary() : ArbitraryContainer<UTF8String>(defaultMinSize, defaultMaxSize), elemGen(unicodeGen)
{
}

Arbitrary<UTF8String>::Arbitrary(Arbitrary<uint32_t>& _elemGen)
    : ArbitraryContainer<UTF8String>(defaultMinSize, defaultMaxSize),
      elemGen([_elemGen](Random& rand) mutable { return _elemGen(rand); })
{
}

Arbitrary<UTF8String>::Arbitrary(GenFunction<uint32_t> _elemGen)
    : ArbitraryContainer<UTF8String>(defaultMinSize, defaultMaxSize), elemGen(_elemGen)
{
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
        Shrinkable<uint32_t> codeShr = elemGen(rand);
        uint32_t code = codeShr.get();

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
    return util::binarySearchShrinkableU(len - minSizeCopy)
        .template transform<UTF8String>([str, minSizeCopy, positions](const uint64_t& size) -> UTF8String {
            if (positions.empty())
                return UTF8String();
            else
                return UTF8String(str.substr(0, positions[size + minSizeCopy]));
        });
}

}  // namespace proptest
