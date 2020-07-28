#pragma once
#include <functional>
#include <memory>
#include "../Random.hpp"
#include "../Shrinkable.hpp"

namespace proptest {

template <typename GEN>
decltype(auto) generator(GEN&& gen);
template <typename T>
struct Generator;

template <typename T, typename U>
Generator<U> transform(std::function<Shrinkable<T>(Random&)> gen, std::function<U(T&)> transformer)
{
    auto genPtr = std::make_shared<decltype(gen)>(gen);
    auto transformerPtr = std::make_shared<std::function<U(const T&)>>(
        [transformer](const T& t) { return transformer(const_cast<T&>(t)); });
    return generator([genPtr, transformerPtr](Random& rand) {
        Shrinkable<T> shrinkable = (*genPtr)(rand);
        return shrinkable.transform(transformerPtr);
    });
}

}  // namespace proptest
