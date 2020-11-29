#pragma once
#include <functional>
#include <type_traits>
#include "../Shrinkable.hpp"
#include "../Random.hpp"
#include "../gen.hpp"

namespace proptest {

template <typename T, typename LazyEval>
Generator<T> lazy(LazyEval&& lazyEval)
{
    auto lazyEvalPtr = std::make_shared<std::function<T()>>(std::forward<LazyEval>(lazyEval));
    return generator([lazyEvalPtr](Random&) { return make_shrinkable<T>((*lazyEvalPtr)()); });
}

template <typename LazyEval, typename T = typename std::result_of<LazyEval()>::type>
auto lazy(LazyEval&& lazyEval) -> Generator<T>
{
    return lazy<T>(std::forward<LazyEval>(lazyEval));
}

template <typename T, typename U = T>
Generator<T> just(U* valuePtr)
{
    std::shared_ptr<T> sharedPtr(valuePtr);
    return generator([sharedPtr](Random&) { return Shrinkable<T>(sharedPtr); });
}

template <typename T, typename U = T>
Generator<T> just(std::shared_ptr<T> sharedPtr)
{
    return generator([sharedPtr](Random&) { return Shrinkable<T>(sharedPtr); });
}

template <typename T>
std::enable_if_t<std::is_trivial<T>::value, Generator<T>> just(T&& value)
{
    return generator([value](Random&) { return make_shrinkable<T>(value); });
}

template <typename T>
std::enable_if_t<!std::is_trivial<T>::value, Generator<T>> just(const T& value)
{
    auto ptr = std::make_shared<T>(value);  // requires copy constructor
    return generator([ptr](Random&) { return Shrinkable<T>(ptr); });
}

}  // namespace proptest
