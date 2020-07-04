#include "../gen.hpp"
#include "../util/cesu8string.hpp"
#include "cesu8string.hpp"
#include "unicode.hpp"
#include "util.hpp"
#include "numeric.hpp"
#include <vector>
#include <iostream>
#include <ios>
#include <iomanip>
#include <sstream>

namespace PropertyBasedTesting {

size_t Arbitrary<CESU8String>::defaultMinSize = 0;
size_t Arbitrary<CESU8String>::defaultMaxSize = 200;

Arbitrary<CESU8String>::Arbitrary()
    : ArbitraryContainer<CESU8String>(defaultMinSize, defaultMaxSize), elemGen(unicodeGen)
{
}

Arbitrary<CESU8String>::Arbitrary(Arbitrary<uint32_t>& _elemGen)
    : ArbitraryContainer<CESU8String>(defaultMinSize, defaultMaxSize),
      elemGen([_elemGen](Random& rand) mutable { return _elemGen(rand); })
{
}

Arbitrary<CESU8String>::Arbitrary(std::function<Shrinkable<uint32_t>(Random&)> _elemGen)
    : ArbitraryContainer<CESU8String>(defaultMinSize, defaultMaxSize), elemGen(_elemGen)
{
}

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
    return binarySearchShrinkable(len - minSizeCopy)
        .template transform<CESU8String>([str, minSizeCopy, positions](const uint64_t& size) -> CESU8String {
            if (positions.empty())
                return CESU8String();
            else
                return CESU8String(str.substr(0, positions[size + minSizeCopy]));
        });
}

}  // namespace PropertyBasedTesting
