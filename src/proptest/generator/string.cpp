#include "proptest/gen.hpp"
#include "proptest/generator/string.hpp"
#include "proptest/generator/util.hpp"
#include "proptest/generator/integral.hpp"
#include "proptest/shrinker/string.hpp"
#include "proptest/util/std.hpp"

namespace proptest {

size_t Arbi<string>::defaultMinSize = 0;
size_t Arbi<string>::defaultMaxSize = 200;

// defaults to ascii characters
Arbi<string>::Arbi()
    : ArbiContainer<string>(defaultMinSize, defaultMaxSize), elemGen(interval<char>(0x1, 0x7f))
{
}

Arbi<string>::Arbi(Arbi<char>& _elemGen)
    : ArbiContainer<string>(defaultMinSize, defaultMaxSize),
      elemGen([_elemGen](Random& rand) mutable { return _elemGen(rand); })
{
}

Arbi<string>::Arbi(GenFunction<char> _elemGen)
    : ArbiContainer<string>(defaultMinSize, defaultMaxSize), elemGen(_elemGen)
{
}

Shrinkable<string> Arbi<string>::operator()(Random& rand)
{
    size_t size = rand.getRandomSize(minSize, maxSize + 1);
    string str(size, ' ' /*, allocator()*/);
    for (size_t i = 0; i < size; i++)
        str[i] = elemGen(rand).get();

    return shrinkString(str, minSize);
}

}  // namespace proptest
