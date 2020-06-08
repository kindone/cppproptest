#pragma once

#include <functional>
#include <tuple>
#include <stdexcept>
#include "util/function_traits.hpp"
#include "util/typelist.hpp"
#include "api.hpp"
#include "Random.hpp"
#include "Shrinkable.hpp"
#include "assert.hpp"
#include "combinator/transform.hpp"
#include "combinator/filter.hpp"

namespace PropertyBasedTesting {

class Random;

template <typename T>
struct Gen
{
    using type = T;

    virtual ~Gen() {}

    virtual Shrinkable<T> operator()(Random& rand) {
        throw std::runtime_error("operator() should be defined for Gen");
    }
};

template <typename T>
struct CustomGen : public Gen<T> {

    CustomGen(std::function<Shrinkable<T>(Random&)> gen) : genPtr(std::make_shared<std::function<Shrinkable<T>(Random&)>>(gen)) {}

    virtual Shrinkable<T> operator()(Random& rand) {
        return (*genPtr)(rand);
    }

    template <typename U>
    CustomGen<U> transform(std::function<U(const T&)> transformer)
    {
        auto thisPtr = clone();
        return CustomGen<U>(PropertyBasedTesting::transform<T, U>([thisPtr](Random& rand) {
            return (*thisPtr->genPtr)(rand);
        }, transformer));
    }

    template <typename Criteria>
    CustomGen<T> filter(Criteria&& criteria)
    {
        auto thisPtr = clone();
        return CustomGen<T>(PropertyBasedTesting::filter<T>([thisPtr](Random& rand) {
            return (*thisPtr->genPtr)(rand);
        }, std::forward<Criteria>(criteria)));
    }

    std::shared_ptr<CustomGen<T>> clone() {
        return std::make_shared<CustomGen<T>>(*dynamic_cast<CustomGen<T>*>(this));
    }

    std::shared_ptr<std::function<Shrinkable<T>(Random&)>> genPtr;
};

template <typename T> struct Arbitrary;

template <typename T>
struct ArbitraryBase : public Gen<T> {

    template <typename U>
    CustomGen<U> transform(std::function<U(const T&)> transformer)
    {
        auto thisPtr = clone();
        return CustomGen<U>(PropertyBasedTesting::transform<T, U>([thisPtr](Random& rand) {
            return thisPtr->operator()(rand);
        }, transformer));
    }

    template <typename Criteria>
    CustomGen<T> filter(Criteria&& criteria)
    {
        auto thisPtr = clone();
        return CustomGen<T>(PropertyBasedTesting::filter<T>([thisPtr](Random& rand) {
            return thisPtr->operator()(rand);
        }, std::forward<Criteria>(criteria)));
    }

    std::shared_ptr<Arbitrary<T>> clone() {
        return std::make_shared<Arbitrary<T>>(*dynamic_cast<Arbitrary<T>*>(this));
    }
};

template <typename T>
struct Arbitrary : public ArbitraryBase<T>
{
};

}  // namespace PropertyBasedTesting

#include "util/invokeWithArgs.hpp"
#include "util/invokeWithGenTuple.hpp"
#include "util/createGenTuple.hpp"
