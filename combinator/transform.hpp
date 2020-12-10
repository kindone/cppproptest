#pragma once
#include "../util/std.hpp"
#include "../Random.hpp"
#include "../Shrinkable.hpp"
#include "../GenBase.hpp"

namespace proptest {

template <typename GEN>
decltype(auto) generator(GEN&& gen);
template <typename T>
struct Generator;

template <typename T, typename U>
Generator<U> transform(GenFunction<T> gen, function<U(T&)> transformer)
{
    auto genPtr = util::make_shared<decltype(gen)>(gen);
    auto transformerPtr = util::make_shared<function<U(const T&)>>(
        [transformer](const T& t) { return transformer(const_cast<T&>(t)); });
    return generator([genPtr, transformerPtr](Random& rand) {
        Shrinkable<T> shrinkable = (*genPtr)(rand);
        return shrinkable.map(transformerPtr);
    });
}

}  // namespace proptest
