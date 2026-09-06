/*
 * Copyright (C) 2021, 2024 nukeykt
 * Modified by J.C. Moyer
 * Original source file: src/mcu_opcodes.h
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

struct mcu_t;

enum class MCU_Operand_Size : uint8_t
{
    BYTE,
    WORD
};

extern void (*MCU_Operand_Table[256])(mcu_t& mcu, uint8_t operand);
extern void (*MCU_Opcode_Table[32])(mcu_t& mcu, uint8_t opcode, uint8_t opcode_reg);
