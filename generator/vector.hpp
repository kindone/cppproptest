#pragma once

#include "../gen.hpp"
#include "../Random.hpp"
#include "../util/printing.hpp"
#include "util.hpp"
#include <vector>
#include <iostream>
#include <cmath>
#include <memory>

namespace PropertyBasedTesting {

template <typename T>
class PROPTEST_API Arbitrary<std::vector<T>> final : public ArbitraryContainer<std::vector<T>> {
public:
    using Vector = std::vector<T>;
    using ArbitraryContainer<std::vector<T>>::minSize;
    using ArbitraryContainer<std::vector<T>>::maxSize;
    static size_t defaultMinSize;
    static size_t defaultMaxSize;

    Arbitrary() : ArbitraryContainer<std::vector<T>>(defaultMinSize, defaultMaxSize), elemGen(Arbitrary<T>()) {}

    Arbitrary(const Arbitrary<T>& _elemGen)
        : ArbitraryContainer<std::vector<T>>(defaultMinSize, defaultMaxSize),
          elemGen([_elemGen](Random& rand) -> Shrinkable<T> { return _elemGen(rand); })
    {
    }

    Arbitrary(std::function<Shrinkable<T>(Random&)> _elemGen)
        : ArbitraryContainer<std::vector<T>>(defaultMinSize, defaultMaxSize), elemGen(_elemGen)
    {
    }

private:
    using vector_t = std::vector<Shrinkable<T>>;
    using shrinkable_t = Shrinkable<vector_t>;
    using stream_t = Stream<shrinkable_t>;
    using e_stream_t = Stream<Shrinkable<T>>;

    static stream_t shrinkBulk(const shrinkable_t& ancestor, size_t power, size_t offset)
    {
        static std::function<stream_t(const shrinkable_t&, size_t, size_t, const shrinkable_t&, size_t, size_t,
                                      std::shared_ptr<std::vector<e_stream_t>>)>
            genStream = [](const shrinkable_t& ancestor, size_t power, size_t offset, const shrinkable_t& parent,
                           size_t frompos, size_t topos,
                           std::shared_ptr<std::vector<e_stream_t>> elemStreams) -> stream_t {
            const size_t size = topos - frompos;
            if (size == 0)
                return stream_t::empty();

            if (elemStreams->size() != size)
                throw std::runtime_error("element streams size error");

            std::shared_ptr<std::vector<e_stream_t>> newElemStreams = std::make_shared<std::vector<e_stream_t>>();
            newElemStreams->reserve(size);

            vector_t newVec = parent.getRef();
            vector_t& ancestorVec = ancestor.getRef();

            if (newVec.size() != ancestorVec.size())
                throw std::runtime_error("vector size error: " + std::to_string(newVec.size()) +
                                         " != " + std::to_string(ancestorVec.size()));

            // shrink each element in frompos~topos, put parent if shrink no longer possible
            bool nothingToDo = true;

            for (size_t i = 0; i < elemStreams->size(); i++) {
                if ((*elemStreams)[i].isEmpty()) {
                    newVec[i + frompos] = make_shrinkable<T>(ancestorVec[i + frompos].get());
                    newElemStreams->push_back(e_stream_t::empty());  // [1] -> []
                } else {
                    newVec[i + frompos] = (*elemStreams)[i].head();
                    newElemStreams->push_back((*elemStreams)[i].tail());  // [0,4,6,7] -> [4,6,7]
                    nothingToDo = false;
                }
            }
            if (nothingToDo)
                return stream_t::empty();

            auto newShrinkable = make_shrinkable<vector_t>(newVec);
            newShrinkable = newShrinkable.with(
                [newShrinkable, power, offset]() -> stream_t { return shrinkBulk(newShrinkable, power, offset); });
            return stream_t(
                newShrinkable, [ancestor, power, offset, newShrinkable, frompos, topos, newElemStreams]() -> stream_t {
                    return genStream(ancestor, power, offset, newShrinkable, frompos, topos, newElemStreams);
                });
        };

        size_t parentSize = ancestor.getRef().size();
        size_t numSplits = static_cast<size_t>(std::pow(2, power));
        if (parentSize / numSplits < 1)
            return stream_t::empty();

        if (offset >= numSplits)
            throw std::runtime_error("offset should not reach numSplits");

        size_t frompos = parentSize * offset / numSplits;
        size_t topos = parentSize * (offset + 1) / numSplits;

        if (topos < parentSize)
            throw std::runtime_error("topos error: " + std::to_string(topos) + " != " + std::to_string(parentSize));

        const size_t size = topos - frompos;
        vector_t& parentVec = ancestor.getRef();
        std::shared_ptr<std::vector<e_stream_t>> elemStreams = std::make_shared<std::vector<e_stream_t>>();
        elemStreams->reserve(size);

        bool nothingToDo = true;
        for (size_t i = frompos; i < topos; i++) {
            auto shrinks = parentVec[i].shrinks();
            elemStreams->push_back(shrinks);
            if (!shrinks.isEmpty())
                nothingToDo = false;
        }

        if (nothingToDo)
            return stream_t::empty();

        return genStream(ancestor, power, offset, ancestor, frompos, topos, elemStreams);
    }

