#pragma once
#include "../gen.hpp"
#include "../Random.hpp"
#include "../Shrinkable.hpp"
#include "set.hpp"
#include <memory>
#include <map>

namespace proptest {
template <typename Key, typename T>
class Arbi<std::map<Key, T>> final : public ArbiContainer<std::map<Key, T>> {
    using Map = typename std::map<Key, T>;
    using ArbiContainer<Map>::minSize;
    using ArbiContainer<Map>::maxSize;

public:
    static size_t defaultMinSize;
    static size_t defaultMaxSize;

    Arbi() : ArbiContainer<Map>(defaultMinSize, defaultMaxSize), keyGen(Arbi<Key>()), elemGen(Arbi<T>()) {}

    Shrinkable<Map> operator()(Random& rand) override
    {
        // generate random Ts using elemGen
        size_t size = rand.getRandomSize(minSize, maxSize + 1);
        std::shared_ptr<std::set<Shrinkable<Key>>> shrinkSet = std::make_shared<std::set<Shrinkable<Key>>>();

        while (shrinkSet->size() < size) {
            auto elem = elemGen(rand);
            shrinkSet->insert(elem);
        }

        std::shared_ptr<std::map<Shrinkable<Key>, Shrinkable<T>>> shrinkMap =
            std::make_shared<std::map<Shrinkable<Key>, Shrinkable<T>>>();
        for (auto itr = shrinkSet->begin(); itr != shrinkSet->end(); ++itr) {
            auto key = keyGen(rand);
            shrinkMap->insert(std::pair<Shrinkable<Key>, Shrinkable<T>>(*itr, key));
        }

        // shrink map size with submap using binary numeric shrink of sizes
        size_t minSizeCopy = minSize;
        auto rangeShrinkable =
            util::binarySearchShrinkable(size - minSizeCopy).template map<size_t>([minSizeCopy](const uint64_t& size) {
                return size + minSizeCopy;
            });

        // this make sure shrinking is possible towards minSize
        Shrinkable<std::map<Shrinkable<Key>, Shrinkable<T>>> shrinkable =
            rangeShrinkable.template flatMap<std::map<Shrinkable<Key>, Shrinkable<T>>>([shrinkMap](const size_t& size) {
                if (size == 0)
                    return make_shrinkable<std::map<Shrinkable<Key>, Shrinkable<T>>>();  // empty map

                size_t i = 0;
                auto begin = shrinkMap->begin();
                auto last = shrinkMap->begin();
                for (; last != shrinkMap->end() && i < size; ++last, ++i) {}
                return make_shrinkable<std::map<Shrinkable<Key>, Shrinkable<T>>>(begin, last);
            });

        return shrinkable.template flatMap<std::map<Key, T>>(+[](const std::map<Shrinkable<Key>, Shrinkable<T>>& shr) {
            auto value = make_shrinkable<std::map<Key, T>>();
            std::map<Key, T>& valueMap = value.getRef();

            for (auto itr = shr.begin(); itr != shr.end(); ++itr) {
                auto& pair = *itr;
                valueMap.insert(std::pair<Key, T>(pair.first.getRef(), pair.second.getRef()));
            }

            return value;
        });
    }

    Arbi<Map> setKeyGen(const Arbi<Key>& _keyGen)
    {
        keyGen = _keyGen;
        return *this;
    }

    Arbi<Map> setElemGen(const Arbi<T>& _elemGen)
    {
        elemGen = _elemGen;
        return *this;
    }

    Arbi<Map> setKeyGen(GenFunction<Key> _keyGen)
    {
        keyGen = _keyGen;
        return *this;
    }

    Arbi<Map> setElemGen(GenFunction<T> _elemGen)
    {
        elemGen = _elemGen;
        return *this;
    }

    GenFunction<Key> keyGen;
    GenFunction<T> elemGen;
};

template <typename Key, typename T>
size_t Arbi<std::map<Key, T>>::defaultMinSize = 0;
template <typename Key, typename T>
size_t Arbi<std::map<Key, T>>::defaultMaxSize = 200;

}  // namespace proptest
