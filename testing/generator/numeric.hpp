#ifndef __PROPTEST_NUMRIC_HPP__
#define __PROPTEST_NUMRIC_HPP__
#include "testing/gen.hpp"
#include "testing/Seq.hpp"
#include "testing/Stream.hpp"

namespace PropertyBasedTesting
{

template <>
class PROPTEST_API Arbitrary<int8_t> : public Gen<int8_t>
{
public:
    Shrinkable<int8_t> generate(Random& rand);
    static constexpr int8_t boundaryValues[] = {INT8_MIN, 0, INT8_MAX, -1, 1, -2, 2, INT8_MIN+1, INT8_MAX-1, ' ', '"',  '\'',  '\t', '\n', '\r'};

    //Seq<Shrinkable<int8_t>> shrinks(Shrinkable<int8_t>& target);
};


template <>
class PROPTEST_API Arbitrary<int16_t> : public Gen<int16_t>
{
public:
    int16_t generate(Random& rand);
    static constexpr int16_t boundaryValues[] = {0, -1, 1, -2, 2,
        INT16_MIN, INT16_MAX,
        INT16_MIN+1, INT16_MAX-1,
        INT8_MIN, INT8_MAX, UINT8_MAX,
        INT8_MIN-1, INT8_MIN+1, INT8_MAX-1, INT8_MAX+1, UINT8_MAX-1, UINT8_MAX+1};
};

template <>
struct PROPTEST_API Arbitrary<int32_t> : public Gen<int32_t>
{
public:
    int32_t generate(Random& rand);
    static constexpr int32_t boundaryValues[] = {0, -1, 1, -2, 2,
        INT32_MIN, INT32_MAX,
        INT32_MIN+1, INT32_MAX-1,
        INT16_MIN, INT16_MAX, UINT16_MAX,
        INT16_MIN-1, INT16_MIN+1, INT16_MAX-1, INT16_MAX+1, UINT16_MAX-1, UINT16_MAX+1,
        INT8_MIN, INT8_MAX, UINT8_MAX,
        INT8_MIN-1, INT8_MIN+1, INT8_MAX-1, INT8_MAX+1, UINT8_MAX-1, UINT8_MAX+1};
};

template <>
struct PROPTEST_API Arbitrary<int64_t> : public Gen<int64_t>
{
public:
    int64_t generate(Random& rand);
    static constexpr int64_t boundaryValues[] = { 0, -1, 1, -2, 2,
        INT64_MIN, INT64_MAX,
        INT64_MIN+1, INT64_MAX-1,
        INT32_MIN, INT32_MAX, UINT32_MAX,
        static_cast<int64_t>(INT32_MIN)-1, INT32_MIN+1, INT32_MAX-1, static_cast<int64_t>(INT32_MAX)+1, UINT32_MAX-1, static_cast<int64_t>(UINT32_MAX)+1,
        INT16_MIN, INT16_MAX, UINT16_MAX,
        INT16_MIN-1, INT16_MIN+1, INT16_MAX-1, INT16_MAX+1, UINT16_MAX-1, UINT16_MAX+1,
        INT8_MIN, INT8_MAX, UINT8_MAX,
        INT8_MIN-1, INT8_MIN+1, INT8_MAX-1, INT8_MAX+1, UINT8_MAX-1, UINT8_MAX+1};
};

template <>
class PROPTEST_API Arbitrary<uint8_t> : public Gen<uint8_t>
{
public:
    uint8_t generate(Random& rand);
    static constexpr uint8_t boundaryValues[] = {0, 1, 2,
        UINT8_MAX, UINT8_MAX-1, INT8_MAX, INT8_MAX-1, INT8_MAX+1,
        ' ',  '"',  '\'',  '\t', '\n', '\r'};
};

template <>
class PROPTEST_API Arbitrary<uint16_t> : public Gen<uint16_t>
{
public:
    uint16_t generate(Random& rand);
    static constexpr uint16_t boundaryValues[] = {0, 1, 2,
        UINT16_MAX,
        UINT16_MAX-1,
        INT16_MAX, INT16_MAX-1, INT16_MAX+1,
        INT8_MAX, UINT8_MAX,
        INT8_MAX+1, INT8_MAX-1, UINT8_MAX-1, UINT8_MAX+1};
};

template <>
struct PROPTEST_API Arbitrary<uint32_t> : public Gen<uint32_t>
{
public:
    uint32_t generate(Random& rand);
    static constexpr uint32_t boundaryValues[] = {0, 1, 2,
        UINT32_MAX,
        UINT32_MAX-1,
        INT32_MAX, INT32_MAX-1, static_cast<uint32_t>(INT32_MAX)+1,
        INT16_MAX, UINT16_MAX,
        INT16_MAX-1, INT16_MAX+1, UINT16_MAX-1, UINT16_MAX+1,
        INT8_MAX, UINT8_MAX,
        INT8_MAX+1, INT8_MAX-1, UINT8_MAX-1, UINT8_MAX+1};

};

template <>
struct PROPTEST_API Arbitrary<uint64_t> : public Gen<uint64_t>
{
public:
    uint64_t generate(Random& rand);
    static constexpr uint64_t boundaryValues[] = {0, 1, 2,
        UINT64_MAX,
        UINT64_MAX-1,
        INT32_MAX, UINT32_MAX,
        INT32_MAX-1, static_cast<int64_t>(INT32_MAX)+1, UINT32_MAX-1, static_cast<int64_t>(UINT32_MAX)+1,
        INT16_MAX, UINT16_MAX,
        INT16_MAX-1, INT16_MAX+1, UINT16_MAX-1, UINT16_MAX+1,
        INT8_MAX, UINT8_MAX,
        INT8_MAX+1, INT8_MAX-1, UINT8_MAX-1, UINT8_MAX+1};

};

template <>
struct PROPTEST_API Arbitrary<float> : public Gen<float>
{
public:
    float generate(Random& rand);
    static constexpr float boundaryValues[] = {0.0, 1.0, -1.0};

};
template <>
struct PROPTEST_API Arbitrary<double> : public Gen<double>
{
public:
    double generate(Random& rand);
    static constexpr double boundaryValues[] = {0.0, 1.0, -1.0};

};


} // namespace PropertyBasedTesting

#endif
