#include "proptest/api.hpp"
#include "proptest/util/std.hpp"
#include "proptest/util/any.hpp"
#include "proptest/Stream.hpp"
#include "proptest/Shrinkable.hpp"
#include "proptest/generator/bool.hpp"

namespace proptest {

Arbi<bool>::Arbi(double prob) : trueProb(prob) {}

Shrinkable<bool> Arbi<bool>::operator()(Random& rand)
{
    bool value = rand.getRandomBool(trueProb);
    return shrinkBool(value);
}


Arbi<bool>::~Arbi() {}

}  // namespace proptest
