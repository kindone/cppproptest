#pragma once

#include "../Shrinkable.hpp"
#include "../api.hpp"
#include <functional>

namespace pbt {

PROPTEST_API Shrinkable<uint32_t> unicodeGen(Random& rand);

}
