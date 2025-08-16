#include <array>

#include "dispatch.h"
#include "dispatch_address.h"

constexpr std::array<std::array<D_OpcodeHandler, 256>, 2> POSTINC_DECODE_TABLES{{
    DefineGenericTable<MCU_Operand_Size::BYTE, Mode_ARn>(),
    DefineGenericTable<MCU_Operand_Size::WORD, Mode_ARn>(),
}};

D_OpcodeHandler GetDispatcherARn(uint8_t opcode, MCU_Operand_Size size)
{
    return POSTINC_DECODE_TABLES[(size_t)size][opcode];
}
