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
    Arbitrary(int _maxLen = 200) : maxLen(_maxLen)  {
    }

    Arbitrary(Arbitrary<T> _elemGen, int _maxLen = 200) : elemGen(_elemGen), maxLen(_maxLen)  {
    }

    Shrinkable<std::vector<T>> operator()(Random& rand) {
        int len = rand.getRandomSize(0, maxLen+1);
        std::vector<Shrinkable<T>> shrinkVec;
        shrinkVec.reserve(len);
        for(int i = 0; i < len; i++)
            shrinkVec.push_back(elemGen(rand));

        // shrink with subvector using binary numeric shrink of lengths
        return binarySearchShrinkable<int>(len).template transform<std::vector<Shrinkable<T>>>([shrinkVec](const int& len) {
            if(len <= 0)
                return std::vector<Shrinkable<T>>();

            auto begin = shrinkVec.begin();
            auto last = shrinkVec.begin() + len;
            return std::vector<Shrinkable<T>>(begin, last);;
        // convert shrinkable<T> to T
        }).template transform<std::vector<T>>([](const std::vector<Shrinkable<T>>& shrinkVec) {
            std::vector<T> valueVec;
            std::transform(shrinkVec.begin(), shrinkVec.end(), std::back_inserter(valueVec), [](const Shrinkable<T>& shr) -> T {
                return shr.get();
            });
            return valueVec;
        });
    }

    int maxLen;
    Arbitrary<T> elemGen;
};


} // namespace PropertyBasedTesting



