#include "testing/gen.hpp"
#include "testing/generator/string.hpp"
#include "testing/generator/util.hpp"
#include <vector>

namespace PropertyBasedTesting {


std::string Arbitrary<std::string>::boundaryValues[1] = {""};

Shrinkable<std::string> Arbitrary<std::string>::operator()(Random& rand) {
    if(rand.getRandomBool()) {
        size_t i = rand.getRandomSize(0, sizeof(boundaryValues) / sizeof(boundaryValues[0]));
        std::string str = std::string(boundaryValues[i]);
        int len = str.size();
        return binarySearchShrinkable<int>(len).transform<std::string>([str](const int& len) {
            return str.substr(0, len);
        });
    }
    else {
        int len = rand.getRandomSize(0, maxLen+1);
        std::string str(len, ' '/*, allocator()*/);
        for(int i = 0; i < len; i++)
            str[i] = rand.getRandomSize(0, 128);

        return binarySearchShrinkable<int>(len).transform<std::string>([str](const int& len) {
            return str.substr(0, len);
        });

    }


    /*
    return make_shrinkable<std::string>(str).with([str]() -> stream_t {
        return stream_t::empty();
    });
    */
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
std::string Arbitrary<UTF8String>::boundaryValues[1] = {""};

bool isValidUTF8(std::vector<uint8_t>& chars) {
    for(size_t i = 0; i < chars.size(); i++) {
        if(chars[i] <= 0x7f) {
            continue;
        }
        else if(i+1 >= chars.size()) {
            return false;
        }
        else if(0xc2 <= chars[i] && chars[i] <= 0xdf) {
            if(0x80 <= chars[i+1] && chars[i+1] <= 0xbf)
            {
                i++;
            }
            else
                return false;
        }
        else if(i+2 >= chars.size()) {
            return false;
        }
        else if(0xe0 == chars[i]) {
            if(0xa0 <= chars[i+1] && chars[i+1] <= 0xbf
                && 0x80 <= chars[i+2] && chars[i+2] <= 0xbf) {
                i+=2;
            }
            else
                return false;
        }
        else if(0xe1 <= chars[i] && chars[i] <= 0xec) {
            if(0x80 <= chars[i+1] && chars[i+1] <= 0xbf
                && 0x80 <= chars[i+2] && chars[i+2] <= 0xbf) {
                i+=2;
            }
            else
                return false;
        }
        else if(0xed == chars[i]) {
            if(0x80 <= chars[i+1] && chars[i+1] <= 0x9f
                && 0x80 <= chars[i+2] && chars[i+2] <= 0xbf) {
                i+=2;
            }
            else
                return false;
        }
        else if(0xee <= chars[i] && chars[i] <= 0xef) {
            if(0x80 <= chars[i+1] && chars[i+1] <= 0xbf
                && 0x80 <= chars[i+2] && chars[i+2] <= 0xbf) {
                i+=2;
            }
            else
                return false;
        }
        else if(i+3 >= chars.size()) {
            return false;
        }
        else if(0xf0 == chars[i]) {
            if(0x90 <= chars[i+1] && chars[i+1] <= 0xbf
                && 0x80 <= chars[i+2] && chars[i+2] <= 0xbf
                && 0x80 <= chars[i+3] && chars[i+3] <= 0xbf) {
                i+=3;
            }
            else
                return false;
        }
        else if(0xf1 <= chars[i] && chars[i] <= 0xf3) {
            if(0x80 <= chars[i+1] && chars[i+1] <= 0xbf
                && 0x80 <= chars[i+2] && chars[i+2] <= 0xbf
                && 0x80 <= chars[i+3] && chars[i+3] <= 0xbf) {
                i+=3;
            }
            else
                return false;

        }
        else if(0xf4 == chars[i]) {
            if(0x80 <= chars[i+1] && chars[i+1] <= 0xbf
                && 0x80 <= chars[i+2] && chars[i+2] <= 0xbf
                && 0x80 <= chars[i+3] && chars[i+3] <= 0xbf) {
                i+=3;
            }
            else
                return false;

        }
        else
            return false;
    }
    return true;
}

Shrinkable<UTF8String> Arbitrary<UTF8String>::operator()(Random& rand) {
    if(rand.getRandomBool()) {
        size_t i = rand.getRandomSize(0, sizeof(boundaryValues) / sizeof(boundaryValues[0]));
        return make_shrinkable<UTF8String>(UTF8String(boundaryValues[i]/*, allocator()*/));
    }

    int len = rand.getRandomSize(0, maxLen+1);
    std::vector<uint8_t> chars/*, allocator()*/;
    std::vector<uint8_t> nums/*, allocator()*/;
    chars.reserve(len*4);

    const int ranges[][2] = {{0, 0x7f}, {0xc2,0xdf}, {0xe0,0xe0}, {0xe1,0xec}, {0xed, 0xed}, {0xee,0xef}, {0xf0,0xf0}, {0xf1,0xf3}, {0xf4,0xf4}};
    const int numRanges = sizeof(ranges)/sizeof(ranges[0]);

    int numbers = 0;
    for(int i = 0 ; i < numRanges; i++) {
        numbers += (ranges[i][1] - ranges[i][0] + 1);
    }

    for(int i = 0; i < len; i++) {
        uint8_t n = rand.getRandomSize(0, numbers);
        nums.push_back(n);
        if(n <= 0x7f) {
            chars.push_back(0x0 + n);
            continue;
        }
        n -= (0x7f+1);
        if(n < (0xdf-0xc2+1)) {
            chars.push_back(0xc2 + n);
            chars.push_back(rand.getRandomSize(0x80, 0xbf+1));
            continue;
        }
        n -= (0xdf-0xc2+1);
        if(n < 1) {
            chars.push_back(0xe0);
            chars.push_back(rand.getRandomSize(0xa0, 0xbf+1));
            chars.push_back(rand.getRandomSize(0x80, 0xbf+1));
            continue;
        }
        n -= 1;
        if(n < (0xec-0xe1+1)) {
            chars.push_back(0xe1 + n);
            chars.push_back(rand.getRandomSize(0x80, 0xbf+1));
            chars.push_back(rand.getRandomSize(0x80, 0xbf+1));
            continue;
        }
        n -= (0xec-0xe1+1);
        if(n < 1) {
            chars.push_back(0xed);
            chars.push_back(rand.getRandomSize(0x80, 0x9f+1));
            chars.push_back(rand.getRandomSize(0x80, 0xbf+1));
            continue;
        }
        n -= 1;
        if(n < (0xef - 0xee + 1)) {
            chars.push_back(0xee + n);
            chars.push_back(rand.getRandomSize(0x80, 0xbf+1));
            chars.push_back(rand.getRandomSize(0x80, 0xbf+1));
            continue;
        }
        n -= (0xef - 0xee + 1);
        if(n < 1) {
            chars.push_back(0xf0);
            chars.push_back(rand.getRandomSize(0x90, 0xbf+1));
            chars.push_back(rand.getRandomSize(0x80, 0xbf+1));
            chars.push_back(rand.getRandomSize(0x80, 0xbf+1));
            continue;
        }
        n -= 1;
        if(n < (0xf3-0xf1+1)) {
            chars.push_back(0xf1 + n);
            chars.push_back(rand.getRandomSize(0x80, 0xbf+1));
            chars.push_back(rand.getRandomSize(0x80, 0xbf+1));
            chars.push_back(rand.getRandomSize(0x80, 0xbf+1));
            continue;
        }
        n -= (0xf3-0xf1+1);
        chars.push_back(0xf4);
        chars.push_back(rand.getRandomSize(0x80, 0xbf+1));
        chars.push_back(rand.getRandomSize(0x80, 0xbf+1));
        chars.push_back(rand.getRandomSize(0x80, 0xbf+1));
    }

    /*
    if(!isValidUTF8(chars)) {
        _STL::ostringstream os;
        os << "not a valid utf8 string: ";
        printf("not a valid utf8 string: ");
        for(size_t i = 0; i < chars.size(); i++) {
            os << static_cast<int>(chars[i]) << " ";
            printf("%x(%d) ", chars[i], nums[i]);
        }

        printf("\n");

        throw std::runtime_error(__FILE__, __LINE__, os.c_str());
    }
    */

    UTF8String str(chars.size(), ' '/*, allocator()*/);
    for(size_t i = 0; i < chars.size(); i++) {
        str[i] = chars[i];
    }

    return make_shrinkable<UTF8String>(str);
}

} // namespace PropertyBasedTesting
