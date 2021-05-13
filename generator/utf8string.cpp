#include "../gen.hpp"
#include "../util/utf8string.hpp"
#include "../shrinker/stringlike.hpp"
#include "utf8string.hpp"
#include "unicode.hpp"
#include "util.hpp"
#include "integral.hpp"
#include "../util/std.hpp"

namespace proptest {

size_t Arbi<UTF8String>::defaultMinSize = 0;
size_t Arbi<UTF8String>::defaultMaxSize = 200;

Arbi<UTF8String>::Arbi() : ArbiContainer<UTF8String>(defaultMinSize, defaultMaxSize), elemGen(UnicodeGen()) {}

Arbi<UTF8String>::Arbi(Arbi<uint32_t>& _elemGen)
    : ArbiContainer<UTF8String>(defaultMinSize, defaultMaxSize),
      elemGen([_elemGen](Random& rand) mutable { return _elemGen(rand); })
{
}

Arbi<UTF8String>::Arbi(GenFunction<uint32_t> _elemGen)
    : ArbiContainer<UTF8String>(defaultMinSize, defaultMaxSize), elemGen(_elemGen)
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
Shrinkable<UTF8String> Arbi<UTF8String>::operator()(Random& rand)
{
    size_t len = rand.getRandomSize(minSize, maxSize + 1);
    vector<uint8_t> chars /*, allocator()*/;
    vector<int> positions /*, allocator()*/;
    vector<uint32_t> codes;

    chars.reserve(len * 4);
    codes.reserve(len);
    positions.reserve(len);

    // cout << "utf8 gen, len = " << len << endl;

    for (size_t i = 0; i < len; i++) {
        // U+D800..U+DFFF is forbidden for surrogate use
        Shrinkable<uint32_t> codeShr = elemGen(rand);
        uint32_t code = codeShr.get();

        positions.push_back(chars.size());
        codes.push_back(code);

        util::encodeUTF8(code, chars);
    }
    positions.push_back(chars.size());

    if (!util::isValidUTF8(chars)) {
        stringstream os;
        os << "not a valid UTF-8 string: ";
        printf("not a valid UTF-8 string: ");
        for (size_t i = 0; i < chars.size(); i++) {
            os << static_cast<int>(chars[i]) << " ";
            printf("%x ", chars[i]);
        }
        printf("\n");

        throw runtime_error(os.str());
    }

    // cout << "hex = {";
    // util::UTF8ToHex(cout, chars);
    // cout << "}, decoded = \"";
    // util::decodeUTF8(cout, chars);
    // cout << "\"" << endl;

    UTF8String str(chars.size(), ' ' /*, allocator()*/);
    for (size_t i = 0; i < chars.size(); i++) {
        str[i] = chars[i];
    }

    return shrinkStringLike<UTF8String>(str, minSize, len, positions);
}

}  // namespace proptest
