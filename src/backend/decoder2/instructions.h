#pragma once

#include <cstdint>

constexpr uint16_t SX(uint8_t byte)
{
    return (uint16_t)(int8_t)byte;
}

enum class I_AddressMode
{
    Rn,
    ARn,
    Ad8_Rn,
    Ad16_Rn,
    AMRn,
    ARnP,
    Aaa8,
    Aaa16,
    imm8,
    imm16,
};

const char* ToCString(I_AddressMode mode);

constexpr bool I_RefsAddress(I_AddressMode mode)
{
    switch (mode)
    {
    case I_AddressMode::ARn:
    case I_AddressMode::Ad8_Rn:
    case I_AddressMode::Ad16_Rn:
    case I_AddressMode::AMRn:
    case I_AddressMode::ARnP:
    case I_AddressMode::Aaa8:
    case I_AddressMode::Aaa16:
        return true;
    default:
        return false;
    }
}

constexpr bool I_HasImmediate(I_AddressMode mode)
{
    switch (mode)
    {
    case I_AddressMode::imm8:
    case I_AddressMode::imm16:
        return true;
    default:
        return false;
    }
}

// These types represent individual addressing modes. Each instruction handler
// is parameterized by one of these types and uses it to select the correct
// overloads of functions that read/write instruction operands.

// clang-format off
// Register direct (Rn)
struct I_Rn_State{};
// Register indirect (@Rn)
struct I_ARn_State{};
// Register indirect with displacement (@(d:8, Rn), @(d:16, Rn))
// This type combines both modes since they share the same instructions and the
// code to handle each displacement is identical.
struct I_d8d16_Rn_State{};
// Register indirect with pre-decrement or post-increment (@-Rn, @+Rn)
struct I_AMRn_State{};
struct I_ARnP_State{};
// Immediate (#xx:8, #xx:16)
struct I_imm8_State{};
struct I_imm16_State{};
// Absolute (@aa:8, @aa:16)
struct I_aa8_State{};
struct I_aa16_State{};

// clang-format on
