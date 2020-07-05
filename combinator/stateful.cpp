#include "stateful.hpp"
#include "../Random.hpp"
#include "../Shrinkable.hpp"

namespace pbt {

Shrinkable<EmptyModel> EmptyModelGen(Random&)
{
    return make_shrinkable<EmptyModel>();
}

}  // namespace pbt
