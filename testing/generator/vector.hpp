#pragma once

#include "testing/gen.hpp"
#include "testing/Random.hpp"
#include "testing/printing.hpp"
#include "testing/generator/util.hpp"
#include <vector>
#include <iostream>

namespace PropertyBasedTesting
{

template <typename T>
class PROPTEST_API Arbitrary< std::vector<T>> : public Gen< std::vector<T> >
{
public:
    static size_t defaultMaxLen;

    Arbitrary() : elemGen(Arbitrary<T>()), maxLen(defaultMaxLen)  {
    }

    Arbitrary(Arbitrary<T> _elemGen)
     : elemGen([_elemGen](Random& rand)->Shrinkable<T>{ return _elemGen(rand); })
     , maxLen(defaultMaxLen)  {
    }

    Arbitrary(std::function<Shrinkable<T>(Random&)> _elemGen) : elemGen(_elemGen), maxLen(defaultMaxLen)  {
    }

    using vector_t = std::vector<Shrinkable<T>>;
    using shrinkable_t = Shrinkable<vector_t>;
    using stream_t = Stream<shrinkable_t>;
    using element_t = T;
    using e_shrinkable_t = Shrinkable<element_t>;
    using e_stream_t = Stream<e_shrinkable_t>;

    static stream_t shrinkBulk(const shrinkable_t& parent, size_t frompos, size_t topos) {
        static std::function<stream_t(const std::vector<e_stream_t>&)> genStream = [parent, frompos, topos](const std::vector<e_stream_t>& elemStreams) {
            const size_t size = topos - frompos;
            vector_t newVec = vector_t();
                newVec.reserve(size);
            std::vector<e_stream_t> newElemStreams;
                newElemStreams.reserve(size);

            vector_t& parentVec = parent.getRef();

            // shrink each element in frompos~topos, put parent if shrink no longer possible
            for(size_t i = 0; i < elemStreams.size(); i++) {
                if(elemStreams[i].isEmpty()) {
                    newVec.push_back(parentVec[i]);
                    newElemStreams.push_back(e_stream_t::empty());  // [1] -> []
                }
                else {
                    newVec.push_back(elemStreams[i].head());
                    newElemStreams.push_back(elemStreams[i].tail()); // [0,4,6,7] -> [4,6,7]
                }
            }
            auto newShrinkable = make_shrinkable<vector_t>(newVec);
            newShrinkable = newShrinkable.with([newShrinkable, frompos, topos]() {
                return shrinkBulk(newShrinkable, frompos, topos);
            });
            return stream_t(newShrinkable, [newElemStreams]() {
                return genStream(newElemStreams);
            });
        };

        const size_t size = topos - frompos;
        vector_t& parentVec = parent.getRef();
        std::vector<e_stream_t> elemStreams;
            elemStreams.reserve(size);
        for(size_t i = frompos; i < topos; i++) {
            elemStreams.push_back(parentVec[i]);
        }

        return genStream(elemStreams);
    }

    static shrinkable_t shrinkBulkRecursive(const shrinkable_t& shrinkable, size_t frompos, size_t topos) {
        if(frompos >= topos) {
            return shrinkable;
        }

        // entire
        shrinkable_t newShrinkable = shrinkable.concat([frompos, topos](const shrinkable_t& shr) {
            auto vecSize = shr.getRef().size();
            if(frompos >= vecSize)
                return stream_t::empty();
            else if(topos > vecSize)
                return shrinkBulk(shr, frompos, vecSize);
            else
                return shrinkBulk(shr, frompos, topos);
        });

        size_t midpos = frompos/2 + topos/2 + ((frompos % 2 != 0 && topos % 2 != 0) ? 1 : 0);

        // front part
        if(frompos < midpos) {
            newShrinkable = newShrinkable.concat([frompos, midpos](const shrinkable_t& shr) {
                return shrinkBulkRecursive(shr, frompos, midpos);
            });
        }

        // rear part
        if(midpos < topos) {
            newShrinkable = newShrinkable.concat([midpos, topos](const shrinkable_t& shr) {
                return shrinkBulkRecursive(shr, midpos, topos);
            });
        }

        return newShrinkable;
    }

    Shrinkable<std::vector<T>> operator()(Random& rand) {
        int len = rand.getRandomSize(0, maxLen+1);
        std::vector<Shrinkable<T>> shrinkVec;
        shrinkVec.reserve(len);
        for(int i = 0; i < len; i++)
            shrinkVec.push_back(elemGen(rand));

        // shrink vector size with subvector using binary numeric shrink of lengths
        shrinkable_t shrinkable = binarySearchShrinkable<int>(len).template transform<std::vector<Shrinkable<T>>>([shrinkVec](const int& len) {
            if(len <= 0)
                return std::vector<Shrinkable<T>>();

            auto begin = shrinkVec.begin();
            auto last = shrinkVec.begin() + len; // subvector of (0, len)
            return std::vector<Shrinkable<T>>(begin, last);
        });

        // concat shrinks with parent as argument
        const auto maxSize = shrinkable.getRef().size();
        auto genStream = [](size_t i) {
            return [i](const shrinkable_t& parent) {
                vector_t parentRef = parent.getRef();
                const size_t size = parentRef.size();

                if(size == 0 || size - 1 < i)
                    return stream_t::empty();

                size_t pos = size - 1 - i;
                e_shrinkable_t& elem = parentRef[pos];
                // {0,2,3} to {[x,x,x,0], ...,[x,x,x,3]}
                // make sure {1} shrinked from 2 is also transformed to [x,x,x,1]
                shrinkable_t vecWithElems = elem.template transform<vector_t>([pos, parentRef](const element_t& val) {
                    auto copy = parentRef;
                    copy[pos] = make_shrinkable<element_t>(val);
                    return copy;
                });
                shrinkable_t cropped = vecWithElems;//.take(2);
                return cropped.shrinks();
            };
        };

        for(size_t i = 0; i < maxSize; i++) {
            shrinkable = shrinkable.concat(genStream(i));
        }

        shrinkable = shrinkable.concat([](const shrinkable_t& shr) {
            auto vecSize = shr.getRef().size();
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
    int maxLen;
};

template <typename T>
size_t Arbitrary<std::vector<T>>::defaultMaxLen = 200;


} // namespace PropertyBasedTesting



