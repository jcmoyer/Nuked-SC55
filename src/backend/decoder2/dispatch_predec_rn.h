#pragma once

#include "dispatch.h"
#include "mcu_opcodes.h"

D_OpcodeHandler GetDispatcherPreDecRn(uint8_t opcode, MCU_Operand_Size size);
