#pragma once

#include "testing/gen.hpp"
#include "testing/Random.hpp"
#include "testing/printing.hpp"
#include "testing/generator/util.hpp"
#include <vector>
#include <iostream>
#include <cmath>

namespace PropertyBasedTesting
{

template <typename T>
class PROPTEST_API Arbitrary< std::vector<T>> : public Gen< std::vector<T> >
{
public:
    static size_t defaultMinLen;
    static size_t defaultMaxLen;

    Arbitrary() : elemGen(Arbitrary<T>()), minLen(defaultMinLen), maxLen(defaultMaxLen)  {
    }

    Arbitrary(Arbitrary<T> _elemGen)
     : elemGen([_elemGen](Random& rand)->Shrinkable<T>{ return _elemGen(rand); })
     , minLen(defaultMinLen)
     , maxLen(defaultMaxLen) {
    }

    Arbitrary(std::function<Shrinkable<T>(Random&)> _elemGen) : elemGen(_elemGen), minLen(defaultMinLen), maxLen(defaultMaxLen)  {
    }

    using vector_t = std::vector<Shrinkable<T>>;
    using shrinkable_t = Shrinkable<vector_t>;
    using stream_t = Stream<shrinkable_t>;
    using element_t = T;
    using e_shrinkable_t = Shrinkable<element_t>;
    using e_stream_t = Stream<e_shrinkable_t>;

    static stream_t shrinkBulk(const shrinkable_t& parent, size_t power, size_t offset) {

        static std::function<stream_t(size_t, size_t, const std::vector<e_stream_t>&)> genStream = [parent, power, offset](size_t frompos, size_t topos, const std::vector<e_stream_t>& elemStreams) {
            const size_t size = topos - frompos;
            if(size == 0)
                return stream_t::empty();

            vector_t newVec = vector_t();
                newVec.reserve(size);
            std::vector<e_stream_t> newElemStreams;
                newElemStreams.reserve(size);

            vector_t& parentVec = parent.getRef();

            // shrink each element in frompos~topos, put parent if shrink no longer possible
            bool nothingTodo = true;
            for(size_t i = 0; i < elemStreams.size(); i++) {
                if(elemStreams[i].isEmpty()) {
                    newVec.push_back(parentVec[i]);
                    newElemStreams.push_back(e_stream_t::empty());  // [1] -> []
                }
                else {
                    newVec.push_back(elemStreams[i].head());
                    newElemStreams.push_back(elemStreams[i].tail()); // [0,4,6,7] -> [4,6,7]
                    nothingTodo = false;
                }
            }
            if(nothingTodo)
                return stream_t::empty();

            auto newShrinkable = make_shrinkable<vector_t>(newVec);
            // newShrinkable = newShrinkable.with([newShrinkable, power, offset]() {
            //     return shrinkBulk(newShrinkable, power, offset);
            // });
            return stream_t(newShrinkable, [frompos, topos, newElemStreams]() {
                return genStream(frompos, topos, newElemStreams);
            });
        };

        size_t parentSize = parent.getRef().size();
        size_t numSplits = std::pow(2,power);
        if(parentSize / numSplits < 1)
            return stream_t::empty();

        if(offset >= numSplits)
            throw std::runtime_error("offset should not reach numSplits");

        size_t frompos = parentSize * offset / numSplits;
        size_t topos = parentSize * (offset+1) / numSplits;

        const size_t size = topos - frompos;
        vector_t& parentVec = parent.getRef();
        std::vector<e_stream_t> elemStreams;
            elemStreams.reserve(size);
        for(size_t i = frompos; i < topos; i++) {
            elemStreams.push_back(parentVec[i]);
        }

        return genStream(frompos, topos, elemStreams);
    }

