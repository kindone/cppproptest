#pragma once
#include "../util/std.hpp"
#include "../Shrinkable.hpp"
#include "../Random.hpp"
#include "../gen.hpp"

namespace proptest {

template <typename T, typename LazyEval>
Generator<T> lazy(LazyEval&& lazyEval)
{
    auto lazyEvalPtr = util::make_shared<function<T()>>(util::forward<LazyEval>(lazyEval));
    return generator([lazyEvalPtr](Random&) { return make_shrinkable<T>((*lazyEvalPtr)()); });
}

template <typename LazyEval, typename T = typename result_of<LazyEval()>::type>
auto lazy(LazyEval&& lazyEval) -> Generator<T>
{
    return lazy<T>(util::forward<LazyEval>(lazyEval));
}

template <typename T, typename U = T>
Generator<T> just(U* valuePtr)
{
    shared_ptr<T> sharedPtr(valuePtr);
    return generator([sharedPtr](Random&) { return Shrinkable<T>(sharedPtr); });
}

template <typename T, typename U = T>
Generator<T> just(shared_ptr<T> sharedPtr)
{
    return generator([sharedPtr](Random&) { return Shrinkable<T>(sharedPtr); });
}

template <typename T>
enable_if_t<is_trivial<T>::value, Generator<T>> just(T&& value)
{
    return generator([value](Random&) { return make_shrinkable<T>(value); });
}

template <typename T>
enable_if_t<!is_trivial<T>::value, Generator<T>> just(const T& value)
{
    auto ptr = util::make_shared<T>(value);  // requires copy constructor
    return generator([ptr](Random&) { return Shrinkable<T>(ptr); });
}

}  // namespace proptest
