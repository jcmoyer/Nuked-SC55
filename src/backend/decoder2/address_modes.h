/*
 * Copyright (C) 2024-2026 J.C. Moyer
 *
 * This file is part of Nuked-SC55.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#pragma once

#include <cstdint>

namespace decoder2
{

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

// Register indirect with displacement (@(d:8,Rn), @(d:16,Rn))
struct Mode_Ad8_Rn{};
struct Mode_Ad16_Rn{};

// Register indirect with pre-decrement (@-Rn)
struct Mode_APreDecRn{};
// Register indirect with post-increment (@Rn+)
struct Mode_APostIncRn{};

// Immediate (#xx:8, #xx:16)
struct Mode_Imm8{};
struct Mode_Imm16{};

// Absolute (@aa:8, @aa:16)
struct Mode_Aaa8{};
struct Mode_Aaa16{};

// clang-format on

constexpr uint8_t ModeEASize(Mode_Rn)
{
    return 1;
}

constexpr uint8_t ModeEASize(Mode_ARn)
{
    return 1;
}

constexpr uint8_t ModeEASize(Mode_Ad8_Rn)
{
    return 2;
}

constexpr uint8_t ModeEASize(Mode_Ad16_Rn)
{
    return 3;
}

constexpr uint8_t ModeEASize(Mode_APreDecRn)
{
    return 1;
}

constexpr uint8_t ModeEASize(Mode_APostIncRn)
{
    return 1;
}

constexpr uint8_t ModeEASize(Mode_Aaa8)
{
    return 2;
}

constexpr uint8_t ModeEASize(Mode_Aaa16)
{
    return 3;
}

constexpr uint8_t ModeEASize(Mode_Imm8)
{
    return 2;
}

constexpr uint8_t ModeEASize(Mode_Imm16)
{
    return 3;
}

} // namespace decoder2
