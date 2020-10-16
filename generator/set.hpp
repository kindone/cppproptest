#pragma once
#include "../gen.hpp"
#include "../Random.hpp"
#include "../Shrinkable.hpp"
#include "util.hpp"
#include <memory>
#include <set>

namespace std {

template <typename T>
class less<proptest::Shrinkable<T>> {
public:
    constexpr bool operator()(const proptest::Shrinkable<T>& lhs, const proptest::Shrinkable<T>& rhs) const
    {
        return lhs.getRef() < rhs.getRef();
    }
};

}  // namespace std

namespace proptest {

template <typename T>
class Arbi<std::set<T>> final : public ArbiContainer<std::set<T>> {
    using Set = typename std::set<T>;
    using ArbiContainer<Set>::minSize;
    using ArbiContainer<Set>::maxSize;

public:
    static size_t defaultMinSize;
    static size_t defaultMaxSize;

    Arbi() : ArbiContainer<Set>(defaultMinSize, defaultMaxSize), elemGen(Arbi<T>()) {}

    Arbi(const Arbi<T>& _elemGen)
        : ArbiContainer<Set>(defaultMinSize, defaultMaxSize),
          elemGen([_elemGen](Random& rand) -> Shrinkable<T> { return _elemGen(rand); })
    {
    }

    Arbi(GenFunction<T> _elemGen) : ArbiContainer<Set>(defaultMinSize, defaultMaxSize), elemGen(_elemGen) {}

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
            util::binarySearchShrinkableU(size - minSizeCopy)
                .template map<uint64_t>([minSizeCopy](const uint64_t& _size) { return _size + minSizeCopy; });
        // this make sure shrinking is possible towards minSize
        Shrinkable<std::set<Shrinkable<T>>> shrinkable =
            rangeShrinkable.template flatMap<std::set<Shrinkable<T>>>([shrinkSet](const size_t& _size) {
                if (_size == 0)
                    return make_shrinkable<std::set<Shrinkable<T>>>();

                size_t i = 0;
                auto begin = shrinkSet->begin();
                auto last = shrinkSet->begin();
                for (; last != shrinkSet->end() && i < _size; ++last, ++i) {}
                return make_shrinkable<std::set<Shrinkable<T>>>(begin, last);
            });

        return shrinkable.template flatMap<std::set<T>>(+[](const std::set<Shrinkable<T>>& shr) {
            auto value = make_shrinkable<std::set<T>>();
            std::set<T>& valueSet = value.getRef();

            for (auto itr = shr.begin(); itr != shr.end(); ++itr) {
                valueSet.insert(std::move(itr->getRef()));
            }

            return value;
        });
    }

    GenFunction<T> elemGen;
};

template <typename T>
size_t Arbi<std::set<T>>::defaultMinSize = 0;
template <typename T>
size_t Arbi<std::set<T>>::defaultMaxSize = 200;

}  // namespace proptest
