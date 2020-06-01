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
Arbitrary<std::string>::Arbitrary() : elemGen(fromTo<char>(0x1, 0x7f)), minSize(defaultMinSize), maxSize(defaultMaxSize)
{
}

Arbitrary<std::string>::Arbitrary(Arbitrary<char>& _elemGen)
    : elemGen([_elemGen](Random& rand) mutable { return _elemGen(rand); }),
      minSize(defaultMinSize),
      maxSize(defaultMaxSize)
{
}

Arbitrary<std::string>::Arbitrary(std::function<Shrinkable<char>(Random&)> _elemGen)
    : elemGen(_elemGen), minSize(defaultMinSize), maxSize(defaultMaxSize)
{
}

Arbitrary<std::string> Arbitrary<std::string>::setMinSize(size_t size)
{
    minSize = size;
    return *this;
}

Arbitrary<std::string> Arbitrary<std::string>::setMaxSize(size_t size)
{
    maxSize = size;
    return *this;
}

Arbitrary<std::string> Arbitrary<std::string>::setSize(size_t size)
{
    minSize = size;
    maxSize = size;
    return *this;
}

Shrinkable<std::string> Arbitrary<std::string>::operator()(Random& rand)
{
    size_t size = rand.getRandomSize(minSize, maxSize + 1);
    std::string str(size, ' ' /*, allocator()*/);
    for (size_t i = 0; i < size; i++)
        str[i] = elemGen(rand).get();

    size_t minSizeCopy = minSize;
    return binarySearchShrinkable<size_t>(size - minSizeCopy)
        .transform<std::string>([str, minSizeCopy](const size_t& size) { return str.substr(0, size + minSizeCopy); });

    /*
    return make_shrinkable<std::string>(str).with([str]() -> stream_t {
        return stream_t::empty();
    });
    */
}

}  // namespace PropertyBasedTesting
