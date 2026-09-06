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

#include "decoder2/disassemble.h"

namespace decoder2
{

using Disassembler = void (*)(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);

// Rn
Disassembler GetDecoderRn(uint8_t byte);

// @Rn, @(d:8,Rn), @(d:16,Rn), @-Rn, @Rn+, @aa:8, @aa:16
Disassembler GetDecoderGeneric(uint8_t byte);

// #xx:8
Disassembler GetDecoderimm8(uint8_t byte);

// #xx:16
Disassembler GetDecoderimm16(uint8_t byte);

// Short-form instructions
Disassembler GetDecoderTop(uint8_t byte);

} // namespace decoder2
