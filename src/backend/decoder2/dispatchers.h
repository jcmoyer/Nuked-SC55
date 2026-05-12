#pragma once

#include "dispatch.h"
#include "mcu_opcodes.h"

namespace decoder2
{

// Top level decode table for instructions. Depending on `byte` the handler
// returned might decode a general form instruction or a special form
// instruction.
D_Handler GetDispatcherTop(uint8_t byte);

// Rn
// 1010[Sz]rrr | opcode [...]
D_OpcodeHandler GetDispatcherRn(uint8_t opcode, MCU_Operand_Size size);

// @Rn
// 1101[Sz]rrr | opcode [...]
D_OpcodeHandler GetDispatcherARn(uint8_t opcode, MCU_Operand_Size size);

// @(d:8,Rn)
// 1110[Sz]rrr | disp8 | opcode [...]
D_OpcodeHandler GetDispatcherAd8Rn(uint8_t opcode, MCU_Operand_Size size);

// @(d:16,Rn)
// 1111[Sz]rrr | disp16 | opcode [...]
D_OpcodeHandler GetDispatcherAd16Rn(uint8_t opcode, MCU_Operand_Size size);

// @-Rn
// 1011[Sz]rrr | opcode [...]
D_OpcodeHandler GetDispatcherAPreDecRn(uint8_t opcode, MCU_Operand_Size size);

// @Rn+
// 1100[Sz]rrr | opcode [...]
D_OpcodeHandler GetDispatcherAPostIncRn(uint8_t opcode, MCU_Operand_Size size);

// #xx:8
// 00000100 | imm8 | opcode [...]
D_OpcodeHandler GetDispatcherImm8(uint8_t opcode);

// #xx:16
// 00001100 | imm16 | opcode [...]
D_OpcodeHandler GetDispatcherImm16(uint8_t opcode);

// @aa:8
// 0000[Sz]101 | addr8 | opcode [...]
D_OpcodeHandler GetDispatcherAaa8(uint8_t opcode, MCU_Operand_Size size);

// @aa:16
// 0001[Sz]101 | addr16 | opcode [...]
D_OpcodeHandler GetDispatcherAaa16(uint8_t opcode, MCU_Operand_Size size);

} // namespace decoder2
