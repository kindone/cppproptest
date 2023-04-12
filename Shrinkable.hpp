#pragma once

#include "api.hpp"
#include "Stream.hpp"
#include "util/any.hpp"
#include "util/std.hpp"

namespace proptest {

struct ShrinkableAny;

template <typename T>
struct Shrinkable;

template <typename T, typename... Args>
Shrinkable<T> make_shrinkable(Args&&... args);

template <typename T, typename... Args>
ShrinkableAny make_shrinkable_any(Args&&... args);

template <typename T, typename... Args>
Any make_anything(Args&&... args)
{
    Any any;
    any.ptr = static_pointer_cast<void>(util::make_shared<T>(args...));
    return any;
}

namespace util {

struct AnyFunction
{
    using F = function<Any(const Any&)>;

    template <typename T, typename U>
    AnyFunction(function<U(const T&)> inFunc);

    Any operator()(const Any& in);

    shared_ptr<F> funcPtr;
};

struct ShrinkableAnyFunction
{
    using F = function<ShrinkableAny(const Any&)>;

    template <typename T, typename U>
    ShrinkableAnyFunction(function<Shrinkable<U>(const T&)> inFunc);

    ShrinkableAny operator()(const Any& in);

    shared_ptr<F> funcPtr;
};

struct ShrinkableAnyFunction1
{
    using F = function<ShrinkableAny(const ShrinkableAny&)>;

    template <typename T, typename U>
    ShrinkableAnyFunction1(function<Shrinkable<U>(const Shrinkable<T>&)> inFunc);

    ShrinkableAny operator()(const ShrinkableAny& in);

    shared_ptr<F> funcPtr;
};

struct BoolFunction
{
    using F = function<bool(const Any&)>;

    template <typename T>
    BoolFunction(function<bool(const T&)> inFunc);

    bool operator()(const Any& in);

    shared_ptr<F> funcPtr;
};

struct StreamFunction
{
    using F = function<Stream(const ShrinkableAny&)>;

    template <typename T>
    StreamFunction(function<Stream(const Shrinkable<T>&)> inFunc);

    Stream operator()(const ShrinkableAny& in);

    shared_ptr<F> funcPtr;
};

}

struct PROPTEST_API ShrinkableAny
{
    ShrinkableAny(shared_ptr<Any> p);
    ShrinkableAny(const ShrinkableAny& other);
    virtual ~ShrinkableAny() = default;

    ShrinkableAny& operator=(const ShrinkableAny& other);

    ShrinkableAny with(function<Stream()> _shrinks) const;

    ShrinkableAny with(shared_ptr<function<Stream()>> newShrinksPtr) const;

    Any& getAnyRef() const;

    ShrinkableAny map(util::AnyFunction transformer) const;

    ShrinkableAny map(shared_ptr<util::AnyFunction> transformerPtr) const;

    ShrinkableAny map(shared_ptr<function<Any(const Any&)>> transformerPtr) const;

    ShrinkableAny flatMap(util::ShrinkableAnyFunction transformer) const;

    ShrinkableAny flatMap(shared_ptr<util::ShrinkableAnyFunction> transformerPtr) const;

    ShrinkableAny mapShrinkable(util::ShrinkableAnyFunction1 transformer) const;

    ShrinkableAny mapShrinkable(
        shared_ptr<util::ShrinkableAnyFunction1> transformerPtr) const;

    // provide filtered generation, shrinking
    ShrinkableAny filter(util::BoolFunction criteria) const;

    ShrinkableAny filter(shared_ptr<util::BoolFunction> criteriaPtr) const;

    // provide filtered generation, shrinking
    ShrinkableAny filter(util::BoolFunction criteria, int tolerance) const;

    ShrinkableAny filter(shared_ptr<util::BoolFunction> criteriaPtr, int tolerance) const;

    ShrinkableAny filter(shared_ptr<function<bool(const Any&)>> criteriaPtr, int tolerance) const;

    // concat: continues with then after horizontal dead end
    ShrinkableAny concatStatic(function<Stream()> then) const;

    ShrinkableAny concatStatic(shared_ptr<function<Stream()>> thenPtr) const;

    // concat: extend shrinks stream with function taking parent as argument
    ShrinkableAny concat(util::StreamFunction then) const;

    ShrinkableAny concat(shared_ptr<util::StreamFunction> thenPtr) const;

    // andThen: continues with then after vertical dead end
    ShrinkableAny andThenStatic(function<Stream()> then) const;

    ShrinkableAny andThenStatic(shared_ptr<function<Stream()>> thenPtr) const;

    ShrinkableAny andThen(util::StreamFunction then) const;

    ShrinkableAny andThen(shared_ptr<util::StreamFunction> thenPtr) const;

    ShrinkableAny take(int n) const;

    template <typename T>
    Shrinkable<T> as() const;

public:
    shared_ptr<Any> ptr;
    shared_ptr<function<Stream()>> shrinksPtr;

protected:

    ShrinkableAny(shared_ptr<Any> p, shared_ptr<function<Stream()>> s);

    static shared_ptr<function<Stream()>> emptyPtr();

public:
    Stream shrinks() const;

    template <typename U, typename... Args>
    friend ShrinkableAny make_shrinkable_any(Args&&... args);
};


template <typename T>
struct Shrinkable : public ShrinkableAny
{
    using type = T;

    Shrinkable(shared_ptr<Any> p) : ShrinkableAny(p) {}
    Shrinkable(const ShrinkableAny& other) : ShrinkableAny(other) {}

    Shrinkable& operator=(const Shrinkable& other)
    {
        ptr = other.ptr;
        shrinksPtr = other.shrinksPtr;
        return *this;
    }

