#pragma once
#include <vector>
#include <functional>

#include "../Random.hpp"
#include "../assert.hpp"
#include "../gen.hpp"
#include "oneof.hpp"
#include "just.hpp"

namespace proptest {

namespace util {
template <typename T>
struct WeightedValue;
}

template <typename Impl, typename T = Impl>
util::WeightedValue<T> weightedVal(Impl&& value, double weight);

namespace util {

template <typename T>
struct WeightedValue
{
    WeightedValue(std::shared_ptr<T> _valuePtr, double _weight) : valuePtr(_valuePtr), weight(_weight) {}

    std::shared_ptr<T> valuePtr;
    double weight;
};

template <typename T>
std::enable_if_t<!std::is_same<std::decay_t<T>, WeightedValue<T>>::value, WeightedValue<T>> ValueToWeighted(T&& value)
{
    return weightedVal<T>(std::forward<T>(value), 0.0);
}

template <typename T>
WeightedValue<T> ValueToWeighted(WeightedValue<T>&& weighted)
{
    return std::forward<WeightedValue<T>>(weighted);
}

template <typename T>
WeightedValue<T>& ValueToWeighted(WeightedValue<T>& weighted)
{
    return weighted;
}

}  // namespace util

template <typename Impl, typename T>
util::WeightedValue<T> weightedVal(Impl&& value, double weight)
{
    std::shared_ptr<T> valuePtr = std::make_shared<T>(std::forward<Impl>(value));
    return util::WeightedValue<T>(valuePtr, weight);
}

// a value can be a raw Impl or a weightedVal(Impl, weight)
template <typename T, typename... Impl>
decltype(auto) elementOf(Impl&&... values)
{
    using WeightedValueVec = std::vector<util::WeightedValue<T>>;
    using WeightedVec = std::vector<util::Weighted<T>>;
    WeightedValueVec wvaluevec{util::ValueToWeighted<T>(std::forward<Impl>(values))...};

    auto genVecPtr = std::make_shared<WeightedVec>();

    std::transform(
        wvaluevec.begin(), wvaluevec.end(), std::back_inserter(*genVecPtr),
        +[](const util::WeightedValue<T>& wvalue) { return weightedGen<T>(just(wvalue.valuePtr), wvalue.weight); });

    return util::oneOfHelper<T>(genVecPtr);
}

}  // namespace proptest
