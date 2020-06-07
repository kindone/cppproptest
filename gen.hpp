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
    Gen() {
        auto caller = [this](Random& rand) {
            return this->operator()(rand);
        };
        genPtr = std::make_shared<std::function<Shrinkable<T>(Random&)>>(caller);
    }

    Gen(std::function<Shrinkable<T>(Random&)> gen) : genPtr(std::make_shared<std::function<Shrinkable<T>(Random&)>>(gen)) {}
    virtual ~Gen() {}

    virtual Shrinkable<T> operator()(Random& rand) {
        if(!genPtr)
            throw std::runtime_error("operator() should be defined for Gen");
        else
            return (*genPtr)(rand);
    }

    template <typename U>
    decltype(auto) transform(std::function<U(const T&)> transformer)
    {
        return PropertyBasedTesting::transform<T, U>(static_cast<std::function<Shrinkable<T>(Random&)>>(*this), transformer);
    }

    template <typename Criteria>
    decltype(auto) filter(Criteria&& criteria)
    {
        return PropertyBasedTesting::filter<T>(static_cast<std::function<Shrinkable<T>(Random&)>>(*this), std::forward<Criteria>(criteria));
    }

    std::shared_ptr<std::function<Shrinkable<T>(Random&)>> genPtr;
};

template <typename T>
struct Arbitrary : public Gen<T>
{
};

}  // namespace PropertyBasedTesting

#include "util/invokeWithArgs.hpp"
#include "util/invokeWithGenTuple.hpp"
#include "util/createGenTuple.hpp"
