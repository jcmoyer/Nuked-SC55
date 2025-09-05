#include "address_modes.h"

#include <utility>

const char* ToCString(I_AddressMode mode)
{
    switch (mode)
    {
    case I_AddressMode::Rn:
        return "Rn";
    case I_AddressMode::ARn:
        return "ARn";
    case I_AddressMode::Ad8_Rn:
        return "Ad8_Rn";
    case I_AddressMode::Ad16_Rn:
        return "Ad16_Rn";
    case I_AddressMode::AMRn:
        return "AMRn";
    case I_AddressMode::ARnP:
        return "ARnP";
    case I_AddressMode::Aaa8:
        return "Aaa8";
    case I_AddressMode::Aaa16:
        return "Aaa16";
    case I_AddressMode::imm8:
        return "imm8";
    case I_AddressMode::imm16:
        return "imm16";
    }
    std::unreachable();
}
