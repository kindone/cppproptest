#include "../gen.hpp"
#include "string.hpp"
#include "util.hpp"
#include "integral.hpp"
#include <vector>
#include <iostream>
#include <ios>
#include <iomanip>
#include <sstream>

namespace proptest {

size_t Arbi<std::string>::defaultMinSize = 0;
size_t Arbi<std::string>::defaultMaxSize = 200;

// defaults to ascii characters
Arbi<std::string>::Arbi()
    : ArbiContainer<std::string>(defaultMinSize, defaultMaxSize), elemGen(interval<char>(0x1, 0x7f))
{
}

Arbi<std::string>::Arbi(Arbi<char>& _elemGen)
    : ArbiContainer<std::string>(defaultMinSize, defaultMaxSize),
      elemGen([_elemGen](Random& rand) mutable { return _elemGen(rand); })
{
}

Arbi<std::string>::Arbi(GenFunction<char> _elemGen)
    : ArbiContainer<std::string>(defaultMinSize, defaultMaxSize), elemGen(_elemGen)
{
}

Shrinkable<std::string> Arbi<std::string>::operator()(Random& rand)
{
    size_t size = rand.getRandomSize(minSize, maxSize + 1);
    std::string str(size, ' ' /*, allocator()*/);
    for (size_t i = 0; i < size; i++)
        str[i] = elemGen(rand).get();

    size_t minSizeCopy = minSize;
    return util::binarySearchShrinkableU(size - minSizeCopy).map<std::string>([str, minSizeCopy](const uint64_t& size) {
        return str.substr(0, size + minSizeCopy);
    });
}

}  // namespace proptest