    static stream_t shrinkBulkRecursive(const shrinkable_t& shrinkable, size_t power, size_t offset)
    {
        if (shrinkable.getRef().empty())
            return stream_t::empty();

        size_t vecSize = shrinkable.getRef().size();
        size_t numSplits = static_cast<size_t>(std::pow(2, power));
        if (vecSize / numSplits < 1 || offset >= numSplits)
            return stream_t::empty();
        // entirety
        shrinkable_t newShrinkable = shrinkable.concat([power, offset](const shrinkable_t& shr) -> stream_t {
            size_t vecSize = shr.getRef().size();
            size_t numSplits = static_cast<size_t>(std::pow(2, power));
            if (vecSize / numSplits < 1 || offset >= numSplits)
                return stream_t::empty();
            // std::cout << "entire: " << power << ", " << offset << std::endl;
            return shrinkBulk(shr, power, offset);
        });

        return newShrinkable.shrinks();
    }

public:
    Shrinkable<std::vector<T>> operator()(Random& rand) override
    {
        size_t size = rand.getRandomSize(minSize, maxSize + 1);
        std::shared_ptr<vector_t> shrinkVec = std::make_shared<vector_t>();
        shrinkVec->reserve(size);
        for (size_t i = 0; i < size; i++)
            shrinkVec->push_back(elemGen(rand));

        // shrink vector size with subvector using binary numeric shrink of sizes
        size_t minSizeCopy = minSize;
        auto rangeShrinkable =
            binarySearchShrinkable<size_t>(size - minSizeCopy)
                .template transform<size_t>([minSizeCopy](const int& size) -> size_t { return size + minSizeCopy; });
        // this make sure shrinking is possible towards minSize
        shrinkable_t shrinkable = rangeShrinkable.template transform<std::vector<Shrinkable<T>>>(
            [shrinkVec](const size_t& size) -> Shrinkable<std::vector<Shrinkable<T>>> {
                if (size <= 0)
                    return make_shrinkable<std::vector<Shrinkable<T>>>();

                auto begin = shrinkVec->begin();
                auto last = shrinkVec->begin() + size;  // subvector of (0, size)
                return make_shrinkable<std::vector<Shrinkable<T>>>(begin, last);
            });

        shrinkable =
            shrinkable.andThen([](const shrinkable_t& shr) -> stream_t { return shrinkBulkRecursive(shr, 0, 0); });

        auto vecShrinkable =
            shrinkable.template transform<std::vector<T>>([](const vector_t& shrinkVec) -> Shrinkable<std::vector<T>> {
                auto value = make_shrinkable<std::vector<T>>();
                std::vector<T>& valueVec = value.getRef();
                std::transform(shrinkVec.begin(), shrinkVec.end(), std::back_inserter(valueVec),
                               [](const Shrinkable<T>& shr) -> T { return std::move(shr.getRef()); });
                return value;
            });

        return vecShrinkable;
    }

    // FIXME: turn to shared_ptr
private:
    std::function<Shrinkable<T>(Random&)> elemGen;
};

template <typename T>
size_t Arbitrary<std::vector<T>>::defaultMinSize = 0;
template <typename T>
size_t Arbitrary<std::vector<T>>::defaultMaxSize = 200;

}  // namespace PropertyBasedTesting
