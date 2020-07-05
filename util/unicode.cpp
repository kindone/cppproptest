#include "misc.hpp"
#include "unicode.hpp"
#include <iostream>
#include <iomanip>
#include <ios>
#include <vector>

namespace pbt {
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

std::ostream& charAsHex(std::ostream& os, uint8_t c)
{
    util::IosFlagSaver iosFlagSaver(os);
    os << "\\x" << std::setfill('0') << std::setw(2) << std::hex << static_cast<int>(c);
    return os;
}

std::ostream& charAsHex(std::ostream& os, uint8_t c1, uint8_t c2)
{
    util::IosFlagSaver iosFlagSaver(os);
    os << "\\x" << std::setfill('0') << std::setw(2) << std::hex << static_cast<int>(c1);
    os << "\\x" << std::setfill('0') << std::setw(2) << std::hex << static_cast<int>(c2);
    return os;
}

std::ostream& charAsHex(std::ostream& os, uint8_t c1, uint8_t c2, uint8_t c3)
{
    util::IosFlagSaver iosFlagSaver(os);
    os << "\\x" << std::setfill('0') << std::setw(2) << std::hex << static_cast<int>(c1);
    os << "\\x" << std::setfill('0') << std::setw(2) << std::hex << static_cast<int>(c2);
    os << "\\x" << std::setfill('0') << std::setw(2) << std::hex << static_cast<int>(c3);
    return os;
}

std::ostream& charAsHex(std::ostream& os, uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4)
{
    util::IosFlagSaver iosFlagSaver(os);
    os << "\\x" << std::setfill('0') << std::setw(2) << std::hex << static_cast<int>(c1);
    os << "\\x" << std::setfill('0') << std::setw(2) << std::hex << static_cast<int>(c2);
    os << "\\x" << std::setfill('0') << std::setw(2) << std::hex << static_cast<int>(c3);
    os << "\\x" << std::setfill('0') << std::setw(2) << std::hex << static_cast<int>(c4);
    return os;
}

std::ostream& charAsHex(std::ostream& os, std::vector<uint8_t>& chars)
{
    util::IosFlagSaver iosFlagSaver(os);

    if (chars.size() > 0)
        os << /*"\\x" <<*/ std::setfill('0') << std::setw(2) << std::hex << static_cast<int>(chars[0]);
    for (size_t i = 1; i < chars.size(); i++) {
        os << " " << std::setfill('0') << std::setw(2) << std::hex << static_cast<int>(chars[i]);
    }
    return os;
}

std::ostream& charAsHex(std::ostream& os, const std::string& chars)
{
    util::IosFlagSaver iosFlagSaver(os);

    if (chars.size() > 0)
        os << /*"\\x" <<*/ std::setfill('0') << std::setw(2) << std::hex
           << static_cast<int>(static_cast<uint8_t>(chars[0]));
    for (size_t i = 1; i < chars.size(); i++) {
        os << " " << std::setfill('0') << std::setw(2) << std::hex << static_cast<int>(static_cast<uint8_t>(chars[i]));
    }
    return os;
}

std::ostream& validChar(std::ostream& os, uint8_t c)
{
    if (static_cast<char>(c) == '\\')
        os << "\\\\";
    else if (c < 0x20) {
        util::IosFlagSaver iosFlagSaver(os);
        os << "\\x" << std::setfill('0') << std::setw(2) << std::hex << static_cast<int>(c);
    } else
        os << static_cast<char>(c);

    return os;
}

std::ostream& validChar2(std::ostream& os, uint8_t c)
{
    if (static_cast<char>(c) == '\\')
        os << "\\\\";
    else if (c < 0x20) {
        util::IosFlagSaver iosFlagSaver(os);
        os << "\\u" << std::setfill('0') << std::setw(4) << std::hex << static_cast<int>(c);
    } else
        os << static_cast<char>(c);

    return os;
}

// std::ostream& validChar(std::ostream& os, uint8_t c1, uint8_t c2)
// {
//     util::IosFlagSaver iosFlagSaver(os);
//     os << "\\u" << std::setfill('0') << std::setw(2) << std::hex << static_cast<int>(c1) << static_cast<int>(c2);
//     return os;
// }

// std::ostream& validChar(std::ostream& os, uint8_t c1, uint8_t c2, uint8_t c3)
// {
//     util::IosFlagSaver iosFlagSaver(os);
//     os << "\\U" << std::setfill('0') << std::setw(2) << std::hex << static_cast<int>(0) << static_cast<int>(c1)
//        << static_cast<int>(c2) << static_cast<int>(c3);
//     return os;
// }

// std::ostream& validChar(std::ostream& os, uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4)
// {
//     util::IosFlagSaver iosFlagSaver(os);
//     os << "\\U" << std::setfill('0') << std::setw(2) << std::hex << static_cast<int>(c1) << static_cast<int>(c2)
//        << static_cast<int>(c3) << static_cast<int>(c4);
//     return os;
// }

}  // namespace util
}  // namespace pbt
