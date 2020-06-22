#include "../gen.hpp"
#include "string.hpp"
#include "util.hpp"
#include "numeric.hpp"
#include <vector>
#include <iostream>
#include <ios>
#include <iomanip>
#include <sstream>

namespace PropertyBasedTesting {

size_t Arbitrary<std::string>::defaultMinSize = 0;
size_t Arbitrary<std::string>::defaultMaxSize = 200;

// defaults to ascii characters
Arbitrary<std::string>::Arbitrary()
    : ArbitraryContainer<std::string>(defaultMinSize, defaultMaxSize), elemGen(fromTo<char>(0x1, 0x7f))
{
}

Arbitrary<std::string>::Arbitrary(Arbitrary<char>& _elemGen)
    : ArbitraryContainer<std::string>(defaultMinSize, defaultMaxSize),
      elemGen([_elemGen](Random& rand) mutable { return _elemGen(rand); })
{
}

Arbitrary<std::string>::Arbitrary(std::function<Shrinkable<char>(Random&)> _elemGen)
    : ArbitraryContainer<std::string>(defaultMinSize, defaultMaxSize), elemGen(_elemGen)
{
}

Shrinkable<std::string> Arbitrary<std::string>::operator()(Random& rand)
{
    size_t size = rand.getRandomSize(minSize, maxSize + 1);
    std::string str(size, ' ' /*, allocator()*/);
    for (size_t i = 0; i < size; i++)
        str[i] = elemGen(rand).get();

    size_t minSizeCopy = minSize;
    return binarySearchShrinkableU(size - minSizeCopy).transform<std::string>([str, minSizeCopy](const uint64_t& size) {
        return str.substr(0, size + minSizeCopy);
    });

    /*
    return make_shrinkable<std::string>(str).with([str]() -> stream_t {
        return stream_t::empty();
    });
    */
}

}  // namespace PropertyBasedTesting
