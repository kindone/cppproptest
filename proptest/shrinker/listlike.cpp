#include "listlike.hpp"

namespace proptest {

namespace util {

VectorShrinker::stream_t VectorShrinker::shrinkBulk(const VectorShrinker::shrinkable_t& ancestor, size_t power, size_t offset)
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
                newVec[i + frompos] = ShrinkableAny(ancestorVec[i + frompos]);
                newElemStreams->push_back(e_stream_t::empty());  // [1] -> []
            } else {
                newVec[i + frompos] = (*elemStreams)[i].head<ShrinkableAny>();
                newElemStreams->push_back((*elemStreams)[i].tail());  // [0,4,6,7] -> [4,6,7]
                nothingToDo = false;
            }
        }
        if (nothingToDo)
            return stream_t::empty();

        auto newShrinkable = make_shrinkable<shrinkable_vector_t>(newVec);
        newShrinkable = newShrinkable.with(
            [newShrinkable, _power, _offset]() -> stream_t { return shrinkBulk(newShrinkable, _power, _offset); });
        return stream_t(ShrinkableAny(newShrinkable),
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

VectorShrinker::stream_t VectorShrinker::shrinkElementwise(const VectorShrinker::shrinkable_t& shrinkable, size_t power, size_t offset)
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

VectorShrinker::shrinkable_t VectorShrinker::shrinkMid(shared_ptr<vector<ShrinkableAny>> shrinkableCont, size_t minSize, size_t frontSize, size_t rearSize) {
    // remove mid as much as possible
    size_t minRearSize = minSize >= frontSize ? minSize - frontSize : 0;
    size_t maxRearSize = shrinkableCont->size() - frontSize;
    // rear size within [minRearSize, minRearSize]
    auto rangeShrinkable = util::binarySearchShrinkable(maxRearSize - minRearSize).template map<size_t>([minRearSize](const size_t& s) { return s + minRearSize; });
    return rangeShrinkable.template flatMap<vector<ShrinkableAny>>([shrinkableCont, frontSize](const size_t& rearSize) {
        // concat front and rear
        auto cont = util::make_shared<Any>(util::make_any<vector<ShrinkableAny>>(shrinkableCont->begin(), shrinkableCont->begin() + frontSize));
        auto& contRef = cont->cast<vector<ShrinkableAny>>();
        contRef.insert(contRef.end(), shrinkableCont->begin() + (contRef.size()-rearSize), shrinkableCont->end());
        return Shrinkable<vector<ShrinkableAny>>(cont);
    }).concat([minSize, frontSize, rearSize](const shrinkable_t& parent) {
        size_t parentSize = parent.getRef().size();
        // no further shrinking possible
        if(parentSize <= minSize || parentSize <= frontSize)
            return Stream::empty();
        return shrinkMid(parent.getSharedPtr(), minSize, frontSize + 1, rearSize).shrinks();
    });
}

VectorShrinker::shrinkable_t VectorShrinker::shrinkFrontAndThenMid(shared_ptr<vector<ShrinkableAny>> shrinkableCont, size_t minSize, size_t rearSize) {
    // remove front as much as possible
    size_t minFrontSize = minSize >= rearSize ? minSize - rearSize : 0;
    size_t maxFrontSize = shrinkableCont->size() - rearSize;
    // front size within [min,max]
    auto rangeShrinkable = util::binarySearchShrinkable(maxFrontSize - minFrontSize).template map<size_t>([minFrontSize](const size_t& s) { return s + minFrontSize; });
    return rangeShrinkable.template flatMap<vector<ShrinkableAny>>([shrinkableCont, maxFrontSize](const size_t& frontSize) {
        // concat front and rear
        auto cont = util::make_shared<Any>(util::make_any<vector<ShrinkableAny>>(shrinkableCont->begin(), shrinkableCont->begin() + frontSize));
        auto& contRef = cont->cast<vector<ShrinkableAny>>();
        contRef.insert(contRef.end(), shrinkableCont->begin() + maxFrontSize, shrinkableCont->end());
        return Shrinkable<vector<ShrinkableAny>>(cont);
    }).concat([minSize, rearSize](const shrinkable_t& parent) {
        // reduce front [0,size-rearSize-1] as much possible
        size_t parentSize = parent.getRef().size();
        // no further shrinking possible
        if(parentSize <= minSize || parentSize <= rearSize) {
            // try shrinking mid
            if(minSize < parentSize && rearSize + 1 < parentSize)
                return shrinkMid(parent.getSharedPtr(), minSize, 1, rearSize + 1).shrinks();
            else
                return Stream::empty();
        }
        // shrink front further by fixing last element in front to rear
        // [1,[2,3,4]]
        // [[1,2,3],4]
        // [[1,2],3,4]
        return shrinkFrontAndThenMid(parent.getSharedPtr(), minSize, rearSize + 1).shrinks();
    });
}

} // namespace util

Shrinkable<vector<ShrinkableAny>> shrinkMembershipwise(const shared_ptr<vector<ShrinkableAny>>& shrinkableCont, size_t minSize) {
    return util::VectorShrinker::shrinkFrontAndThenMid(shrinkableCont, minSize, 0);
}

} // namespace proptest
