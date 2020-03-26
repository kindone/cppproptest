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

    Arbitrary() : maxLen(defaultMaxLen)  {
    }

    Arbitrary(Arbitrary<T> _elemGen) : elemGen(_elemGen), maxLen(defaultMaxLen)  {
    }

    static Stream<Shrinkable<std::vector<Shrinkable<T>> >> shrinkElement(const Shrinkable<std::vector<Shrinkable<T>>>& shrinkableVecShrinkable, size_t pos) {
        std::vector<Shrinkable<T>> shrinkableVec = shrinkableVecShrinkable.getRef();
        // replace with this one
        Stream<Shrinkable<T>> streamShrinkableT = shrinkableVec[pos].shrinks();
        //replace  with new one based on this shrinks
        Stream<Shrinkable<std::vector<Shrinkable<T>>>> streamShrinkableVectorShrinkable = streamShrinkableT.template transform<Shrinkable<std::vector<Shrinkable<T>>>>([shrinkableVecShrinkable,pos](const Shrinkable<T>& shr) {
            Shrinkable<std::vector<Shrinkable<T>>> copy = shrinkableVecShrinkable;
            return copy.template transform<std::vector<Shrinkable<T>>>([shr,pos](const std::vector<Shrinkable<T>>& s){
                auto c = s;
                c[pos] = shr;
                return c;
            });
        });
        return streamShrinkableVectorShrinkable;
    }

    Shrinkable<std::vector<T>> operator()(Random& rand) {
        int len = rand.getRandomSize(0, maxLen+1);
        std::vector<Shrinkable<T>> shrinkVec;
        shrinkVec.reserve(len);
        for(int i = 0; i < len; i++)
            shrinkVec.push_back(elemGen(rand));

        std::vector<T> vec;
        for(int i = 0; i < len; i++)
            vec.push_back(elemGen(rand));

        return binarySearchShrinkable<int>(len).template transform<std::vector<T>>([vec](const int& len) {
            if(len <= 0)
                return std::vector<T>();

            auto begin = vec.begin();
            auto last = vec.begin() + len;
            return std::vector<T>(begin, last);;
        });

        // shrink with subvector using binary numeric shrink of lengths
        auto shrinkableVecShrinkable =  binarySearchShrinkable<int>(len).template transform<std::vector<Shrinkable<T>>>([shrinkVec](const int& len) {
            if(len <= 0)
                return std::vector<Shrinkable<T>>();

            auto begin = shrinkVec.begin();
            auto last = shrinkVec.begin() + len;
            return std::vector<Shrinkable<T>>(begin, last);;
        });

        // TODO: apply 0~size-1 and save to new var
        shrinkableVecShrinkable = shrinkableVecShrinkable.concat([](const Shrinkable<std::vector<Shrinkable<T>>>& shrinkVecShrink) -> Stream<Shrinkable<std::vector<Shrinkable<T>>>>{
            return shrinkElement(shrinkVecShrink, shrinkVecShrink.getRef().size()-1);
        });

        auto vecShrinkable = shrinkableVecShrinkable.template transform<std::vector<T>>([](const std::vector<Shrinkable<T>>& shrinkVec) -> std::vector<T> {
            std::vector<T> valueVec;
            std::transform(shrinkVec.begin(), shrinkVec.end(), std::back_inserter(valueVec), [](const Shrinkable<T>& shr) -> T {
                return shr.get();
            });
            return valueVec;
        });

        return vecShrinkable;
    }

    int maxLen;
    Arbitrary<T> elemGen;
};

template <typename T>
size_t Arbitrary<std::vector<T>>::defaultMaxLen = 200;


} // namespace PropertyBasedTesting



