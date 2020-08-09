#pragma once
#include "../gen.hpp"
#include "../api.hpp"
#include "../Stream.hpp"
#include "util.hpp"

namespace proptest {

template <typename GEN>
decltype(auto) generator(GEN&& gen);

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
    {
        return util::binarySearchShrinkableU(static_cast<T>(value - min)).template map<T>([min](const uint64_t& value) {
            return static_cast<T>(value + min);
        });
    } else if (max <= 0)  // [-5,-3] -> [-2,0] -> [-5,-3]
    {
        return util::binarySearchShrinkable(static_cast<T>(value - max)).template map<T>([max](const int64_t& value) {
            return static_cast<T>(value + max);
        });
    } else  // [-2, 2]
    {
        auto transformer = +[](const int64_t& value) { return static_cast<T>(value); };
        return util::binarySearchShrinkable(value).template map<T>(transformer);
    }
}

template <>
class PROPTEST_API Arbitrary<int8_t> final : public ArbitraryBase<int8_t> {
public:
    Shrinkable<int8_t> operator()(Random& rand) override;
    static constexpr int8_t boundaryValues[] = {INT8_MIN,     0,   INT8_MAX, -1,   1,    -2,   2,   INT8_MIN + 1,
                                                INT8_MAX - 1, ' ', '"',      '\'', '\t', '\n', '\r'};
};

template <>
class PROPTEST_API Arbitrary<int16_t> final : public ArbitraryBase<int16_t> {
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
struct PROPTEST_API Arbitrary<int32_t> final : public ArbitraryBase<int32_t>
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
struct PROPTEST_API Arbitrary<int64_t> final : public ArbitraryBase<int64_t>
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
class PROPTEST_API Arbitrary<char> final : public ArbitraryBase<char> {
public:
    Shrinkable<char> operator()(Random& rand) override;
    static constexpr char boundaryValues[] = {0};
};

template <>
class PROPTEST_API Arbitrary<uint8_t> final : public ArbitraryBase<uint8_t> {
public:
    Shrinkable<uint8_t> operator()(Random& rand) override;
    static constexpr uint8_t boundaryValues[] = {
        0, 1, 2, UINT8_MAX, UINT8_MAX - 1, INT8_MAX, INT8_MAX - 1, INT8_MAX + 1, ' ', '"', '\'', '\t', '\n', '\r'};
};

template <>
class PROPTEST_API Arbitrary<uint16_t> final : public ArbitraryBase<uint16_t> {
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
struct PROPTEST_API Arbitrary<uint32_t> final : public ArbitraryBase<uint32_t>
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
struct PROPTEST_API Arbitrary<uint64_t> : public ArbitraryBase<uint64_t>
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

/**
 * Generates a positive integer, excluding 0
 */
template <typename T>
Generator<T> natural(T max = std::numeric_limits<T>::max())
{
    return Generator<T>([max](Random& rand) { return generateInteger<T>(rand, 1, max); });
}

/**
 * Generates 0 or a positive integer
 */
template <typename T>
Generator<T> nonNegative(T max = std::numeric_limits<T>::max())
{
    return Generator<T>([max](Random& rand) { return generateInteger<T>(rand, 0, max); });
}

/**
 * Generates numeric values in [min, max]
 * e.g. interval(0,100) generates a value in {0, ..., 100}
 */
template <typename T>
Generator<T> interval(T min, T max)
{
    return Generator<T>([min, max](Random& rand) { return generateInteger<T>(rand, min, max); });
}

/**
 * Generates numeric values in [a, a+count)
 * e.g. integers(0,100) generates a value in {0, ..., 99}
 */
template <typename T>
Generator<T> integers(T start, T count)
{
    return Generator<T>(
        [start, count](Random& rand) { return generateInteger<T>(rand, start, static_cast<T>(start + count - 1)); });
}

}  // namespace proptest
