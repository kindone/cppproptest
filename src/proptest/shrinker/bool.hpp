#pragma once
#include "proptest/Shrinkable.hpp"
#include "proptest/api.hpp"

namespace proptest {

PROPTEST_API Shrinkable<bool> shrinkBool(bool value);

}
