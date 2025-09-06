#include <array>

#include "dispatch.h"
#include "dispatch_address.h"

constexpr std::array<std::array<D_OpcodeHandler, 256>, 2> PREDEC_DECODE_TABLES{{
    DefineGenericTable<MCU_Operand_Size::BYTE, Mode_PreDecRn>(),
    DefineGenericTable<MCU_Operand_Size::WORD, Mode_PreDecRn>(),
}};

D_OpcodeHandler GetDispatcherPreDecRn(uint8_t opcode, MCU_Operand_Size size)
{
    return PREDEC_DECODE_TABLES[(size_t)size][opcode];
}
