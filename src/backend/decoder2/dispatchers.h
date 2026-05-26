#pragma once

#include "cache.h"
#include "dispatch.h"
#include "types.h"

struct CodeReader;

namespace decoder2
{

using Dispatcher = DecodeError (*)(CodeReader& reader, uint8_t byte, CachedInstruction& instr);

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
