#include "misc.hpp"
#include "unicode.hpp"
#include "std.hpp"

namespace proptest {
namespace util {

ostream& codepage(ostream& os, uint32_t code)
{
    util::IosFlagSaver iosFlagSaver(os);
    if (code < 0x10000) {
        os << "\\u";
        os << setfill('0') << setw(2) << util::hex << ((code & 0xff00) >> 8);
        os << setfill('0') << setw(2) << util::hex << (code & 0xff);
    } else {
        os << "\\U";
        os << setfill('0') << setw(2) << util::hex << ((code & 0xff0000) >> 16);
        os << setfill('0') << setw(2) << util::hex << ((code & 0xff00) >> 8);
        os << setfill('0') << setw(2) << util::hex << (code & 0xff);
    }

    return os;
}

ostream& validString(ostream& os, const string& str)
{
    for (size_t i = 0; i < str.size(); i++)
        validChar(os, static_cast<uint8_t>(str[i]));
    return os;
}

ostream& charAsHex(ostream& os, uint8_t c)
{
    util::IosFlagSaver iosFlagSaver(os);
    os << "\\x" << setfill('0') << setw(2) << util::hex << static_cast<int>(c);
    return os;
}

ostream& charAsHex(ostream& os, uint8_t c1, uint8_t c2)
{
    util::IosFlagSaver iosFlagSaver(os);
    os << "\\x" << setfill('0') << setw(2) << util::hex << static_cast<int>(c1);
    os << "\\x" << setfill('0') << setw(2) << util::hex << static_cast<int>(c2);
    return os;
}

ostream& charAsHex(ostream& os, uint8_t c1, uint8_t c2, uint8_t c3)
{
    util::IosFlagSaver iosFlagSaver(os);
    os << "\\x" << setfill('0') << setw(2) << util::hex << static_cast<int>(c1);
    os << "\\x" << setfill('0') << setw(2) << util::hex << static_cast<int>(c2);
    os << "\\x" << setfill('0') << setw(2) << util::hex << static_cast<int>(c3);
    return os;
}

ostream& charAsHex(ostream& os, uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4)
{
    util::IosFlagSaver iosFlagSaver(os);
    os << "\\x" << setfill('0') << setw(2) << util::hex << static_cast<int>(c1);
    os << "\\x" << setfill('0') << setw(2) << util::hex << static_cast<int>(c2);
    os << "\\x" << setfill('0') << setw(2) << util::hex << static_cast<int>(c3);
    os << "\\x" << setfill('0') << setw(2) << util::hex << static_cast<int>(c4);
    return os;
}

ostream& charAsHex(ostream& os, vector<uint8_t>& chars)
{
    util::IosFlagSaver iosFlagSaver(os);

    if (chars.size() > 0)
        os << /*"\\x" <<*/ setfill('0') << setw(2) << util::hex << static_cast<int>(chars[0]);
    for (size_t i = 1; i < chars.size(); i++) {
        os << " " << setfill('0') << setw(2) << util::hex << static_cast<int>(chars[i]);
    }
    return os;
}

ostream& charAsHex(ostream& os, const string& chars)
{
    util::IosFlagSaver iosFlagSaver(os);

    if (chars.size() > 0)
        os << /*"\\x" <<*/ setfill('0') << setw(2) << hex
           << static_cast<int>(static_cast<uint8_t>(chars[0]));
    for (size_t i = 1; i < chars.size(); i++) {
        os << " " << setfill('0') << setw(2) << util::hex << static_cast<int>(static_cast<uint8_t>(chars[i]));
    }
    return os;
}

ostream& validChar(ostream& os, uint8_t c)
{
    if (static_cast<char>(c) == '\\')
        os << "\\\\";
    else if (c < 0x20 || c == 0x7f) {
        util::IosFlagSaver iosFlagSaver(os);
        os << "\\x" << setfill('0') << setw(2) << util::hex << static_cast<int>(c);
    } else
        os << static_cast<char>(c);

    return os;
}

ostream& validChar2(ostream& os, uint8_t c)
{
    if (static_cast<char>(c) == '\\')
        os << "\\\\";
    else if (c < 0x20 || c == 0x7f) {
        util::IosFlagSaver iosFlagSaver(os);
        os << "\\u" << setfill('0') << setw(4) << util::hex << static_cast<int>(c);
    } else
        os << static_cast<char>(c);

    return os;
}

// ostream& validChar(ostream& os, uint8_t c1, uint8_t c2)
// {
//     util::IosFlagSaver iosFlagSaver(os);
//     os << "\\u" << setfill('0') << setw(2) << util::hex << static_cast<int>(c1) << static_cast<int>(c2);
//     return os;
// }

// ostream& validChar(ostream& os, uint8_t c1, uint8_t c2, uint8_t c3)
// {
//     util::IosFlagSaver iosFlagSaver(os);
//     os << "\\U" << setfill('0') << setw(2) << util::hex << static_cast<int>(0) << static_cast<int>(c1)
//        << static_cast<int>(c2) << static_cast<int>(c3);
//     return os;
// }

// ostream& validChar(ostream& os, uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4)
// {
//     util::IosFlagSaver iosFlagSaver(os);
//     os << "\\U" << setfill('0') << setw(2) << util::hex << static_cast<int>(c1) << static_cast<int>(c2)
//        << static_cast<int>(c3) << static_cast<int>(c4);
//     return os;
// }

}  // namespace util
}  // namespace proptest
