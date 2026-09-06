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

#include "decoder2/dispatch.h"
#include "decoder2/instruction.h"
#include "decoder2/types.h"

namespace decoder2
{

class CodeReader;

using Dispatcher = DecodeError (*)(CodeReader& reader, uint8_t byte, DecodedInstruction& instr);

// Top level decode table for instructions. Depending on `byte` the handler
// returned might decode a general form instruction or a special form
// instruction.
Dispatcher GetDispatcherTop(uint8_t byte);

// Rn
// 1010[Sz]rrr | opcode [...]
Dispatcher GetDispatcherRn(uint8_t opcode, Size size);

// @Rn
// 1101[Sz]rrr | opcode [...]
Dispatcher GetDispatcherARn(uint8_t opcode, Size size);

// @(d:8,Rn)
// 1110[Sz]rrr | disp8 | opcode [...]
Dispatcher GetDispatcherAd8Rn(uint8_t opcode, Size size);

// @(d:16,Rn)
// 1111[Sz]rrr | disp16 | opcode [...]
Dispatcher GetDispatcherAd16Rn(uint8_t opcode, Size size);

// @-Rn
// 1011[Sz]rrr | opcode [...]
Dispatcher GetDispatcherAPreDecRn(uint8_t opcode, Size size);

// @Rn+
// 1100[Sz]rrr | opcode [...]
Dispatcher GetDispatcherAPostIncRn(uint8_t opcode, Size size);

// #xx:8
// 00000100 | imm8 | opcode [...]
Dispatcher GetDispatcherImm8(uint8_t opcode);

// #xx:16
// 00001100 | imm16 | opcode [...]
Dispatcher GetDispatcherImm16(uint8_t opcode);

// @aa:8
// 0000[Sz]101 | addr8 | opcode [...]
Dispatcher GetDispatcherAaa8(uint8_t opcode, Size size);

// @aa:16
// 0001[Sz]101 | addr16 | opcode [...]
Dispatcher GetDispatcherAaa16(uint8_t opcode, Size size);

} // namespace decoder2
