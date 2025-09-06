#include "address_modes.h"

#include <utility>

const char* ToCString(AddressMode mode)
{
    switch (mode)
    {
    case AddressMode::Rn:
        return "Rn";
    case AddressMode::ARn:
        return "ARn";
    case AddressMode::Ad8_Rn:
        return "Ad8_Rn";
    case AddressMode::Ad16_Rn:
        return "Ad16_Rn";
    case AddressMode::APreDecRn:
        return "APreDecRn";
    case AddressMode::APostIncRn:
        return "APostIncRn";
    case AddressMode::Aaa8:
        return "Aaa8";
    case AddressMode::Aaa16:
        return "Aaa16";
    case AddressMode::imm8:
        return "imm8";
    case AddressMode::imm16:
        return "imm16";
    }
    std::unreachable();
}
