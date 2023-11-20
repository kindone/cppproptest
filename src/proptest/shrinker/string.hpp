#pragma once
#include "proptest/Shrinkable.hpp"

namespace proptest {

Shrinkable<string> shrinkString(const string& str, size_t minSize);

}
