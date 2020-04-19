#pragma once
#include <functional>
#include "testing/Shrinkable.hpp"
#include "testing/Random.hpp"

namespace PropertyBasedTesting {

template <typename T, typename LazyEval>
std::function<Shrinkable<T>(Random&)> just(LazyEval&& lazyEval) {
    auto lazyEvalPtr = std::make_shared<std::function<T()>>(lazyEval);
    return [lazyEvalPtr](Random& rand) {
        return make_shrinkable<T>((*lazyEvalPtr)());
    };
}

}