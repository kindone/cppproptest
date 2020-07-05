#include "assert.hpp"
#include <iostream>
#include <sstream>

namespace pbt {
namespace util {

std::ostream& errorOrEmpty(bool error) {
    static std::stringstream str;
    static std::ostream& empty = str;
    if(error)
        return std::cerr;
    else
        return empty;
}

}  // namespace util
}  // namespace pbt
