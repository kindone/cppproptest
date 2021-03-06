#pragma once
#include "../util/std.hpp"
#include "../Shrinkable.hpp"
#include "../Random.hpp"
#include "../gen.hpp"

namespace proptest {

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
