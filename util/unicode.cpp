#include "misc.hpp"
#include <iostream>
#include <iomanip>
#include <ios>

namespace PropertyBasedTesting {
namespace util {

std::ostream& codepage(std::ostream& os, uint32_t code)
{
    util::IosFlagSaver iosFlagSaver(os);
    if (code < 0x10000) {
        os << "\\u";
        os << std::setfill('0') << std::setw(2) << std::hex << ((code & 0xff00) >> 8);
        os << std::setfill('0') << std::setw(2) << std::hex << (code & 0xff);
    } else {
        os << "\\U";
        os << std::setfill('0') << std::setw(2) << std::hex << ((code & 0xff0000) >> 16);
        os << std::setfill('0') << std::setw(2) << std::hex << ((code & 0xff00) >> 8);
        os << std::setfill('0') << std::setw(2) << std::hex << (code & 0xff);
    }

    return os;
}

}  // namespace util
}  // namespace PropertyBasedTesting
