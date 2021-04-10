#pragma once
#include "../Shrinkable.hpp"
#include "../util/std.hpp"
#include "../generator/util.hpp"

namespace proptest {

namespace util {

template <typename T>
struct VectorShrinker
{
    using shrinkable_vector_t = vector<Shrinkable<T>>;
    using shrinkable_t = Shrinkable<shrinkable_vector_t>;
    using stream_t = Stream<shrinkable_t>;
    using e_stream_t = Stream<Shrinkable<T>>;

    static stream_t shrinkBulk(const shrinkable_t& ancestor, size_t power, size_t offset)
    {
        static function<stream_t(const shrinkable_t&, size_t, size_t, const shrinkable_t&, size_t, size_t,
                                 shared_ptr<vector<e_stream_t>>)>
            genStream = +[](const shrinkable_t& _ancestor, size_t _power, size_t _offset, const shrinkable_t& parent,
                            size_t frompos, size_t topos, shared_ptr<vector<e_stream_t>> elemStreams) -> stream_t {
            const size_t size = topos - frompos;
            if (size == 0)
                return stream_t::empty();

            if (elemStreams->size() != size)
                throw runtime_error("element streams size error");

            shared_ptr<vector<e_stream_t>> newElemStreams = util::make_shared<vector<e_stream_t>>();
            newElemStreams->reserve(size);

            shrinkable_vector_t newVec = parent.getRef();
            shrinkable_vector_t& ancestorVec = _ancestor.getRef();

            if (newVec.size() != ancestorVec.size())
                throw runtime_error("list size error: " + to_string(newVec.size()) +
                                    " != " + to_string(ancestorVec.size()));

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
        size_t numSplits = static_cast<size_t>(pow(2, power));
        if (parentSize / numSplits < 1)
            return stream_t::empty();

        if (offset >= numSplits)
            throw runtime_error("offset should not reach numSplits");

        size_t frompos = parentSize * offset / numSplits;
        size_t topos = parentSize * (offset + 1) / numSplits;

        if (topos < parentSize)
            throw runtime_error("topos error: " + to_string(topos) + " != " + to_string(parentSize));

        const size_t size = topos - frompos;
        shrinkable_vector_t& parentVec = ancestor.getRef();
        shared_ptr<vector<e_stream_t>> elemStreams = util::make_shared<vector<e_stream_t>>();
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
        size_t numSplits = static_cast<size_t>(pow(2, power));
        if (vecSize / numSplits < 1 || offset >= numSplits)
            return stream_t::empty();
        // entirety
        shrinkable_t newShrinkable = shrinkable.concat([power, offset](const shrinkable_t& shr) -> stream_t {
            size_t _vecSize = shr.getRef().size();
            size_t _numSplits = static_cast<size_t>(pow(2, power));
            if (_vecSize / _numSplits < 1 || offset >= _numSplits)
                return stream_t::empty();
            // cout << "entire: " << power << ", " << offset << endl;
            return shrinkBulk(shr, power, offset);
        });

        return newShrinkable.shrinks();
    }
};

}  // namespace util

template <template <typename...> class ListLike, typename T>
Shrinkable<ListLike<T>> shrinkListLike(const shared_ptr<vector<Shrinkable<T>>>& shrinkableVector, size_t minSize)
{
    using shrinkable_vector_t = vector<Shrinkable<T>>;
    using shrinkable_t = Shrinkable<shrinkable_vector_t>;

    size_t size = shrinkableVector->size();
    // shrink list size with sub-list using binary numeric shrink of sizes
    auto rangeShrinkable =
        util::binarySearchShrinkableU(size - minSize).template map<size_t>([minSize](const uint64_t& _size) -> size_t {
            return _size + minSize;
        });
    // this make sure shrinking is possible towards minSize
    shrinkable_t shrinkable = rangeShrinkable.template flatMap<shrinkable_vector_t>(
        [shrinkableVector](const size_t& _size) -> Shrinkable<shrinkable_vector_t> {
            if (_size <= 0)
                return make_shrinkable<shrinkable_vector_t>();

            auto begin = shrinkableVector->begin();
            auto last = shrinkableVector->begin() + _size;  // sub-list of (0, size)
            return make_shrinkable<shrinkable_vector_t>(begin, last);
        });

    shrinkable = shrinkable.andThen(
        +[](const shrinkable_t& shr) { return util::VectorShrinker<T>::shrinkBulkRecursive(shr, 0, 0); });

    auto listLikeShrinkable = shrinkable.template flatMap<ListLike<T>>(
        +[](const shrinkable_vector_t& _shrinkableVector) -> Shrinkable<ListLike<T>> {
            auto value = make_shrinkable<ListLike<T>>();
            ListLike<T>& valueVec = value.getRef();
            transform(
                _shrinkableVector.begin(), _shrinkableVector.end(), back_inserter(valueVec),
                +[](const Shrinkable<T>& shr) -> T { return shr.getRef(); });
            return value;
        });

    return listLikeShrinkable;
}

template <template <typename...> class ListLike, typename T>
Shrinkable<ListLike<Shrinkable<T>>> shrinkListLikeLength(const shared_ptr<ListLike<Shrinkable<T>>>& shrinkableElems,
                                                         size_t minSize)
{
    auto size = shrinkableElems->size();
    auto rangeShrinkable =
        util::binarySearchShrinkable(size - minSize).template map<size_t>([minSize](const size_t& s) { return s + minSize; });
    return rangeShrinkable.template map<ListLike<Shrinkable<T>>>([shrinkableElems](const size_t& newSize) {
        if (newSize == 0)
            return ListLike<Shrinkable<T>>();
        else
            return ListLike<Shrinkable<T>>(shrinkableElems->begin(), shrinkableElems->begin() + newSize);
    });
}

}  // namespace proptest
