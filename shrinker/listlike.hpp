#pragma once
#include "../Shrinkable.hpp"
#include <memory>

namespace proptest {

namespace util {

template <typename T>
struct VectorShrinker {
    using shrinkable_vector_t = std::vector<Shrinkable<T>>;
    using shrinkable_t = Shrinkable<shrinkable_vector_t>;
    using stream_t = Stream<shrinkable_t>;
    using e_stream_t = Stream<Shrinkable<T>>;

    static stream_t shrinkBulk(const shrinkable_t& ancestor, size_t power, size_t offset)
    {
        static std::function<stream_t(const shrinkable_t&, size_t, size_t, const shrinkable_t&, size_t, size_t,
                                      std::shared_ptr<std::vector<e_stream_t>>)>
            genStream =
                +[](const shrinkable_t& _ancestor, size_t _power, size_t _offset, const shrinkable_t& parent,
                    size_t frompos, size_t topos, std::shared_ptr<std::vector<e_stream_t>> elemStreams) -> stream_t {
            const size_t size = topos - frompos;
            if (size == 0)
                return stream_t::empty();

            if (elemStreams->size() != size)
                throw std::runtime_error("element streams size error");

            std::shared_ptr<std::vector<e_stream_t>> newElemStreams = std::make_shared<std::vector<e_stream_t>>();
            newElemStreams->reserve(size);

            shrinkable_vector_t newVec = parent.getRef();
            shrinkable_vector_t& ancestorVec = _ancestor.getRef();

            if (newVec.size() != ancestorVec.size())
                throw std::runtime_error("list size error: " + std::to_string(newVec.size()) +
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

            auto newShrinkable = make_shrinkable<shrinkable_vector_t>(newVec);
            newShrinkable = newShrinkable.with(
                [newShrinkable, _power, _offset]() -> stream_t { return shrinkBulk(newShrinkable, _power, _offset); });
            return stream_t(newShrinkable,
                            [_ancestor, _power, _offset, newShrinkable, frompos, topos, newElemStreams]() -> stream_t {
                                return genStream(_ancestor, _power, _offset, newShrinkable, frompos, topos,
                                                 newElemStreams);
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
        shrinkable_vector_t& parentVec = ancestor.getRef();
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
            size_t _vecSize = shr.getRef().size();
            size_t _numSplits = static_cast<size_t>(std::pow(2, power));
            if (_vecSize / _numSplits < 1 || offset >= _numSplits)
                return stream_t::empty();
            // std::cout << "entire: " << power << ", " << offset << std::endl;
            return shrinkBulk(shr, power, offset);
        });

        return newShrinkable.shrinks();
    }

};

} // namespace util


template <template <typename...> class ListLike, typename T>
Shrinkable<ListLike<T>> shrinkListLike(const std::shared_ptr<std::vector<Shrinkable<T>>>& shrinkableVector, size_t minSize) {
    using shrinkable_vector_t = std::vector<Shrinkable<T>>;
    using shrinkable_t = Shrinkable<shrinkable_vector_t>;

    size_t size = shrinkableVector->size();
    // shrink list size with sub-list using binary numeric shrink of sizes
    auto rangeShrinkable =
        util::binarySearchShrinkableU(size - minSize)
            .template map<size_t>([minSize](const uint64_t& _size) -> size_t { return _size + minSize; });
    // this make sure shrinking is possible towards minSize
    shrinkable_t shrinkable = rangeShrinkable.template flatMap<shrinkable_vector_t>(
        [shrinkableVector](const size_t& _size) -> Shrinkable<shrinkable_vector_t> {
            if (_size <= 0)
                return make_shrinkable<shrinkable_vector_t>();

            auto begin = shrinkableVector->begin();
            auto last = shrinkableVector->begin() + _size;  // sub-list of (0, size)
            return make_shrinkable<shrinkable_vector_t>(begin, last);
        });

    shrinkable =
        shrinkable.andThen(+[](const shrinkable_t& shr) { return util::VectorShrinker<T>::shrinkBulkRecursive(shr, 0, 0); });

    auto listLikeShrinkable =
        shrinkable.template flatMap<ListLike<T>>(+[](const shrinkable_vector_t& _shrinkableVector) -> Shrinkable<ListLike<T>> {
            auto value = make_shrinkable<ListLike<T>>();
            ListLike<T>& valueVec = value.getRef();
            std::transform(
                _shrinkableVector.begin(), _shrinkableVector.end(), std::back_inserter(valueVec),
                +[](const Shrinkable<T>& shr) -> T { return shr.getRef(); });
            return value;
        });

    return listLikeShrinkable;
}

} // namespace proptes
