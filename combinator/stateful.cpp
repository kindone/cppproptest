#include "stateful.hpp"
#include "../Random.hpp"
#include "../Shrinkable.hpp"

namespace PropertyBasedTesting {

Shrinkable<EmptyModel> EmptyModelGen(Random&)
{
    return make_shrinkable<EmptyModel>();
}

}  // namespace PropertyBasedTesting
