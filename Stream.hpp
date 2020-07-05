#pragma once

#include <functional>
#include <memory>

namespace pbt {

template <typename T>
struct Stream;

template <typename T>
struct Iterator
{
    Iterator(const Stream<T>& str) : stream(str) {}

    bool hasNext() const { return !stream.isEmpty(); }

    T next()
    {
        if (!hasNext()) {
            throw std::invalid_argument("iterator has no more next");
        }
        T value = stream.head();
        stream = stream.tail();
        return value;
    }

    Stream<T> stream;
};

template <typename T>
struct Stream
{
    using type = T;
    Stream() {}
    Stream(const Stream& other) : headPtr(other.headPtr), tailGen(other.tailGen) {}
    Stream(const std::shared_ptr<Stream<T>>& other) : headPtr(other->headPtr), tailGen(other->tailGen) {}

    Stream(const T& h, std::function<Stream<T>()> gen)
        : headPtr(std::make_shared<T>(h)), tailGen(std::make_shared<std::function<Stream<T>()>>(gen))
    {
    }

    Stream(const std::shared_ptr<T>& h, std::function<Stream<T>()> gen)
        : headPtr(h), tailGen(std::make_shared<std::function<Stream<T>()>>(gen))
    {
    }

    Stream(const T& h) : headPtr(std::make_shared<T>(h)), tailGen(std::make_shared<std::function<Stream<T>()>>(done()))
    {
    }

    bool isEmpty() const { return !static_cast<bool>(headPtr); }

    T head() const { return *headPtr; }

    Stream<T> tail() const
    {
        if (isEmpty())
            return Stream();

        return Stream((*tailGen)());
    }

    Iterator<T> iterator() const { return Iterator<T>{Stream<T>{*this}}; }

    template <typename U = T>
    Stream<U> transform(std::function<U(const T&)> transformer)
    {
        auto transformerPtr = std::make_shared<decltype(transformer)>(transformer);
        return transform<U>(transformerPtr);
    }

    template <typename U = T>
    Stream<U> transform(std::shared_ptr<std::function<U(const T&)>> transformerPtr)
    {
        if (isEmpty()) {
            return Stream<U>::empty();
        } else {
            auto thisTailGen = tailGen;
            return Stream<U>((*transformerPtr)(head()), [transformerPtr, thisTailGen]() -> Stream<U> {
                return (*thisTailGen)().transform(transformerPtr);
            });
        }
    }

    Stream<T> filter(std::function<bool(const T&)> criteria) const
    {
        auto criteriaPtr = std::make_shared<decltype(criteria)>(criteria);
        return filter(criteriaPtr);
    }

    Stream<T> filter(std::shared_ptr<std::function<bool(const T&)>> criteriaPtr) const
    {
        if (isEmpty()) {
            return Stream::empty();
        } else {
            for (auto itr = iterator(); itr.hasNext();) {
                auto value = itr.next();
                if ((*criteriaPtr)(value)) {
                    auto tail = itr.stream;
                    return Stream<T>{value, [criteriaPtr, tail]() { return tail.filter(criteriaPtr); }};
                }
            }
            return Stream<T>::empty();
        }
    }

    Stream<T> concat(const Stream<T>& other) const
    {
        if (isEmpty())
            return other;
        else {
            return Stream<T>(headPtr,
                             [tailGen = this->tailGen, other]() { return Stream((*tailGen)()).concat(other); });
        }
    }

    Stream<T> take(int n) const
    {
        if (isEmpty())
            return empty();
        else {
            auto self = *this;
            if (n == 0)
                return Stream::empty();

            return Stream(headPtr, [self, n]() { return Stream<T>(self.tail()).take(n - 1); });
        }
    }

    std::shared_ptr<T> headPtr;
    std::shared_ptr<std::function<Stream<T>()>> tailGen;

    static Stream<T> empty() { return Stream(); }

    static std::function<Stream<T>()> done()
    {
        static auto produceEmpty = []() -> Stream<T> { return empty(); };
        return produceEmpty;
    }

    static Stream<T> one(T&& a) { return Stream(a); }

    static Stream<T> two(T&& a, T&& b)
    {
        return Stream(a, [=]() -> Stream<T> { return Stream(b); });
    }
};

}  // namespace pbt
