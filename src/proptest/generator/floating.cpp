#include "proptest/gen.hpp"
#include "proptest/shrinker/floating.hpp"
#include "proptest/generator/floating.hpp"
#include "proptest/generator/bool.hpp"
#include "proptest/generator/util.hpp"
#include "proptest/generator/tuple.hpp"
#include "proptest/util/std.hpp"

namespace proptest {

Shrinkable<float> Arbi<float>::operator()(Random& rand)
{
    auto raw = rand.getRandomUInt32();
    float value = *reinterpret_cast<float*>(&raw);

    return make_shrinkable<float>(value).with([value]() { return shrinkFloat(value); });
}

Shrinkable<double> Arbi<double>::operator()(Random& rand)
{
    auto raw = rand.getRandomUInt64();
    double value = *reinterpret_cast<double*>(&raw);

    return make_shrinkable<double>(value).with([value]() { return shrinkFloat(value); });
}

}  // namespace proptest
