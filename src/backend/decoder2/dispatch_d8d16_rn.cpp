#include <array>

#include "dispatch.h"
#include "dispatch_address.h"
#include "dispatch_handlers.h"

constexpr std::array<std::array<D_OpcodeHandler, 256>, 2> DECODE_TABLES{{
    DefineGenericTable<MCU_Operand_Size::BYTE, Mode_d8d16_Rn>(),
    DefineGenericTable<MCU_Operand_Size::WORD, Mode_d8d16_Rn>(),
}};

D_OpcodeHandler GetDispatcherd8d16Rn(uint8_t opcode, MCU_Operand_Size size)
{
    return DECODE_TABLES[(size_t)size][opcode];
}
