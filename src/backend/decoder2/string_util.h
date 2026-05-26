#pragma once

#include <cstdint>
#include <string>

namespace decoder2
{

constexpr const char* HEX_DIGITS = "0123456789abcdef";

inline void WriteHexU8(std::string& s, uint8_t val)
{
    uint8_t hi = val >> 4;
    uint8_t lo = val & 0x0f;
    s.push_back(HEX_DIGITS[hi]);
    s.push_back(HEX_DIGITS[lo]);
}

inline void WriteHexU16(std::string& s, uint16_t val)
{
    WriteHexU8(s, (uint8_t)(val >> 8));
    WriteHexU8(s, (uint8_t)val);
}

inline void WriteBinU8(std::string& s, uint8_t x)
{
    for (uint8_t mask = 0x80; mask; mask >>= 1)
    {
        s.push_back((x & mask) ? '1' : '0');
    }
}

} // namespace decoder2
