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
