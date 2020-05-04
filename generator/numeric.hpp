#pragma once
#include "../gen.hpp"
#include "../Stream.hpp"
#include "util.hpp"

namespace PropertyBasedTesting {

template <typename T>
Shrinkable<T> generateInteger(Random& rand, T min = std::numeric_limits<T>::min(),
                              T max = std::numeric_limits<T>::max())
{
    T value = 0;
    if (min == std::numeric_limits<T>::min() && max == std::numeric_limits<T>::max() && rand.getRandomBool()) {
        uint32_t i =
            rand.getRandomSize(0, sizeof(Arbitrary<T>::boundaryValues) / sizeof(Arbitrary<T>::boundaryValues[0]));
        value = Arbitrary<T>::boundaryValues[i];
    } else if (std::numeric_limits<T>::min() < 0)
        value = rand.getRandom<T>(min, max);
    else
        value = rand.getRandomU<T>(min, max);

    if (value < min || max < value)
        throw std::runtime_error("invalid range");

    if (min >= 0)  // [3,5] -> [0,2] -> [3,5]
        return binarySearchShrinkable<T>(static_cast<T>(value - min)).template transform<T>([min](const T& value) {
            return value + min;
        });
    else if (max <= 0)  // [-5,-3] -> [-2,0] -> [-5,-3]
        return binarySearchShrinkable<T>(static_cast<T>(value - max)).template transform<T>([max](const T& value) {
            return value + max;
        });
    else  // [-2, 2]
        return binarySearchShrinkable<T>(value);
}

template <>
class PROPTEST_API Arbitrary<int8_t> final : public Gen<int8_t> {
public:
    Shrinkable<int8_t> operator()(Random& rand) override;
    static constexpr int8_t boundaryValues[] = {INT8_MIN,     0,   INT8_MAX, -1,   1,    -2,   2,   INT8_MIN + 1,
                                                INT8_MAX - 1, ' ', '"',      '\'', '\t', '\n', '\r'};
};

template <>
class PROPTEST_API Arbitrary<int16_t> final : public Gen<int16_t> {
public:
    Shrinkable<int16_t> operator()(Random& rand) override;
    static constexpr int16_t boundaryValues[] = {0,
                                                 -1,
                                                 1,
                                                 -2,
                                                 2,
                                                 INT16_MIN,
                                                 INT16_MAX,
                                                 INT16_MIN + 1,
                                                 INT16_MAX - 1,
                                                 INT8_MIN,
                                                 INT8_MAX,
                                                 UINT8_MAX,
                                                 INT8_MIN - 1,
                                                 INT8_MIN + 1,
                                                 INT8_MAX - 1,
                                                 INT8_MAX + 1,
                                                 UINT8_MAX - 1,
                                                 UINT8_MAX + 1};
};

template <>
struct PROPTEST_API Arbitrary<int32_t> final : public Gen<int32_t>
{
public:
    Shrinkable<int32_t> operator()(Random& rand) override;
    static constexpr int32_t boundaryValues[] = {0,
                                                 -1,
                                                 1,
                                                 -2,
                                                 2,
                                                 INT32_MIN,
                                                 INT32_MAX,
                                                 INT32_MIN + 1,
                                                 INT32_MAX - 1,
                                                 INT16_MIN,
                                                 INT16_MAX,
                                                 UINT16_MAX,
                                                 INT16_MIN - 1,
                                                 INT16_MIN + 1,
                                                 INT16_MAX - 1,
                                                 INT16_MAX + 1,
                                                 UINT16_MAX - 1,
                                                 UINT16_MAX + 1,
                                                 INT8_MIN,
                                                 INT8_MAX,
                                                 UINT8_MAX,
                                                 INT8_MIN - 1,
                                                 INT8_MIN + 1,
                                                 INT8_MAX - 1,
                                                 INT8_MAX + 1,
                                                 UINT8_MAX - 1,
                                                 UINT8_MAX + 1};
};

template <>
struct PROPTEST_API Arbitrary<int64_t> final : public Gen<int64_t>
{
public:
    Shrinkable<int64_t> operator()(Random& rand) override;
    static constexpr int64_t boundaryValues[] = {0,
                                                 -1,
                                                 1,
                                                 -2,
                                                 2,
                                                 INT64_MIN,
                                                 INT64_MAX,
                                                 INT64_MIN + 1,
                                                 INT64_MAX - 1,
                                                 INT32_MIN,
                                                 INT32_MAX,
                                                 UINT32_MAX,
                                                 static_cast<int64_t>(INT32_MIN) - 1,
                                                 INT32_MIN + 1,
                                                 INT32_MAX - 1,
                                                 static_cast<int64_t>(INT32_MAX) + 1,
                                                 UINT32_MAX - 1,
                                                 static_cast<int64_t>(UINT32_MAX) + 1,
                                                 INT16_MIN,
                                                 INT16_MAX,
                                                 UINT16_MAX,
                                                 INT16_MIN - 1,
                                                 INT16_MIN + 1,
                                                 INT16_MAX - 1,
                                                 INT16_MAX + 1,
                                                 UINT16_MAX - 1,
                                                 UINT16_MAX + 1,
                                                 INT8_MIN,
                                                 INT8_MAX,
                                                 UINT8_MAX,
                                                 INT8_MIN - 1,
                                                 INT8_MIN + 1,
                                                 INT8_MAX - 1,
                                                 INT8_MAX + 1,
                                                 UINT8_MAX - 1,
                                                 UINT8_MAX + 1};
};

template <>
class PROPTEST_API Arbitrary<char> final : public Gen<char> {
public:
    Shrinkable<char> operator()(Random& rand) override;
    static constexpr char boundaryValues[] = {0};
};

template <>
class PROPTEST_API Arbitrary<uint8_t> final : public Gen<uint8_t> {
public:
    Shrinkable<uint8_t> operator()(Random& rand) override;
    static constexpr uint8_t boundaryValues[] = {
        0, 1, 2, UINT8_MAX, UINT8_MAX - 1, INT8_MAX, INT8_MAX - 1, INT8_MAX + 1, ' ', '"', '\'', '\t', '\n', '\r'};
};

template <>
class PROPTEST_API Arbitrary<uint16_t> final : public Gen<uint16_t> {
public:
    Shrinkable<uint16_t> operator()(Random& rand) override;
    static constexpr uint16_t boundaryValues[] = {0,
                                                  1,
                                                  2,
                                                  UINT16_MAX,
                                                  UINT16_MAX - 1,
                                                  INT16_MAX,
                                                  INT16_MAX - 1,
                                                  INT16_MAX + 1,
                                                  INT8_MAX,
                                                  UINT8_MAX,
                                                  INT8_MAX + 1,
                                                  INT8_MAX - 1,
                                                  UINT8_MAX - 1,
                                                  UINT8_MAX + 1};
};

template <>
struct PROPTEST_API Arbitrary<uint32_t> final : public Gen<uint32_t>
{
public:
    Shrinkable<uint32_t> operator()(Random& rand) override;
    static constexpr uint32_t boundaryValues[] = {0,
                                                  1,
                                                  2,
                                                  UINT32_MAX,
                                                  UINT32_MAX - 1,
                                                  INT32_MAX,
                                                  INT32_MAX - 1,
                                                  static_cast<uint32_t>(INT32_MAX) + 1,
                                                  INT16_MAX,
                                                  UINT16_MAX,
                                                  INT16_MAX - 1,
                                                  INT16_MAX + 1,
                                                  UINT16_MAX - 1,
                                                  UINT16_MAX + 1,
                                                  INT8_MAX,
                                                  UINT8_MAX,
                                                  INT8_MAX + 1,
                                                  INT8_MAX - 1,
                                                  UINT8_MAX - 1,
                                                  UINT8_MAX + 1};
};

template <>
struct PROPTEST_API Arbitrary<uint64_t> : public Gen<uint64_t>
{
public:
    Shrinkable<uint64_t> operator()(Random& rand) override;
    static constexpr uint64_t boundaryValues[] = {0,
                                                  1,
                                                  2,
                                                  UINT64_MAX,
                                                  UINT64_MAX - 1,
                                                  INT32_MAX,
                                                  UINT32_MAX,
                                                  INT32_MAX - 1,
                                                  static_cast<int64_t>(INT32_MAX) + 1,
                                                  UINT32_MAX - 1,
                                                  static_cast<int64_t>(UINT32_MAX) + 1,
                                                  INT16_MAX,
                                                  UINT16_MAX,
                                                  INT16_MAX - 1,
                                                  INT16_MAX + 1,
                                                  UINT16_MAX - 1,
                                                  UINT16_MAX + 1,
                                                  INT8_MAX,
                                                  UINT8_MAX,
                                                  INT8_MAX + 1,
                                                  INT8_MAX - 1,
                                                  UINT8_MAX - 1,
                                                  UINT8_MAX + 1};
};

template <typename T>
std::function<Shrinkable<T>(Random& rand)> nonZero(T max = std::numeric_limits<T>::max())
{
    return [max](Random& rand) { return generateInteger<T>(rand, 1, max); };
}

template <typename T>
std::function<Shrinkable<T>(Random& rand)> nonNegative(T max = std::numeric_limits<T>::max())
{
    return [max](Random& rand) { return generateInteger<T>(rand, 0, max); };
}

// generates numeric in [a, b]
template <typename T>
std::function<Shrinkable<T>(Random& rand)> fromTo(T min, T max)
{
    return [min, max](Random& rand) { return generateInteger<T>(rand, min, max); };
}

// generates numeric in [a, b)
template <typename T>
std::function<Shrinkable<T>(Random& rand)> inRange(T fromInclusive, T toExclusive)
{
    return
        [fromInclusive, toExclusive](Random& rand) { return generateInteger<T>(rand, fromInclusive, static_cast<T>(toExclusive - 1)); };
}

template <>
struct PROPTEST_API Arbitrary<float> : public Gen<float>
{
public:
    Shrinkable<float> operator()(Random& rand) override;
    static constexpr float boundaryValues[] = {0.0, 1.0, -1.0};
};

template <>
struct PROPTEST_API Arbitrary<double> : public Gen<double>
{
public:
    Shrinkable<double> operator()(Random& rand) override;
    static constexpr double boundaryValues[] = {0.0, 1.0, -1.0};
};

}  // namespace PropertyBasedTesting
