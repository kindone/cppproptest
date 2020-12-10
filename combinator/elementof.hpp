#pragma once
#include "../util/std.hpp"

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
    WeightedValue(shared_ptr<T> _valuePtr, double _weight) : valuePtr(_valuePtr), weight(_weight) {}

    shared_ptr<T> valuePtr;
    double weight;
};

template <typename T>
enable_if_t<!is_same<decay_t<T>, WeightedValue<T>>::value, WeightedValue<T>> ValueToWeighted(T&& value)
{
    return weightedVal<T>(util::forward<T>(value), 0.0);
}

template <typename T>
WeightedValue<T> ValueToWeighted(WeightedValue<T>&& weighted)
{
    return util::forward<WeightedValue<T>>(weighted);
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
    shared_ptr<T> valuePtr = util::make_shared<T>(util::forward<Impl>(value));
    return util::WeightedValue<T>(valuePtr, weight);
}

// a value can be a raw Impl or a weightedVal(Impl, weight)
template <typename T, typename... Impl>
decltype(auto) elementOf(Impl&&... values)
{
    using WeightedValueVec = vector<util::WeightedValue<T>>;
    using WeightedVec = vector<util::Weighted<T>>;
    WeightedValueVec wvaluevec{util::ValueToWeighted<T>(util::forward<Impl>(values))...};

    auto genVecPtr = util::make_shared<WeightedVec>();

    transform(
        wvaluevec.begin(), wvaluevec.end(), back_inserter(*genVecPtr),
        +[](const util::WeightedValue<T>& wvalue) { return weightedGen<T>(just(wvalue.valuePtr), wvalue.weight); });

    return util::oneOfHelper<T>(genVecPtr);
}

}  // namespace proptest
