#pragma once
#include "../util/std.hpp"
#include "../Shrinkable.hpp"
#include "../Random.hpp"
#include "../gen.hpp"

/**
 * @file just.hpp
 * @brief Generator combinator for generating just a specific value
 */

namespace proptest {

/**
 * @ingroup Combinators
 * @brief Generator combinator for generating just a specific value
 * @details Will always generate a specific value of type T. e.g. just(1339) will generate 1339
 */
template <typename T>
enable_if_t<is_trivial<T>::value, Generator<T>> just(T&& value)
{
    return generator([value](Random&) { return make_shrinkable<T>(value); });
}

/**
 * @ingroup Combinators
 * @brief Generator combinator for generating just a specific value
 * @details Will always generate a specific value of type T.
 */
template <typename T>
enable_if_t<!is_trivial<T>::value, Generator<T>> just(const T& value)
{
    auto ptr = util::make_shared<T>(value);  // requires copy constructor
    return generator([ptr](Random&) { return Shrinkable<T>(ptr); });
}

/**
 * @ingroup Combinators
 * @brief Generator combinator for generating just a specific value, given as raw pointer of type T
 * @details Will always generate a specific value of type T
 */
template <typename T, typename U>
enable_if_t<is_same<decay_t<T>, decay_t<U>>::value, Generator<T>> just(U* valuePtr)
{
    shared_ptr<T> sharedPtr(valuePtr);
    return generator([sharedPtr](Random&) { return Shrinkable<T>(sharedPtr); });
}

/**
 * @ingroup Combinators
 * @brief Generator combinator for generating specific value, given as shared_ptr<T>
 * @details Will always generate a specific value of type T, using a shared_ptr<T>. This can be useful if T is
 * non-copyable.
 */
template <typename T>
Generator<T> just(shared_ptr<T> sharedPtr)
{
    return generator([sharedPtr](Random&) { return Shrinkable<T>(sharedPtr); });
}

}  // namespace proptest
