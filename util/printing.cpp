#include "printing.hpp"
#include "api.hpp"
#include "util/string.hpp"

namespace PropertyBasedTesting {

PROPTEST_API std::ostream& show(std::ostream& os, const std::string& str)
{
    os << "\"";
    decodeUTF8(os, str);
    os << "\"";
    // os << "\"" << str << "\"";
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
