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
class Arbitrary<std::set<T>> : public Gen<std::set<T>> {
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

    Shrinkable<Set> operator()(Random& rand)
    {
        // generate random Ts using elemGen
        size_t size = rand.getRandomSize(minSize, maxSize + 1);
        std::shared_ptr<std::set<Shrinkable<T>>> shrinkSet = std::make_shared<std::set<Shrinkable<T>>>();

        while (shrinkSet->size() < size) {
            auto elem = elemGen(rand);
            shrinkSet->insert(elem);
        }
        // shrink set size with subset using binary numeric shrink of sizes
        int minSizeCopy = minSize;
        auto rangeShrinkable =
            binarySearchShrinkable<int>(size - minSizeCopy).template transform<int>([minSizeCopy](const int& size) {
                return size + minSizeCopy;
            });
        // this make sure shrinking is possible towards minSize
        Shrinkable<std::set<Shrinkable<T>>> shrinkable =
            rangeShrinkable.template transform<std::set<Shrinkable<T>>>([shrinkSet](const int& size) {
                if (size <= 0)
                    return make_shrinkable<std::set<Shrinkable<T>>>();

                int i = 0;
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

    Arbitrary<std::set<T>> setMinSize(int size)
    {
        minSize = size;
        return *this;
    }

    Arbitrary<std::set<T>> setMaxSize(int size)
    {
        maxSize = size;
        return *this;
    }

    Arbitrary<std::set<T>> setSize(int size)
    {
        minSize = size;
        maxSize = size;
        return *this;
    }

    std::function<Shrinkable<T>(Random&)> elemGen;
    int minSize;
    int maxSize;
};

template <typename T>
size_t Arbitrary<std::set<T>>::defaultMinSize = 0;
template <typename T>
size_t Arbitrary<std::set<T>>::defaultMaxSize = 200;

}  // namespace PropertyBasedTesting
