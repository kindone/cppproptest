#pragma once

#include "../util/std.hpp"
#include "../Shrinkable.hpp"
#include "../Random.hpp"
#include "../gen.hpp"

/**
 * @file lazy.hpp
 * @brief Generator combinator for generating a value of type T in lazy fashion
 */

namespace proptest {

/**
 * @ingroup Combinators
 * @brief Generator combinator for generating a value of type T by calling a function<T()>
 * @tparam T generated type from resultant generator
 * @tparam LazyEval callable type with signature void -> T
 * @details This is intended to support lazy evaluation for generating a constant value. It's similar to `just`
 * combinator, but it generates a value in lazily.
 */
template <typename T, typename LazyEval>
    requires(is_convertible_v<LazyEval&&, function<T()>>)
Generator<T> lazy(LazyEval&& lazyEval)
{
    auto lazyEvalPtr = util::make_shared<function<T()>>(util::forward<LazyEval>(lazyEval));
    return generator([lazyEvalPtr](Random&) { return make_shrinkable<T>((*lazyEvalPtr)()); });
}

/**
 * @ingroup Combinators
 * @brief Generator combinator for generating a value of type T by calling a callable void -> T
 * @tparam T (deduced) generated type from resultant generator
 * @tparam LazyEval callable type with signature void -> T
 * @details This is intended to support lazy evaluation for generating a constant value. It's similar to `just`
 * combinator, but it generates a value in lazily.
 * This overload is for deducted type T
 */
template <typename LazyEval>
auto lazy(LazyEval&& lazyEval) -> Generator<invoke_result_t<LazyEval>>
{
    return lazy<invoke_result_t<LazyEval>>(util::forward<LazyEval>(lazyEval));
}

}  // namespace proptest
