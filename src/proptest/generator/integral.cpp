#include "proptest/gen.hpp"
#include "proptest/generator/integral.hpp"
#include "proptest/generator/bool.hpp"
#include "proptest/generator/util.hpp"
#include "proptest/util/std.hpp"


// for template instantiations
#define DEFINE_GENERATEINTEGER(TYPE) \
    template Shrinkable<TYPE> generateInteger(Random& rand, TYPE min, TYPE max)

#define DEFINE_NATURAL(TYPE) \
    template Generator<TYPE> natural(TYPE max)

#define DEFINE_NONNEGATIVE(TYPE) \
    template Generator<TYPE> nonNegative(TYPE max)

#define DEFINE_INTERVAL(TYPE) \
    template Generator<TYPE> interval(TYPE min, TYPE max)

#define DEFINE_INRANGE(TYPE) \
    template Generator<TYPE> inRange(TYPE from, TYPE to)

#define DEFINE_INTEGERS(TYPE) \
    template Generator<TYPE> integers(TYPE start, TYPE count)

#define DEFINE_FOR_ALL_INTTYPES(DEF) \
    DEF(int8_t);\
    DEF(int16_t);\
    DEF(int32_t);\
    DEF(int64_t);\
    DEF(uint8_t);\
    DEF(uint16_t);\
    DEF(uint32_t);\
    DEF(uint64_t)


namespace proptest {

DEFINE_FOR_ALL_INTTYPES(DEFINE_INTEGERS);

Shrinkable<char> Arbi<char>::operator()(Random& rand)
{
    return generateInteger<char>(rand);
}

Shrinkable<int8_t> Arbi<int8_t>::operator()(Random& rand)
{
    return generateInteger<int8_t>(rand);
}

Shrinkable<int16_t> Arbi<int16_t>::operator()(Random& rand)
{
    return generateInteger<int16_t>(rand);
}

Shrinkable<int32_t> Arbi<int32_t>::operator()(Random& rand)
{
    return generateInteger<int32_t>(rand);
}

Shrinkable<int64_t> Arbi<int64_t>::operator()(Random& rand)
{
    return generateInteger<int64_t>(rand);
}

Shrinkable<uint8_t> Arbi<uint8_t>::operator()(Random& rand)
{
    return generateInteger<uint8_t>(rand);
}

Shrinkable<uint16_t> Arbi<uint16_t>::operator()(Random& rand)
{
    return generateInteger<uint16_t>(rand);
}

Shrinkable<uint32_t> Arbi<uint32_t>::operator()(Random& rand)
{
    return generateInteger<uint32_t>(rand);
}

Shrinkable<uint64_t> Arbi<uint64_t>::operator()(Random& rand)
{
    return generateInteger<uint64_t>(rand);
}

// template instantiation
DEFINE_FOR_ALL_INTTYPES(DEFINE_GENERATEINTEGER);
DEFINE_FOR_ALL_INTTYPES(DEFINE_NATURAL);
DEFINE_FOR_ALL_INTTYPES(DEFINE_NONNEGATIVE);
DEFINE_FOR_ALL_INTTYPES(DEFINE_INTERVAL);
DEFINE_FOR_ALL_INTTYPES(DEFINE_INRANGE);


}  // namespace proptest
