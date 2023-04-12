#include "api.hpp"
#include "Shrinkable.hpp"
#include "util/utf8string.hpp"
#include "util/cesu8string.hpp"

namespace proptest {

namespace util {

Any AnyFunction::operator()(const Any& in) {
    return (*funcPtr)(in);
}

ShrinkableAny ShrinkableAnyFunction::operator()(const Any& in) {
    return (*funcPtr)(in);
}

ShrinkableAny ShrinkableAnyFunction1::operator()(const ShrinkableAny& in) {
    return (*funcPtr)(in);
}

bool BoolFunction::operator()(const Any& in) {
    return (*funcPtr)(in);
}

Stream StreamFunction::operator()(const ShrinkableAny& in) {
    return (*funcPtr)(in);
}

} // namespace util



ShrinkableAny::ShrinkableAny(shared_ptr<Any> p) : ptr(p), shrinksPtr(emptyPtr()) {}
ShrinkableAny::ShrinkableAny(const ShrinkableAny& other) : ptr(other.ptr), shrinksPtr(other.shrinksPtr) {}

ShrinkableAny& ShrinkableAny::operator=(const ShrinkableAny& other)
{
    ptr = other.ptr;
    shrinksPtr = other.shrinksPtr;
    return *this;
}

ShrinkableAny ShrinkableAny::with(function<Stream()> _shrinks) const
{
    return ShrinkableAny(ptr, util::make_shared<decltype(_shrinks)>(_shrinks));
}

ShrinkableAny ShrinkableAny::with(shared_ptr<function<Stream()>> newShrinksPtr) const
{
    return ShrinkableAny(ptr, newShrinksPtr);
}

Any& ShrinkableAny::getAnyRef() const { return *ptr; }

ShrinkableAny ShrinkableAny::map(util::AnyFunction transformer) const
{
    auto transformerPtr = util::make_shared<util::AnyFunction>(transformer);
    return map(transformerPtr);
}

ShrinkableAny ShrinkableAny::map(shared_ptr<util::AnyFunction> transformerPtr) const
{
    auto thisShrinksPtr = shrinksPtr;
    ShrinkableAny shrinkable(util::make_shared<Any>((*transformerPtr)(getAnyRef())));
    return shrinkable.with([thisShrinksPtr, transformerPtr]() {
        return (*thisShrinksPtr)().template transform<ShrinkableAny, ShrinkableAny>(
            [transformerPtr](const ShrinkableAny& shr) { return shr.map(transformerPtr); });
    });
}

ShrinkableAny ShrinkableAny::map(shared_ptr<function<Any(const Any&)>> transformerPtr) const
{
    auto thisShrinksPtr = shrinksPtr;
    ShrinkableAny shrinkable(util::make_shared<Any>((*transformerPtr)(getAnyRef())));
    return shrinkable.with([thisShrinksPtr, transformerPtr]() {
        return (*thisShrinksPtr)().template transform<ShrinkableAny, ShrinkableAny>(
            [transformerPtr](const ShrinkableAny& shr) { return shr.map(transformerPtr); });
    });
}

ShrinkableAny ShrinkableAny::flatMap(util::ShrinkableAnyFunction transformer) const
{
    auto transformerPtr = util::make_shared<util::ShrinkableAnyFunction>(transformer);
    return flatMap(transformerPtr);
}

ShrinkableAny ShrinkableAny::flatMap(shared_ptr<util::ShrinkableAnyFunction> transformerPtr) const
{
    auto thisShrinksPtr = shrinksPtr;
    auto shrinkable = (*transformerPtr)(getAnyRef());
    return shrinkable.with([thisShrinksPtr, transformerPtr]() {
        return (*thisShrinksPtr)().template transform<ShrinkableAny, ShrinkableAny>(
            [transformerPtr](const ShrinkableAny& shr) { return shr.flatMap(transformerPtr); });
    });
}

ShrinkableAny ShrinkableAny::mapShrinkable(util::ShrinkableAnyFunction1 transformer) const
{
    auto transformerPtr = util::make_shared<util::ShrinkableAnyFunction1>(transformer);
    return mapShrinkable(transformerPtr);
}

ShrinkableAny ShrinkableAny::mapShrinkable(
    shared_ptr<util::ShrinkableAnyFunction1> transformerPtr) const
{
    auto thisShrinksPtr = shrinksPtr;
    auto shrinkable = (*transformerPtr)(*this);
    return shrinkable.with([thisShrinksPtr, transformerPtr]() {
        return (*thisShrinksPtr)().template transform<ShrinkableAny, ShrinkableAny>(
            [transformerPtr](const ShrinkableAny& shr) { return shr.mapShrinkable(transformerPtr); });
    });
}

// provide filtered generation, shrinking
ShrinkableAny ShrinkableAny::filter(util::BoolFunction criteria) const
{
    auto criteriaPtr = util::make_shared<util::BoolFunction>(criteria);
    return filter(criteriaPtr);
}

ShrinkableAny ShrinkableAny::filter(shared_ptr<util::BoolFunction> criteriaPtr) const
{
    if (!(*criteriaPtr)(getAnyRef()))
        throw invalid_argument("cannot apply criteria");

    auto thisShrinksPtr = shrinksPtr;

    return with([thisShrinksPtr, criteriaPtr]() {
        auto criteriaForStream = util::make_shared<function<bool(const ShrinkableAny&)>>(
            [criteriaPtr](const ShrinkableAny& shr) -> bool { return (*criteriaPtr)(shr.getAnyRef()); });
        // filter stream's value, and then transform each shrinkable to call filter recursively
        return (*thisShrinksPtr)()
            .filter(criteriaForStream)
            .template transform<ShrinkableAny, ShrinkableAny>(
                [criteriaPtr](const ShrinkableAny& shr) { return shr.filter(criteriaPtr); });
    });
}

// provide filtered generation, shrinking
ShrinkableAny ShrinkableAny::filter(util::BoolFunction criteria, int tolerance) const
{
    auto criteriaPtr = util::make_shared<util::BoolFunction>(criteria);
    return filter(criteriaPtr, tolerance);
}

ShrinkableAny ShrinkableAny::filter(shared_ptr<util::BoolFunction> criteriaPtr, int tolerance) const
{
    if (!(*criteriaPtr)(getAnyRef()))
        throw invalid_argument("cannot apply criteria");

    auto thisShrinksPtr = shrinksPtr;

    static function<Stream(const Stream&,
                                            shared_ptr<function<bool(const ShrinkableAny&)>>,
                                            int)> filterStream =
        [](const Stream& stream,
            shared_ptr<function<bool(const ShrinkableAny&)>> _criteriaPtr,
            int _tolerance) -> Stream {
        if (stream.isEmpty()) {
            return Stream::empty();
        } else {
            for (auto itr = stream.iterator<ShrinkableAny>(); itr.hasNext();) {
                const ShrinkableAny& shr = itr.next();
                if ((*_criteriaPtr)(shr)) {
                    auto tail = itr.stream;
                    return Stream{shr, [_criteriaPtr, tail, _tolerance]() { return filterStream(tail, _criteriaPtr, _tolerance); }};
                } else {
                    // extract from shr's children
                    auto tail = itr.stream;
                    return filterStream(shr.shrinks().take(_tolerance).concat(tail), _criteriaPtr, _tolerance);
                }
            }
            return Stream::empty();
        }
    };

    return with([thisShrinksPtr, criteriaPtr, tolerance]() {
        auto criteriaForStream = util::make_shared<function<bool(const ShrinkableAny&)>>(
            [criteriaPtr](const ShrinkableAny& shr) -> bool { return (*criteriaPtr)(shr.getAnyRef()); });
        // filter stream's value, and then transform each shrinkable to call filter recursively
        return filterStream((*thisShrinksPtr)(), criteriaForStream, tolerance)
            .template transform<ShrinkableAny, ShrinkableAny>(
                [criteriaPtr, tolerance](const ShrinkableAny& shr) { return shr.filter(criteriaPtr, tolerance); });
    });
}

ShrinkableAny ShrinkableAny::filter(shared_ptr<function<bool(const Any&)>> criteriaPtr, int tolerance) const
{
    if (!(*criteriaPtr)(getAnyRef()))
        throw invalid_argument("cannot apply criteria");

    auto thisShrinksPtr = shrinksPtr;

    static function<Stream(const Stream&,
                                            shared_ptr<function<bool(const ShrinkableAny&)>>,
                                            int)> filterStream =
        [](const Stream& stream,
            shared_ptr<function<bool(const ShrinkableAny&)>> _criteriaPtr,
            int _tolerance) -> Stream {
        if (stream.isEmpty()) {
            return Stream::empty();
        } else {
            for (auto itr = stream.iterator<ShrinkableAny>(); itr.hasNext();) {
                const ShrinkableAny& shr = itr.next();
                if ((*_criteriaPtr)(shr)) {
                    auto tail = itr.stream;
                    return Stream{shr, [_criteriaPtr, tail, _tolerance]() { return filterStream(tail, _criteriaPtr, _tolerance); }};
                } else {
                    // extract from shr's children
                    auto tail = itr.stream;
                    return filterStream(shr.shrinks().take(_tolerance).concat(tail), _criteriaPtr, _tolerance);
                }
            }
            return Stream::empty();
        }
    };

    return with([thisShrinksPtr, criteriaPtr, tolerance]() {
        auto criteriaForStream = util::make_shared<function<bool(const ShrinkableAny&)>>(
            [criteriaPtr](const ShrinkableAny& shr) -> bool { return (*criteriaPtr)(shr.getAnyRef()); });
        // filter stream's value, and then transform each shrinkable to call filter recursively
        return filterStream((*thisShrinksPtr)(), criteriaForStream, tolerance)
            .template transform<ShrinkableAny, ShrinkableAny>(
                [criteriaPtr, tolerance](const ShrinkableAny& shr) { return shr.filter(criteriaPtr, tolerance); });
    });
}

// concat: continues with then after horizontal dead end
ShrinkableAny ShrinkableAny::concatStatic(function<Stream()> then) const
{
    auto thenPtr = util::make_shared<decltype(then)>(then);
    return concatStatic(thenPtr);
}

ShrinkableAny ShrinkableAny::concatStatic(shared_ptr<function<Stream()>> thenPtr) const
{
    auto thisShrinksPtr = shrinksPtr;
    return with([thisShrinksPtr, thenPtr]() {
        auto shrinkablesWithThen = (*thisShrinksPtr)().template transform<ShrinkableAny, ShrinkableAny>(
            [thenPtr](const ShrinkableAny& shr) { return shr.concatStatic(thenPtr); });
        return shrinkablesWithThen.concat((*thenPtr)());
    });
}

// concat: extend shrinks stream with function taking parent as argument
ShrinkableAny ShrinkableAny::concat(util::StreamFunction then) const
{
    auto thenPtr = util::make_shared<decltype(then)>(then);
    return concat(thenPtr);
}

ShrinkableAny ShrinkableAny::concat(shared_ptr<util::StreamFunction> thenPtr) const
{
    auto thisShrinksPtr = shrinksPtr;
    return with([copy = *this, thisShrinksPtr, thenPtr]() {
        auto shrinkablesWithThen = (*thisShrinksPtr)().template transform<ShrinkableAny, ShrinkableAny>(
            [thenPtr](const ShrinkableAny& shr) { return shr.concat(thenPtr); });
        return shrinkablesWithThen.concat((*thenPtr)(copy));
    });
}

// andThen: continues with then after vertical dead end
ShrinkableAny ShrinkableAny::andThenStatic(function<Stream()> then) const
{
    auto thisShrinksPtr = shrinksPtr;
    auto thenPtr = util::make_shared<decltype(then)>(then);
    return andThenStatic(thenPtr);
}

ShrinkableAny ShrinkableAny::andThenStatic(shared_ptr<function<Stream()>> thenPtr) const
{
    auto thisShrinksPtr = shrinksPtr;
    if (shrinks().isEmpty()) {
        return with(thenPtr);
    } else {
        return with([thisShrinksPtr, thenPtr]() {
            return (*thisShrinksPtr)().template transform<ShrinkableAny, ShrinkableAny>(
                [thenPtr](const ShrinkableAny& shr) { return shr.andThenStatic(thenPtr); });
        });
    }
}

ShrinkableAny ShrinkableAny::andThen(util::StreamFunction then) const
{
    auto thenPtr = util::make_shared<decltype(then)>(then);
    return andThen(thenPtr);
}

ShrinkableAny ShrinkableAny::andThen(shared_ptr<util::StreamFunction> thenPtr) const
{
    auto thisShrinksPtr = shrinksPtr;
    auto selfSharedPtr = util::make_shared<ShrinkableAny>(*this);
    if (shrinksPtr->operator()().isEmpty()) {
        return with([selfSharedPtr, thenPtr]() { return (*thenPtr)(*selfSharedPtr); });
    } else {
        return with([thisShrinksPtr, thenPtr]() {
            return (*thisShrinksPtr)().template transform<ShrinkableAny, ShrinkableAny>(
                [thenPtr](const ShrinkableAny& shr) { return shr.andThen(thenPtr); });
        });
    }
}

ShrinkableAny ShrinkableAny::take(int n) const
{
    auto thisShrinksPtr = shrinksPtr;
    return with([thisShrinksPtr, n]() {
        auto shrinks = (*thisShrinksPtr)().take(n);
        return shrinks.template transform<ShrinkableAny, ShrinkableAny>([n](const ShrinkableAny& shr) { return shr.take(n); });
    });
}

ShrinkableAny::ShrinkableAny(shared_ptr<Any> p, shared_ptr<function<Stream()>> s)  : ptr(p), shrinksPtr(s) { }

Stream ShrinkableAny::shrinks() const { return (*shrinksPtr)(); }

shared_ptr<function<Stream()>> ShrinkableAny::emptyPtr()
{
    static const auto empty =
        util::make_shared<function<Stream()>>(+[]() { return Stream::empty(); });
    return empty;
}

// explicit instantiation
template class Shrinkable<vector<ShrinkableAny>>;

}  // namespace proptest
