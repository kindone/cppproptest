#pragma once
#include <memory>
#include <utility>
#include <type_traits>
#include <functional>
#include "typelist.hpp"

namespace proptest {

template <class F>
struct function_traits;

template <class R, class... Args>
struct function_traits<R (*)(Args...)> : public function_traits<R(Args...)>
{
};

template <class R, class... Args>
struct function_traits<R(Args...)>
{
    using return_type = R;
    static constexpr std::size_t arity = sizeof...(Args);
    using argument_type_list = util::TypeList<Args...>;

    template <std::size_t N>
    struct argument
    {
        static_assert(N < arity, "error: invalid parameter index.");
        using type = typename std::tuple_element<N, std::tuple<Args...>>::type;
    };
};

// member function pointer
template <class C, class R, class... Args>
struct function_traits<R (C::*)(Args...)> : public function_traits<R(C&, Args...)>
{
};

// const member function pointer
template <class C, class R, class... Args>
struct function_traits<R (C::*)(Args...) const> : public function_traits<R(C&, Args...)>
{
};

// member object pointer
template <class C, class R>
struct function_traits<R(C::*)> : public function_traits<R(C&)>
{
};

// functor
template <class F>
struct function_traits
{
private:
    using call_type = function_traits<decltype(&F::operator())>;
    using full_argument_type_list = typename call_type::argument_type_list;

public:
    static constexpr std::size_t arity = call_type::arity - 1;
    using return_type = typename call_type::return_type;
    using argument_type_list = typename full_argument_type_list::Tail;

    template <std::size_t N>
    struct argument
    {
        static_assert(N < arity, "error: invalid parameter index.");
        using type = typename call_type::template argument<N + 1>::type;
    };
};

template <class F>
struct function_traits<F&> : public function_traits<F>
{
};

template <class F>
struct function_traits<F&&> : public function_traits<F>
{
};

}  // namespace proptest
