#pragma once

#include "util/std.hpp"

namespace proptest {

struct Stream;

template <typename T>
struct Iterator;

struct Stream
{
    Stream() {}
    Stream(const Stream& other) : headPtr(other.headPtr), tailGen(other.tailGen) {}
    Stream(const shared_ptr<Stream>& other) : headPtr(other->headPtr), tailGen(other->tailGen) {}

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

    Stream& operator=(const Stream& other)
    {
        headPtr = other.headPtr;
        tailGen = other.tailGen;

        return *this;
    }

    bool isEmpty() const { return !static_cast<bool>(headPtr); }

    template <typename T>
    T head() const { return any_cast<T>(*headPtr); }

    Stream tail() const
    {
        if (isEmpty())
            return Stream();

        return Stream((*tailGen)());
    }

    template <typename T>
    Iterator<T> iterator() const { return Iterator<T>{Stream{*this}}; }

    template <typename T, typename U>
    Stream transform(function<U(const T&)> transformer)
    {
        auto transformerPtr = util::make_shared<decltype(transformer)>(transformer);
        return transform<T, U>(transformerPtr);
    }

    template <typename T, typename U>
    Stream transform(shared_ptr<function<U(const T&)>> transformerPtr)
    {
        if (isEmpty()) {
            return Stream::empty();
        } else {
            auto thisTailGen = tailGen;
            return Stream((*transformerPtr)(head<T>()), [transformerPtr, thisTailGen]() -> Stream {
                return (*thisTailGen)().transform(transformerPtr);
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

    Stream concat(const Stream& other) const
    {
        if (isEmpty())
            return other;
        else {
            return Stream(headPtr,
                             [tailGen = this->tailGen, other]() { return Stream((*tailGen)()).concat(other); });
        }
    }

    Stream take(int n) const
    {
        if (isEmpty())
            return empty();
        else {
            auto self = *this;
            if (n == 0)
                return Stream::empty();

            return Stream(headPtr, [self, n]() { return Stream(self.tail()).take(n - 1); });
        }
    }

    shared_ptr<util::any> headPtr;
    shared_ptr<function<Stream()>> tailGen;

    static Stream empty() { return Stream(); }

    static function<Stream()> done()
    {
        static auto produceEmpty = +[]() -> Stream { return empty(); };
        return produceEmpty;
    }

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
