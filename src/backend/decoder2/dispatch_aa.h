#pragma once

#include "dispatch.h"
#include "mcu_opcodes.h"

// @aa:8:  0000[Sz]101 | opcode [...]
D_OpcodeHandler GetDispatcherAaa8(uint8_t opcode, MCU_Operand_Size size);

// @aa:16: 0001[Sz]101 | opcode [...]
D_OpcodeHandler GetDispatcherAaa16(uint8_t opcode, MCU_Operand_Size size);
