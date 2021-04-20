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

    static stream_t shrinkElementwise(const shrinkable_t& shrinkable, size_t power, size_t offset)
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

    static shrinkable_t shrinkMid(shared_ptr<vector<Shrinkable<T>>> shrinkableVector, size_t minSize, size_t maxSize) {
        // if maxSize < size, rear exists (rear is fixed)
        // remove mid as much as possible
        size_t rearSize = shrinkableVector->size() - maxSize;
        // front size within [min,max]
        auto rangeShrinkable = util::binarySearchShrinkable(maxSize - minSize).template map<size_t>([minSize](const size_t& s) { return s + minSize; });
        return rangeShrinkable.template flatMap<vector<Shrinkable<T>>>([shrinkableVector, minSize, maxSize, rearSize](const size_t& size) {
            // concat front and rear
            auto vec = util::make_shared<vector<Shrinkable<T>>>(shrinkableVector->begin(), shrinkableVector->begin() + size);
            vec->insert(vec->end(), shrinkableVector->begin() + maxSize, shrinkableVector->end());
            return Shrinkable<vector<Shrinkable<T>>>(vec);
        // NOTE: use of andThen instead of concat reduces diversity and converges faster
        }).concat([minSize, rearSize](const shrinkable_t& parent) {
            // reduce front [0,size-rearSize-1] as much possible
            size_t size = parent.getRef().size();
            // no further shrinking possible
            if(size <= rearSize + 1 || size <= minSize + rearSize)
                return Stream<Shrinkable<vector<Shrinkable<T>>>>::empty();
            // shrink front further by fixing last element in front to rear
            // [1,[2,3,4]]
            // [[1,2,3],4]
            // [[1,2],3,4]
            size_t newRearSize = rearSize + 1;
            return shrinkMid(parent.getSharedPtr(), (minSize > 1) ? (minSize - 1) : 0, size - newRearSize).shrinks();
        });
    }
};

}  // namespace util


template <template <typename...> class ListLike, typename T>
Shrinkable<vector<Shrinkable<T>>> shrinkMembershipwise(const shared_ptr<vector<Shrinkable<T>>>& shrinkableVector, size_t minSize, size_t maxSize) {
    return util::VectorShrinker<T>::shrinkMid(shrinkableVector, minSize, maxSize);
}

/**
 * @brief Shrinking of list-like container using membership-wise and element-wise shrinking
 *
 *  * Membership-wise shrinking searches through inclusion or exclusion of elements in the container
 *  * Element-wise shrinking tries to shrink the elements themselves (e.g. shrink integer elements in vector<int>)
 * @tparam ListLike A list-like container such as vector or list
 * @tparam T Contained type
 * @param shrinkableVector vector of Shrinkable<T>
 * @param minSize minimum size a shrunk list can be
 * @param elementwise whether to enable element-wise shrinking. If false, only membership-wise shrinking is performed
 * @return Shrinkable<ListLike<T>>
 */
template <template <typename...> class ListLike, typename T>
Shrinkable<ListLike<T>> shrinkListLike(const shared_ptr<vector<Shrinkable<T>>>& shrinkableVector, size_t minSize, bool elementwise = false)
{
    // membershipwise shrinking
    Shrinkable<vector<Shrinkable<T>>> shrinkableElemsShr = shrinkMembershipwise<ListLike, T>(shrinkableVector, minSize, shrinkableVector->size());

    // elementwise shrinking
    if(elementwise)
        shrinkableElemsShr = shrinkableElemsShr.andThen(+[](const Shrinkable<vector<Shrinkable<T>>>& parent) {
            return util::VectorShrinker<T>::shrinkElementwise(parent, 0, 0);
        });

    // transform to proper output type
    return shrinkableElemsShr.template flatMap<ListLike<T>>(
        +[](const vector<Shrinkable<T>>& _shrinkableVector) -> Shrinkable<ListLike<T>> {
            auto value = make_shrinkable<ListLike<T>>();
            ListLike<T>& valueVec = value.getRef();
            transform(
                _shrinkableVector.begin(), _shrinkableVector.end(), back_inserter(valueVec),
                +[](const Shrinkable<T>& shr) -> T { return shr.getRef(); });
            return value;
        });
}

/**
 * @brief Simple shrinking of list-like containers into sublists within minSize and the given list size
 *
 * @param shrinkableElems Shrinkable<T>
 * @param minSize minimum size the shrunk container can be
 * @return Shrinkable<ListLike<T>>
 */
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
