#pragma once

#include <iostream>

namespace PropertyBasedTesting {
namespace util {

std::ostream& codepage(std::ostream& os, uint32_t code);
std::ostream& charAsHex(std::ostream& os, uint8_t c);
std::ostream& charAsHex(std::ostream& os, uint8_t c1, uint8_t c2);
std::ostream& charAsHex(std::ostream& os, uint8_t c1, uint8_t c2, uint8_t c3);
std::ostream& charAsHex(std::ostream& os, uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4);
std::ostream& charAsHex(std::ostream& os, std::vector<uint8_t>& chars);
std::ostream& validChar(std::ostream& os, uint8_t c);
std::ostream& validChar2(std::ostream& os, uint8_t c);
// std::ostream& validChar(std::ostream& os, uint8_t c1, uint8_t c2);
// std::ostream& validChar(std::ostream& os, uint8_t c1, uint8_t c2, uint8_t c3);
// std::ostream& validChar(std::ostream& os, uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4);

}  // namespace util
}  // namespace PropertyBasedTesting
