#include <array>

#include "dispatch.h"
#include "dispatch_address.h"
#include "dispatch_handlers.h"

constexpr std::array<std::array<D_OpcodeHandler, 256>, 2> DECODE_TABLES_D8{{
    DefineGenericTable<MCU_Operand_Size::BYTE, Mode_Ad8_Rn>(),
    DefineGenericTable<MCU_Operand_Size::WORD, Mode_Ad8_Rn>(),
}};

constexpr std::array<std::array<D_OpcodeHandler, 256>, 2> DECODE_TABLES_D16{{
    DefineGenericTable<MCU_Operand_Size::BYTE, Mode_Ad16_Rn>(),
    DefineGenericTable<MCU_Operand_Size::WORD, Mode_Ad16_Rn>(),
}};

D_OpcodeHandler GetDispatcherd8Rn(uint8_t opcode, MCU_Operand_Size size)
{
    return DECODE_TABLES_D8[(size_t)size][opcode];
}

D_OpcodeHandler GetDispatcherd16Rn(uint8_t opcode, MCU_Operand_Size size)
{
    return DECODE_TABLES_D16[(size_t)size][opcode];
}
