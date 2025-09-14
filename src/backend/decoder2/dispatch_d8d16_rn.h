#pragma once

#include "dispatch.h"
#include "mcu_opcodes.h"

D_OpcodeHandler GetDispatcherd8Rn(uint8_t opcode, MCU_Operand_Size size);
D_OpcodeHandler GetDispatcherd16Rn(uint8_t opcode, MCU_Operand_Size size);
