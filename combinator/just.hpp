#pragma once
#include <functional>
#include <type_traits>
#include "../Shrinkable.hpp"
#include "../Random.hpp"
#include "../gen.hpp"

namespace proptest {

template <typename T, typename LazyEval>
std::function<Shrinkable<T>(Random&)> just(LazyEval&& lazyEval)
{
    auto lazyEvalPtr = std::make_shared<std::function<T()>>(std::forward<LazyEval>(lazyEval));
    return CustomGen<T>([lazyEvalPtr](Random&) { return make_shrinkable<T>((*lazyEvalPtr)()); });
}

// template <typename T, typename...ARGS>
// std::function<Shrinkable<T>(Random&)> just2(ARGS&&... args) {
//     auto valuePtr = std::make_shared<T>(std::forward<ARGS>(args)...);
//     return [valuePtr](Random& rand) {
//         return make_shrinkable<T>(valuePtr);
//     };
// }

template <typename T, typename U = T>
std::function<Shrinkable<T>(Random&)> just(U* valuePtr)
{
    std::shared_ptr<T> sharedPtr(valuePtr);
    return CustomGen<T>([sharedPtr](Random&) { return make_shrinkable<T>(sharedPtr); });
}

}  // namespace proptest
