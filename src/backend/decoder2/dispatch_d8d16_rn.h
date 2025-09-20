#pragma once

#include "dispatch.h"
#include "mcu_opcodes.h"

D_OpcodeHandler GetDispatcherAd8Rn(uint8_t opcode, MCU_Operand_Size size);
D_OpcodeHandler GetDispatcherAd16Rn(uint8_t opcode, MCU_Operand_Size size);
