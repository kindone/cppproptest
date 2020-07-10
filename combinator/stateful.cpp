#include "stateful.hpp"
#include "../Random.hpp"
#include "../Shrinkable.hpp"

namespace proptest {

Shrinkable<EmptyModel> EmptyModelGen(Random&)
{
    return make_shrinkable<EmptyModel>();
}

}  // namespace proptest
