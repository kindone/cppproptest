#pragma once

#include "../Shrinkable.hpp"
#include "../Random.hpp"
#include "../api.hpp"
#include <functional>

namespace proptest {

PROPTEST_API Shrinkable<uint32_t> unicodeGen(Random& rand);

}