    static stream_t shrinkBulkRecursive(const shrinkable_t& shrinkable, size_t power, size_t offset) {
        if(shrinkable.getRef().empty())
            return stream_t::empty();

        size_t vecSize = shrinkable.getRef().size();
        size_t numSplits = std::pow(2, power);
        if(vecSize / numSplits < 1 || offset >= numSplits)
            return stream_t::empty();
        // entirety
        shrinkable_t newShrinkable = shrinkable.concat([power, offset](const shrinkable_t& shr) {
            size_t vecSize = shr.getRef().size();
            size_t numSplits = std::pow(2, power);
            if(vecSize / numSplits < 1 || offset >= numSplits)
                return stream_t::empty();
            return shrinkBulk(shr, power, offset);
        });

        // front part
        newShrinkable = newShrinkable.concat([power, offset](const shrinkable_t& shr) {
            size_t vecSize = shr.getRef().size();
            size_t numSplits = std::pow(2, power+1);
            if(vecSize / numSplits < 1 || offset*2 >= numSplits)
                return stream_t::empty();
            return shrinkBulkRecursive(shr, power+1, offset*2);
        });

        // rear part
        newShrinkable = newShrinkable.concat([power, offset](const shrinkable_t& shr) {
            size_t vecSize = shr.getRef().size();
            size_t numSplits = std::pow(2, power+1);
            if(vecSize / numSplits < 1 || offset*2+1 >= numSplits)
                return stream_t::empty();
            return shrinkBulkRecursive(shr, power+1, offset*2+1);
        });

        return newShrinkable.shrinks();
    }

    Shrinkable<std::vector<T>> operator()(Random& rand) {
        int len = rand.getRandomSize(minLen, maxLen+1);
        std::vector<Shrinkable<T>> shrinkVec;
        shrinkVec.reserve(len);
        for(int i = 0; i < len; i++)
            shrinkVec.push_back(elemGen(rand));

        // shrink vector size with subvector using binary numeric shrink of lengths
        int minLenCopy = minLen;
        auto rangeShrinkable = binarySearchShrinkable<int>(len - minLenCopy).template transform<int>([minLenCopy](const int& len) {
            return len + minLenCopy;
        });
        // this make sure shrinking is possible towards minLen
        shrinkable_t shrinkable = rangeShrinkable.template transform<std::vector<Shrinkable<T>>>([shrinkVec](const int& len) {
            if(len <= 0)
                return std::vector<Shrinkable<T>>();

            auto begin = shrinkVec.begin();
            auto last = shrinkVec.begin() + len; // subvector of (0, len)
            return std::vector<Shrinkable<T>>(begin, last);
        });

        // concat shrinks with parent as argument
        // const auto maxSize = shrinkable.getRef().size();
        // auto genStream = [](size_t i) {
        //     return [i](const shrinkable_t& parent) {
        //         vector_t parentRef = parent.getRef();
        //         const size_t size = parentRef.size();

        //         if(size == 0 || size - 1 < i)
        //             return stream_t::empty();

        //         size_t pos = size - 1 - i;
        //         e_shrinkable_t& elem = parentRef[pos];
        //         // {0,2,3} to {[x,x,x,0], ...,[x,x,x,3]}
        //         // make sure {1} shrinked from 2 is also transformed to [x,x,x,1]
        //         shrinkable_t vecWithElems = elem.template transform<vector_t>([pos, parentRef](const element_t& val) {
        //             auto copy = parentRef;
        //             copy[pos] = make_shrinkable<element_t>(val);
        //             return copy;
        //         });
        //         shrinkable_t cropped = vecWithElems;//.take(2);
        //         return cropped.shrinks();
        //     };
        // };

        // for(size_t i = 0; i < maxSize; i++) {
        //     shrinkable = shrinkable.concat(genStream(i));
        // }

        shrinkable = shrinkable.concat([](const shrinkable_t& shr) {
            return shrinkBulkRecursive(shr, 0, 0);
        });

        auto vecShrinkable = shrinkable.template transform<std::vector<T>>([](const std::vector<Shrinkable<T>>& shrinkVec) -> std::vector<T> {
            std::vector<T> valueVec;
            std::transform(shrinkVec.begin(), shrinkVec.end(), std::back_inserter(valueVec), [](const Shrinkable<T>& shr) -> T {
                return shr.get();
            });
            return valueVec;
        });

        return vecShrinkable;
    }

    std::function<Shrinkable<T>(Random&)> elemGen;
    int minLen;
    int maxLen;

};

template <typename T>
size_t Arbitrary<std::vector<T>>::defaultMinLen = 0;
template <typename T>
size_t Arbitrary<std::vector<T>>::defaultMaxLen = 200;


} // namespace PropertyBasedTesting



