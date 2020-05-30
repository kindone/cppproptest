#include "printing.hpp"
#include "../api.hpp"
#include "utf8string.hpp"
#include "utf16string.hpp"
#include "cesu8string.hpp"
#include "unicode.hpp"

namespace PropertyBasedTesting {

PROPTEST_API std::ostream& show(std::ostream& os, const std::string& str)
{
    os << "\"";
    util::decodeUTF8(os, str);
    os << "\"";
    os << " (";
    util::charAsHex(os, str);
    os << ")";
    // os << "\"" << str << "\"";
    return os;
}

PROPTEST_API std::ostream& show(std::ostream& os, const UTF8String& str)
{
    os << "\"";
    util::decodeUTF8(os, str);
    os << "\"";
    os << " (";
    util::charAsHex(os, str);
    os << ")";
    return os;
}

PROPTEST_API std::ostream& show(std::ostream& os, const UTF16BEString& str)
{
    os << "\"";
    util::decodeUTF16BE(os, str);
    os << "\"";
    os << " (";
    util::charAsHex(os, str);
    os << ")";
    return os;
}

PROPTEST_API std::ostream& show(std::ostream& os, const UTF16LEString& str)
{
    os << "\"";
    util::decodeUTF16LE(os, str);
    os << "\"";
    os << " (";
    util::charAsHex(os, str);
    os << ")";
    return os;
}

PROPTEST_API std::ostream& show(std::ostream& os, const CESU8String& str)
{
    os << "\"";
    util::decodeCESU8(os, str);
    os << "\"";
    os << " (";
    util::charAsHex(os, str);
    os << ")";
    return os;
}

PROPTEST_API std::ostream& show(std::ostream& os, const bool& val)
{
    os << (val ? "true" : "false");
    return os;
}

PROPTEST_API std::ostream& show(std::ostream& os, const char& val)
{
    os << static_cast<int>(val);
    return os;
}

PROPTEST_API std::ostream& show(std::ostream& os, const int8_t& val)
{
    os << static_cast<int>(val);
    return os;
}

PROPTEST_API std::ostream& show(std::ostream& os, const uint8_t& val)
{
    os << static_cast<int>(val);
    return os;
}

PROPTEST_API std::ostream& show(std::ostream& os, const int16_t& val)
{
    os << val;
    return os;
}

PROPTEST_API std::ostream& show(std::ostream& os, const uint16_t& val)
{
    os << val;
    return os;
}

PROPTEST_API std::ostream& show(std::ostream& os, const int32_t& val)
{
    os << val;
    return os;
}

PROPTEST_API std::ostream& show(std::ostream& os, const uint32_t& val)
{
    os << val;
    return os;
}

PROPTEST_API std::ostream& show(std::ostream& os, const int64_t& val)
{
    os << val;
    return os;
}

PROPTEST_API std::ostream& show(std::ostream& os, const uint64_t& val)
{
    os << val;
    return os;
}

PROPTEST_API std::ostream& show(std::ostream& os, const float& val)
{
    os << val;
    return os;
}

PROPTEST_API std::ostream& show(std::ostream& os, const double& val)
{
    os << val;
    return os;
}

}  // namespace PropertyBasedTesting
