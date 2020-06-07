#pragma once
#include <functional>
#include <memory>
#include "../Random.hpp"
#include "../Shrinkable.hpp"

namespace PropertyBasedTesting {

template <typename T, typename U>
decltype(auto) transform(std::function<Shrinkable<T>(Random&)> gen, std::function<U(const T&)> transformer)
{
    auto genPtr = std::make_shared<decltype(gen)>(gen);
    auto transformerPtr = std::make_shared<decltype(transformer)>(transformer);
    return [genPtr, transformerPtr](Random& rand) {
        Shrinkable<T> shrinkable = (*genPtr)(rand);
        return shrinkable.transform(transformerPtr);
    };
}

}  // namespace PropertyBasedTesting
