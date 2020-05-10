#pragma once
#include "../gen.hpp"
#include "../Random.hpp"
#include "../Shrinkable.hpp"
#include <memory>
#include <set>

namespace std {

template <typename T>
class less<PropertyBasedTesting::Shrinkable<T>> {
public:
    constexpr bool operator()(const PropertyBasedTesting::Shrinkable<T>& lhs,
                              const PropertyBasedTesting::Shrinkable<T>& rhs) const
    {
        return lhs.getRef() < rhs.getRef();
    }
};

}  // namespace std

namespace PropertyBasedTesting {

template <typename T>
class Arbitrary<std::set<T>> final : public Gen<std::set<T>> {
    using Set = typename std::set<T>;

public:
    static size_t defaultMinSize;
    static size_t defaultMaxSize;

    Arbitrary() : elemGen(Arbitrary<T>()), minSize(defaultMinSize), maxSize(defaultMaxSize) {}

    Arbitrary(const Arbitrary<T>& _elemGen)
        : elemGen([_elemGen](Random& rand) -> Shrinkable<T> { return _elemGen(rand); }),
          minSize(defaultMinSize),
          maxSize(defaultMaxSize)
    {
    }

    Arbitrary(std::function<Shrinkable<T>(Random&)> _elemGen)
        : elemGen(_elemGen), minSize(defaultMinSize), maxSize(defaultMaxSize)
    {
    }

    Shrinkable<Set> operator()(Random& rand) override
    {
        // generate random Ts using elemGen
        size_t size = rand.getRandomSize(minSize, maxSize + 1);
        std::shared_ptr<std::set<Shrinkable<T>>> shrinkSet = std::make_shared<std::set<Shrinkable<T>>>();

        while (shrinkSet->size() < size) {
            auto elem = elemGen(rand);
            shrinkSet->insert(elem);
        }
        // shrink set size with subset using binary numeric shrink of sizes
        size_t minSizeCopy = minSize;
        auto rangeShrinkable =
            binarySearchShrinkable<size_t>(size - minSizeCopy).template transform<size_t>([minSizeCopy](const size_t& size) {
                return size + minSizeCopy;
            });
        // this make sure shrinking is possible towards minSize
        Shrinkable<std::set<Shrinkable<T>>> shrinkable =
            rangeShrinkable.template transform<std::set<Shrinkable<T>>>([shrinkSet](const size_t& size) {
                if (size == 0)
                    return make_shrinkable<std::set<Shrinkable<T>>>();

                size_t i = 0;
                auto begin = shrinkSet->begin();
                auto last = shrinkSet->begin();
                for (; last != shrinkSet->end() && i < size; ++last, ++i) {}
                return make_shrinkable<std::set<Shrinkable<T>>>(begin, last);
            });

        return shrinkable.template transform<std::set<T>>([](const std::set<Shrinkable<T>>& shr) {
            auto value = make_shrinkable<std::set<T>>();
            std::set<T>& valueSet = value.getRef();

            for (auto itr = shr.begin(); itr != shr.end(); ++itr) {
                valueSet.insert(std::move(itr->getRef()));
            }

            return value;
        });
    }

    Arbitrary<std::set<T>> setMinSize(size_t size)
    {
        minSize = size;
        return *this;
    }

    Arbitrary<std::set<T>> setMaxSize(size_t size)
    {
        maxSize = size;
        return *this;
    }

    Arbitrary<std::set<T>> setSize(size_t size)
    {
        minSize = size;
        maxSize = size;
        return *this;
    }

    std::function<Shrinkable<T>(Random&)> elemGen;
    size_t minSize;
    size_t maxSize;
};

template <typename T>
size_t Arbitrary<std::set<T>>::defaultMinSize = 0;
template <typename T>
size_t Arbitrary<std::set<T>>::defaultMaxSize = 200;

}  // namespace PropertyBasedTesting
