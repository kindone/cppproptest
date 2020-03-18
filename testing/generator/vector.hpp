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
        std::vector<T> vec;
        vec.reserve(len);
        for(int i = 0; i < len; i++)
            vec.push_back(elemGen(rand));

        //return make_shrinkable<std::vector<T>>(std::move(vec));

        return binarySearchShrinkable<int>(len).template map<std::vector<T>>([vec](const int& len) {
            if(len <= 0)
                return std::vector<T>();

            auto begin = vec.begin();
            auto last = vec.begin() + len;
            return std::vector<T>(begin, last);;
        });

    }

    int maxLen;
    Arbitrary<T> elemGen;
};


} // namespace PropertyBasedTesting



