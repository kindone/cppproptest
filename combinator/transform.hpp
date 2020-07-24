#pragma once
#include <functional>
#include <memory>
#include "../Random.hpp"
#include "../Shrinkable.hpp"

namespace proptest {

template <typename GEN>
decltype(auto) customGen(GEN&& gen);
template <typename T>
struct CustomGen;

template <typename T, typename U>
std::function<Shrinkable<U>(Random&)> transform(std::function<Shrinkable<T>(Random&)> gen,
                                                std::function<U(const T&)> transformer)
{
    auto genPtr = std::make_shared<decltype(gen)>(gen);
    auto transformerPtr = std::make_shared<decltype(transformer)>(transformer);
    return customGen([genPtr, transformerPtr](Random& rand) {
        Shrinkable<T> shrinkable = (*genPtr)(rand);
        return shrinkable.transform(transformerPtr);
    });
}

}  // namespace proptest
