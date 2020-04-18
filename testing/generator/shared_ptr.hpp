#pragma once

#include <memory>
#include "testing/gen.hpp"

namespace PropertyBasedTesting {

template <typename T>
class Arbitrary<std::shared_ptr<T>> : public Gen<std::shared_ptr<T>> {

};

}