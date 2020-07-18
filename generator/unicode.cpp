#include "../Shrinkable.hpp"
#include "../combinator/oneof.hpp"
#include "../generator/integral.hpp"
#include "unicode.hpp"

namespace proptest {

Shrinkable<uint32_t> unicodeGen(Random& rand)
{
    auto frontSize = 0xD7FF - 0x1 + 1;
    auto rearSize = 0x10FFFF - 0xE000 + 1;
    auto size = frontSize + rearSize;
    auto frontGen = weighted<uint32_t>(fromTo<uint32_t>(0x1, 0xD7FF), static_cast<double>(frontSize) / size);
    auto rearGen = weighted<uint32_t>(fromTo<uint32_t>(0xE000, 0x10FFFF), static_cast<double>(rearSize) / size);
    static auto gen = oneOf<uint32_t>(frontGen, rearGen);
    return gen(rand);
}

}  // namespace proptest
