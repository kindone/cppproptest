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

template <typename LazyEval>
auto lazy(LazyEval&& lazyEval) -> Generator<invoke_result_t<LazyEval>>
{
    return lazy<invoke_result_t<LazyEval>>(util::forward<LazyEval>(lazyEval));
}

}  // namespace proptest
