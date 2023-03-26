#pragma once

#include "util/std.hpp"

namespace proptest {

struct Stream;

template <typename T>
struct Iterator;

struct Stream
{
    Stream();
    Stream(const Stream& other);
    Stream(const shared_ptr<Stream>& other);

    template <typename T>
    Stream(const shared_ptr<T>& h, function<Stream()> gen)
        : headPtr(static_pointer_cast<util::any>(h)), tailGen(util::make_shared<function<Stream()>>(gen))
    {
    }

    template <typename T>
    Stream(const T& h, function<Stream()> gen)
        : headPtr(util::make_shared<util::any>(h)), tailGen(util::make_shared<function<Stream()>>(gen))
    {
    }


    template <typename T>
    Stream(const T& h) : headPtr(util::make_shared<util::any>(h)), tailGen(util::make_shared<function<Stream()>>(done()))
    {
    }

    Stream& operator=(const Stream& other);

    bool isEmpty() const;

    template <typename T>
    T head() const { return any_cast<T>(*headPtr); }

    Stream tail() const;

    template <typename T>
    Iterator<T> iterator() const { return Iterator<T>{Stream{*this}}; }

    template <typename T, typename U>
    Stream transform(function<U(const T&)> transformer)
    {
        function<util::any(const util::any&)> anytransformer = [transformer](const util::any& any) -> util::any {
            return util::any(transformer(any_cast<T>(any)));
        };
        auto transformerPtr = util::make_shared<decltype(anytransformer)>(anytransformer);
        return transform2(transformerPtr);
    }

    template <typename T, typename U>
    Stream transform(shared_ptr<function<U(const T&)>> transformerPtr)
    {
        function<util::any(const util::any&)> anytransformer = [transformerPtr](const util::any& any) -> util::any {
            return util::any((*transformerPtr)(any_cast<T>(any)));
        };
        auto transformerPtr2 = util::make_shared<decltype(anytransformer)>(anytransformer);
        return transform2(transformerPtr2);
    }

    Stream transform2(shared_ptr<function<util::any(const util::any&)>> transformerPtr)
    {
        if (isEmpty()) {
            return Stream::empty();
        } else {
            auto thisTailGen = tailGen;
            return Stream((*transformerPtr)(*headPtr), [transformerPtr, thisTailGen]() -> Stream {
                return (*thisTailGen)().transform2(transformerPtr);
            });
        }
    }

    template <typename T>
    Stream filter(function<bool(const T&)> criteria) const
    {
        auto criteriaPtr = util::make_shared<decltype(criteria)>(criteria);
        return filter(criteriaPtr);
    }

    template <typename T>
    Stream filter(shared_ptr<function<bool(const T&)>> criteriaPtr) const
    {
        if (isEmpty()) {
            return Stream::empty();
        } else {
            for (auto itr = iterator<T>(); itr.hasNext();) {
                auto value = itr.next();
                if ((*criteriaPtr)(value)) {
                    auto tail = itr.stream;
                    return Stream{value, [criteriaPtr, tail]() { return tail.filter(criteriaPtr); }};
                }
            }
            return Stream::empty();
        }
    }

    Stream concat(const Stream& other) const;

    Stream take(int n) const;

    shared_ptr<util::any> headPtr;
    shared_ptr<function<Stream()>> tailGen;

    static Stream empty();

    static function<Stream()> done();

    template <typename T>
    static Stream one(T&& a) { return Stream(a); }

    template <typename T>
    static Stream two(T&& a, T&& b)
    {
        return Stream(a, [=]() -> Stream { return Stream(b); });
    }
};


template <typename T>
struct Iterator
{
    Iterator(const Stream& str) : stream(str) {}

    bool hasNext() const { return !stream.isEmpty(); }

    T next()
    {
        if (!hasNext()) {
            throw invalid_argument("iterator has no more next");
        }
        T value = stream.head<T>();
        stream = stream.tail();
        return value;
    }

    Stream stream;
};


}  // namespace proptest
