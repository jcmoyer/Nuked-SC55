#pragma once

enum class AddressMode
{
    // Register direct (Rn)
    Rn,
    // Register indirect (@Rn)
    ARn,
    // Register indirect with displacement (@(d:8, Rn), @(d:16, Rn))
    Ad8_Rn,
    Ad16_Rn,
    // Register indirect with pre-decrement or post-increment (@-Rn, @Rn+)
    APreDecRn,
    APostIncRn,
    // Absolute address (@aa:8, @aa:16)
    Aaa8,
    Aaa16,
    // Immediate (#xx:8, #xx:16)
    imm8,
    imm16,
};

const char* ToCString(AddressMode mode);

// These types represent individual addressing modes in compile-time known
// contexts. Each instruction handler is parameterized by one of these types
// and uses it to generically select the correct functions to read/write
// instruction operands.

// clang-format off

// Register direct (Rn)
struct Mode_Rn{};

// Register indirect (@Rn)
struct Mode_ARn{};

// Register indirect with displacement (@(d:8, Rn), @(d:16, Rn))
//
// This type combines both modes since they share the same instructions and the
// code to handle displacements is identical.
struct Mode_d8_Rn{};
struct Mode_d16_Rn{};

// Register indirect with pre-decrement (@-Rn)
struct Mode_PreDecRn{};
// Register indirect with post-increment (@Rn+)
struct Mode_PostIncRn{};

// Immediate (#xx:8, #xx:16)
struct Mode_Imm8{};
struct Mode_Imm16{};

// Absolute (@aa:8, @aa:16)
struct Mode_Aa8{};
struct Mode_Aa16{};

// clang-format on
