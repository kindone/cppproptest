#ifndef __PROPTEST_VECTOR_HPP
#define __PROPTEST_VECTOR_HPP

#include "testing/gen.hpp"
#include "testing/Random.hpp"
#include <vector>

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

    std::vector<T> generate(Random& rand) {
        int len = rand.getRandomSize(0, maxLen+1);
        std::vector<T> val;
        val.reserve(len);
        for(int i = 0; i < len; i++)
            val.push_back(elemGen.generate(rand));
        return std::vector<T>(std::move(val));
    }

    int maxLen;
    Arbitrary<T> elemGen;
};

} // namespace PropertyBasedTesting

#endif