    // operator T() const { return get(); }
    T get() const { return *static_pointer_cast<T>(ptr->ptr); }
    T* getPtr() const { return static_pointer_cast<T>(ptr->ptr).get(); }
    T& getRef() const { return *static_pointer_cast<T>(ptr->ptr).get(); }
    shared_ptr<T> getSharedPtr() const { return static_pointer_cast<T>(ptr->ptr); }

    template <typename U>
    Shrinkable<U> map(function<U(const T&)> transformer) const
    {
        return Shrinkable<U>(ShrinkableAny::map(transformer));
    }

    template <typename U>
    Shrinkable<U> map(shared_ptr<function<Any(const Any&)>> transformer) const
    {
        return Shrinkable<U>(ShrinkableAny::map(transformer));
    }

    template <typename U>
    Shrinkable<U> flatMap(function<Shrinkable<U>(const T&)> transformer) const
    {
        return Shrinkable<U>(ShrinkableAny::flatMap(transformer));
    }

    template <typename U>
    Shrinkable<U> mapShrinkable(function<Shrinkable<U>(const Shrinkable<T>&)> transformer) const
    {
        return Shrinkable<U>(ShrinkableAny::mapShrinkable(transformer));
    }

    // provide filtered generation, shrinking
    Shrinkable<T> filter(function<bool(const T&)> criteria) const
    {
        return Shrinkable(ShrinkableAny::filter(criteria));
    }

    // provide filtered generation, shrinking
    Shrinkable<T> filter(function<bool(const T&)> criteria, int tolerance) const
    {
        return Shrinkable(ShrinkableAny::filter(criteria, tolerance));
    }

    Shrinkable<T> filter(shared_ptr<function<bool(const Any&)>> criteriaPtr, int tolerance) const
    {
        return Shrinkable(ShrinkableAny::filter(criteriaPtr, tolerance));
    }

    // concat: continues with then after horizontal dead end
    Shrinkable<T> concatStatic(function<Stream()> then) const
    {
        return Shrinkable(ShrinkableAny::concatStatic(then));
    }

    // concat: extend shrinks stream with function taking parent as argument
    Shrinkable<T> concat(function<Stream(const Shrinkable<T>&)> then) const
    {
        return Shrinkable(ShrinkableAny::concat(then));
    }

    // andThen: continues with then after vertical dead end
    Shrinkable<T> andThenStatic(function<Stream()> then) const
    {
        return Shrinkable(ShrinkableAny::andThenStatic(then));
    }

    Shrinkable<T> andThen(function<Stream(const Shrinkable<T>&)> then) const
    {
        return Shrinkable(ShrinkableAny::andThen(then));
    }

    Shrinkable<T> take(int n) const
    {
        return Shrinkable(ShrinkableAny::take(n));
    }

private:
    Shrinkable(shared_ptr<Any> p, shared_ptr<function<Stream()>> s) : ShrinkableAny(p, s) { }


public:

    template <typename U, typename... Args>
    friend Shrinkable<U> make_shrinkable(Args&&... args);
};

namespace util {

template <typename T, typename U>
AnyFunction::AnyFunction(function<U(const T&)> inFunc)
{
    auto inFuncPtr = make_shared<decltype(inFunc)>(inFunc);

    funcPtr = make_shared<F>([inFuncPtr](const Any& a) -> Any {
        return (*inFuncPtr)(a.cast<T>());
    });
}


template <typename T, typename U>
ShrinkableAnyFunction::ShrinkableAnyFunction(function<Shrinkable<U>(const T&)> inFunc) {
    auto inFuncPtr = make_shared<decltype(inFunc)>(inFunc);

    funcPtr = make_shared<F>([inFuncPtr](const Any& a) -> ShrinkableAny {
        return (*inFuncPtr)(a.cast<T>());
    });
}


template <typename T, typename U>
ShrinkableAnyFunction1::ShrinkableAnyFunction1(function<Shrinkable<U>(const Shrinkable<T>&)> inFunc) {
    auto inFuncPtr = make_shared<decltype(inFunc)>(inFunc);

    funcPtr = make_shared<F>([inFuncPtr](const ShrinkableAny& shr) -> ShrinkableAny {
        return (*inFuncPtr)(shr.as<T>());
    });
}

template <typename T>
BoolFunction::BoolFunction(function<bool(const T&)> inFunc) {
    auto inFuncPtr = make_shared<decltype(inFunc)>(inFunc);

    funcPtr = make_shared<F>([inFuncPtr](const Any& a) -> bool {
        return (*inFuncPtr)(a.cast<T>());
    });
}

template <typename T>
StreamFunction::StreamFunction(function<Stream(const Shrinkable<T>&)> inFunc) {
    auto inFuncPtr = make_shared<decltype(inFunc)>(inFunc);

    funcPtr = make_shared<F>([inFuncPtr](const ShrinkableAny& shr) -> Stream {
        return (*inFuncPtr)(shr.as<T>());
    });
}

} // namespace util

template <typename T>
Shrinkable<T> ShrinkableAny::as() const {
    return Shrinkable<T>(*this);
}

template <typename T, typename... Args>
Shrinkable<T> make_shrinkable(Args&&... args)
{
    Shrinkable<T> shrinkable(util::make_shared<Any>(make_anything<T>(args...)));
    return shrinkable;
}

template <typename T, typename... Args>
ShrinkableAny make_shrinkable_any(Args&&... args)
{
    ShrinkableAny shrinkable(util::make_shared<Any>(make_anything<T>(args...)));
    return shrinkable;
}

}  // namespace proptest
