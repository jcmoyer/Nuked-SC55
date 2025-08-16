#pragma once

#include <cstdint>

constexpr uint16_t SX(uint8_t byte)
{
    return (uint16_t)(int8_t)byte;
}

// Match Upper 5 bits
constexpr bool MU5(uint8_t pattern, uint8_t byte)
{
    return (byte & 0b11111000) == pattern;
}

// Match Upper 4 bits
constexpr bool MU4(uint8_t pattern, uint8_t byte)
{
    return (byte & 0b11110000) == pattern;
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

// These types serve dual purposes;
// 1. Tag dispatch - types are used to find the correct functions to call in
//    generic contexts. Specifically, instruction handlers are parameterized by
//    mode.
// 2. Modes referring to a register encode the register as part of their type.

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
