#include "../gen.hpp"
#include "../util/utf16string.hpp"
#include "../shrinker/stringlike.hpp"
#include "utf16string.hpp"
#include "util.hpp"
#include "integral.hpp"
#include "unicode.hpp"
#include "../util/std.hpp"

namespace proptest {

size_t Arbi<UTF16BEString>::defaultMinSize = 0;
size_t Arbi<UTF16BEString>::defaultMaxSize = 200;

Arbi<UTF16BEString>::Arbi() : ArbiContainer<UTF16BEString>(defaultMinSize, defaultMaxSize), elemGen(unicodeGen) {}

Arbi<UTF16BEString>::Arbi(Arbi<uint32_t>& _elemGen)
    : ArbiContainer<UTF16BEString>(defaultMinSize, defaultMaxSize),
      elemGen([_elemGen](Random& rand) mutable { return _elemGen(rand); })
{
}

Arbi<UTF16BEString>::Arbi(GenFunction<uint32_t> _elemGen)
    : ArbiContainer<UTF16BEString>(defaultMinSize, defaultMaxSize), elemGen(_elemGen)
{
}

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
Shrinkable<UTF16BEString> Arbi<UTF16BEString>::operator()(Random& rand)
{
    size_t len = rand.getRandomSize(minSize, maxSize + 1);
    vector<uint8_t> chars /*, allocator()*/;
    vector<int> positions /*, allocator()*/;
    vector<uint32_t> codes;

    chars.reserve(len * 4 + 2);
    codes.reserve(len);
    positions.reserve(len);

    // cout << "UTF16 BE gen, len = " << len << endl;

    for (size_t i = 0; i < len; i++) {
        // U+D800..U+DFFF is forbidden for surrogate use
        Shrinkable<uint32_t> codeShr = elemGen(rand);
        uint32_t code = codeShr.get();

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
                stringstream os;
                os << "invalid surrogate pairs: ";
                os << setfill('0') << setw(2) << hex << static_cast<int>(c0) << " ";
                os << setfill('0') << setw(2) << hex << static_cast<int>(c1) << " ";
                os << setfill('0') << setw(2) << hex << static_cast<int>(c2) << " ";
                os << setfill('0') << setw(2) << hex << static_cast<int>(c3) << " ";
                throw runtime_error(os.str());
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
        stringstream os;
        os << "not a valid UTF-16 BE string: ";
        for (size_t i = 0; i < chars.size(); i++) {
            os << static_cast<int>(chars[i]) << " ";
        }
        throw runtime_error(os.str());
    }

    UTF16BEString str(chars.size(), ' ' /*, allocator()*/);
    for (size_t i = 0; i < chars.size(); i++) {
        str[i] = chars[i];
    }

    return shrinkStringLike<UTF16BEString>(str, minSize, len, positions);
}

size_t Arbi<UTF16LEString>::defaultMinSize = 0;
size_t Arbi<UTF16LEString>::defaultMaxSize = 200;

Arbi<UTF16LEString>::Arbi() : ArbiContainer<UTF16LEString>(defaultMinSize, defaultMaxSize), elemGen(unicodeGen) {}

Arbi<UTF16LEString>::Arbi(Arbi<uint32_t>& _elemGen)
    : ArbiContainer<UTF16LEString>(defaultMinSize, defaultMaxSize),
      elemGen([_elemGen](Random& rand) mutable { return _elemGen(rand); })
{
}

Arbi<UTF16LEString>::Arbi(GenFunction<uint32_t> _elemGen)
    : ArbiContainer<UTF16LEString>(defaultMinSize, defaultMaxSize), elemGen(_elemGen)
{
}

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
Shrinkable<UTF16LEString> Arbi<UTF16LEString>::operator()(Random& rand)
{
    size_t len = rand.getRandomSize(minSize, maxSize + 1);
    vector<uint8_t> chars /*, allocator()*/;
    vector<int> positions /*, allocator()*/;
    vector<uint32_t> codes;

    chars.reserve(len * 4);
    codes.reserve(len);
    positions.reserve(len);

    // cout << "UTF16 LE gen, len = " << len << endl;

    for (size_t i = 0; i < len; i++) {
        // U+D800..U+DFFF is forbidden for surrogate use
        Shrinkable<uint32_t> codeShr = elemGen(rand);
        uint32_t code = codeShr.get();

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
                stringstream os;
                os << "invalid surrogate pairs: ";
                os << setfill('0') << setw(2) << hex << static_cast<int>(c1) << " ";
                os << setfill('0') << setw(2) << hex << static_cast<int>(c0) << " ";
                os << setfill('0') << setw(2) << hex << static_cast<int>(c3) << " ";
                os << setfill('0') << setw(2) << hex << static_cast<int>(c2) << " ";
                throw runtime_error(os.str());
                // throw runtime_error("invalid surrogate pairs: ");
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
        stringstream os;
        os << "not a valid UTF-16 LE string: ";
        printf("not a valid UTF-16 LE string: ");
        for (size_t i = 0; i < chars.size(); i++) {
            os << static_cast<int>(chars[i]) << " ";
            printf("%x ", chars[i]);
        }
        printf("\n");

        throw runtime_error(os.str());
    }

    // cout << "hex = {";
    // util::UTF16LEToHex(cout, chars);
    // cout << "}, decoded = \"";
    // util::decodeUTF16LE(cout, chars);
    // cout << "\"" << endl;

    UTF16LEString str(chars.size(), ' ' /*, allocator()*/);
    for (size_t i = 0; i < chars.size(); i++) {
        str[i] = chars[i];
    }

    return shrinkStringLike<UTF16LEString>(str, minSize, len, positions);
}

}  // namespace proptest
