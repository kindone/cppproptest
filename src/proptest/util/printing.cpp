#include "proptest/util/printing.hpp"
#include "proptest/api.hpp"
#include "proptest/util/utf8string.hpp"
#include "proptest/util/utf16string.hpp"
#include "proptest/util/cesu8string.hpp"
#include "proptest/util/unicode.hpp"

namespace proptest {

PROPTEST_API ostream& show(ostream& os, const char* c_str, size_t len)
{
    string str(c_str, len);
    os << "\"" << util::StringPrintable(str) << "\" (" << util::StringAsHex(str) << ")";
    return os;
}

PROPTEST_API ostream& show(ostream& os, const char* c_str)
{
    string str(c_str);
    os << "(" << util::StringAsHex(str) << ")";
    return os;
}

PROPTEST_API ostream& show(ostream& os, const string& str)
{
    os << "\"" << util::DecodeUTF8(str) << "\" (" << util::StringAsHex(str) << ")";
    return os;
}

PROPTEST_API ostream& show(ostream& os, const UTF8String& str)
{
    os << "\"" << util::DecodeUTF8(str) << "\" (" << util::StringAsHex(str) << ")";
    return os;
}

PROPTEST_API ostream& show(ostream& os, const UTF16BEString& str)
{
    os << "\"" << util::DecodeUTF16BE(str) << "\" (" << util::StringAsHex(str) << ")";
    return os;
}

PROPTEST_API ostream& show(ostream& os, const UTF16LEString& str)
{
    os << "\"" << util::DecodeUTF16LE(str) << "\" (" << util::StringAsHex(str) << ")";
    return os;
}

PROPTEST_API ostream& show(ostream& os, const CESU8String& str)
{
    os << "\"" << util::DecodeCESU8(str) << "\" (" << util::StringAsHex(str) << ")";
    return os;
}

PROPTEST_API ostream& show(ostream& os, const bool& val)
{
    os << (val ? "true" : "false");
    return os;
}

PROPTEST_API ostream& show(ostream& os, const char& val)
{
    os << static_cast<int>(val);
    return os;
}

PROPTEST_API ostream& show(ostream& os, const int8_t& val)
{
    os << static_cast<int>(val);
    return os;
}

PROPTEST_API ostream& show(ostream& os, const uint8_t& val)
{
    os << static_cast<int>(val);
    return os;
}

PROPTEST_API ostream& show(ostream& os, const int16_t& val)
{
    os << val;
    return os;
}

PROPTEST_API ostream& show(ostream& os, const uint16_t& val)
{
    os << val;
    return os;
}

PROPTEST_API ostream& show(ostream& os, const int32_t& val)
{
    os << val;
    return os;
}

PROPTEST_API ostream& show(ostream& os, const uint32_t& val)
{
    os << val;
    return os;
}

PROPTEST_API ostream& show(ostream& os, const int64_t& val)
{
    os << val;
    return os;
}

PROPTEST_API ostream& show(ostream& os, const uint64_t& val)
{
    os << val;
    return os;
}

PROPTEST_API ostream& show(ostream& os, const float& val)
{
    os << val;
    return os;
}

PROPTEST_API ostream& show(ostream& os, const double& val)
{
    os << val;
    return os;
}

}  // namespace proptest
