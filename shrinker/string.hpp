#pragma once
#include "../Shrinkable.hpp"

namespace proptest {

Shrinkable<std::string> shrinkString(const std::string& str, size_t minSize);

}
