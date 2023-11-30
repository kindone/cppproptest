#pragma once
#include "../Shrinkable.hpp"
#include "../api.hpp"

namespace proptest {

PROPTEST_API Shrinkable<bool> shrinkBool(bool value);

}
