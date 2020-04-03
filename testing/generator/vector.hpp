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

    Shrinkable<std::vector<T>> operator()(Random& rand) {
        int len = rand.getRandomSize(0, maxLen+1);
        std::vector<Shrinkable<T>> shrinkVec;
        shrinkVec.reserve(len);
        for(int i = 0; i < len; i++)
            shrinkVec.push_back(elemGen(rand));

        // std::vector<T> vec;
        // for(int i = 0; i < len; i++)
        //     vec.push_back(elemGen(rand));

        // return binarySearchShrinkable<int>(len).template transform<std::vector<T>>([vec](const int& len) {
        //     if(len <= 0)
        //         return std::vector<T>();

        //     auto begin = vec.begin();
        //     auto last = vec.begin() + len;
        //     return std::vector<T>(begin, last);;
        // });

        // shrink with subvector using binary numeric shrink of lengths
        shrinkable_t shrinkableVecShrinkable =  binarySearchShrinkable<int>(len).template transform<std::vector<Shrinkable<T>>>([shrinkVec](const int& len) {
            if(len <= 0)
                return std::vector<Shrinkable<T>>();

            auto begin = shrinkVec.begin();
            auto last = shrinkVec.begin() + len;
            return std::vector<Shrinkable<T>>(begin, last);
        });

        // concat shrinks with parent as argument
        const auto maxSize = shrinkableVecShrinkable.getRef().size();
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
            shrinkableVecShrinkable = shrinkableVecShrinkable.concat(genStream(i));
        }

        auto vecShrinkable = shrinkableVecShrinkable.template transform<std::vector<T>>([](const std::vector<Shrinkable<T>>& shrinkVec) -> std::vector<T> {
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



