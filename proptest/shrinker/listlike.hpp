#pragma once
#include "../Shrinkable.hpp"
#include "../util/std.hpp"
#include "../generator/util.hpp"

namespace proptest {

namespace util {

struct VectorShrinker
{
    using shrinkable_vector_t = vector<ShrinkableAny>;
    using shrinkable_t = Shrinkable<shrinkable_vector_t>;
    using stream_t = Stream;
    using e_stream_t = Stream;

    static stream_t shrinkBulk(const shrinkable_t& ancestor, size_t power, size_t offset);

    static stream_t shrinkElementwise(const shrinkable_t& shrinkable, size_t power, size_t offset);

    static shrinkable_t shrinkMid(shared_ptr<vector<ShrinkableAny>> shrinkableCont, size_t minSize, size_t frontSize, size_t rearSize);

    static shrinkable_t shrinkFrontAndThenMid(shared_ptr<vector<ShrinkableAny>> shrinkableCont, size_t minSize, size_t rearSize);

};

}  // namespace util


PROPTEST_API Shrinkable<vector<ShrinkableAny>> shrinkMembershipwise(const shared_ptr<vector<ShrinkableAny>>& shrinkableCont, size_t minSize);

/**
 * @brief Shrinking of a container (such as a set) using membership-wise shrinking
 *
 *  * Membership-wise shrinking searches through inclusion or exclusion of elements in the container
 * @tparam Container A container such as vector or set
 * @tparam T Contained type
 * @param shrinkableCont container of Shrinkable<T>
 * @param minSize minimum size a shrunk list can be
 * @param elementwise whether to enable element-wise shrinking. If false, only membership-wise shrinking is performed
 * @return Shrinkable<ListLike<T>>
 */
template <template <typename...> class Container, typename T>
Shrinkable<Container<T>> shrinkContainer(const shared_ptr<Container<Shrinkable<T>>>& shrinkableCont, size_t minSize)
{
    // change type to any
    shared_ptr<vector<ShrinkableAny>> shrinkAnyVec = util::make_shared<vector<ShrinkableAny>>();
    util::transform(shrinkableCont->begin(), shrinkableCont->end(), util::back_inserter(*shrinkAnyVec), [](Shrinkable<T> shr) -> ShrinkableAny {
        return shr;
    });
    // membershipwise shrinking
    Shrinkable<vector<ShrinkableAny>> shrinkableElemsShr = shrinkMembershipwise(shrinkAnyVec, minSize);

    // transform to proper output type
    return shrinkableElemsShr.template flatMap<Container<T>>(
        +[](const vector<ShrinkableAny>& _shrinkableVector) -> Shrinkable<Container<T>> {
            auto value = make_shrinkable<Container<T>>();
            Container<T>& valueCont = value.getRef();
            for(auto itr = _shrinkableVector.begin(); itr != _shrinkableVector.end(); ++itr) {
                valueCont.insert(itr->getAnyRef().cast<T>());
            }
            return value;
        });
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
Shrinkable<ListLike<T>> shrinkListLike(const shared_ptr<vector<ShrinkableAny>>& shrinkAnyVec, size_t minSize, bool elementwise = true)
{
    // membershipwise shrinking
    Shrinkable<vector<ShrinkableAny>> shrinkableElemsShr = shrinkMembershipwise(shrinkAnyVec, minSize);

    // elementwise shrinking
    if(elementwise)
        shrinkableElemsShr = shrinkableElemsShr.andThen(+[](const Shrinkable<vector<ShrinkableAny>>& parent) {
            return util::VectorShrinker::shrinkElementwise(parent, 0, 0);
        });

    // transform to proper output type
    return shrinkableElemsShr.template flatMap<ListLike<T>>(
        +[](const vector<ShrinkableAny>& _shrinkAnyVec) -> Shrinkable<ListLike<T>> {
            auto value = make_shrinkable<ListLike<T>>();
            ListLike<T>& valueVec = value.getRef();
            util::transform(
                _shrinkAnyVec.begin(), _shrinkAnyVec.end(), util::back_inserter(valueVec),
                +[](const ShrinkableAny& shr) -> T { return shr.getAnyRef().cast<T>(); });
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
