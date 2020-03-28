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
        auto shrinkableVecShrinkable =  binarySearchShrinkable<int>(len).template transform<std::vector<Shrinkable<T>>>([shrinkVec](const int& len) {
            if(len <= 0)
                return std::vector<Shrinkable<T>>();

            auto begin = shrinkVec.begin();
            auto last = shrinkVec.begin() + len;
            return std::vector<Shrinkable<T>>(begin, last);
        });

        // Shr([shr,...,shr]).with(Stream { shr([]), shr([shr,shr,shr,shr]), shr([shr,shr,shr,shr,shr,shr]), shr([shr,...,shr]) })

        // TODO: apply 0~size-1 and save to new var
        using vector_t = std::vector<Shrinkable<T>>;
        using shrinkable_t = Shrinkable<vector_t>;
        using stream_t = Stream<shrinkable_t>;
        using element_t = T;
        using e_shrinkable_t = Shrinkable<element_t>;
        using e_stream_t = Stream<e_shrinkable_t>;

        auto genElementalShrinks = [](const shrinkable_t& current) -> stream_t{
            // Shr([shr,...,shr]).with(Stream { shr([]), shr([shr,shr,shr,shr]), shr([shr,shr,shr,shr,shr,shr]), shr([shr,...,shr]) })

            vector_t curVec = current.getRef();
            stream_t shrinks = stream_t::empty();

            if(curVec.empty())
                return shrinks;
            const auto size = curVec.size();

            for(int64_t i = size-1; i >= 0; i--)
            {
                e_shrinkable_t& element = curVec[i];
                e_stream_t elemShrinks = element.shrinks();
                // 1Dimensional transform {shr(0), shr(4), shr(6), shr(7)} -> shr([...,shr(0)]), shr([...,shr(4)]), shr([...,shr(6)]), shr([...,shr(7)])
                shrinks = shrinks.concat(elemShrinks.template transform<shrinkable_t>([curVec, i](const e_shrinkable_t& elemShrink) {
                    vector_t vecCopy = curVec;
                    vecCopy[i] = elemShrink;
                    return make_shrinkable<std::vector<Shrinkable<T>>>(vecCopy);
                }));
            }
            return shrinks;
        };

        shrinkableVecShrinkable = shrinkableVecShrinkable.concat(genElementalShrinks);

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



