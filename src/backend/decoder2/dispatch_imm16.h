#pragma once

#include "dispatch.h"
#include "mcu_opcodes.h"

D_OpcodeHandler GetDispatcherimm16(uint8_t opcode, MCU_Operand_Size size);
