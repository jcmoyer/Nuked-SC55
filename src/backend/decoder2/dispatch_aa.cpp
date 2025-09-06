#include <array>

#include "dispatch.h"
#include "dispatch_address.h"
#include "dispatch_handlers.h"

constexpr std::array<std::array<D_OpcodeHandler, 256>, 2> DECODE_TABLES_AA8{{
    DefineGenericTable<MCU_Operand_Size::BYTE, Mode_Aa8>(),
    DefineGenericTable<MCU_Operand_Size::WORD, Mode_Aa8>(),

}};

D_OpcodeHandler GetDispatcherAa8(uint8_t opcode, MCU_Operand_Size size)
{
    return DECODE_TABLES_AA8[(size_t)size][opcode];
}

constexpr std::array<std::array<D_OpcodeHandler, 256>, 2> DECODE_TABLES_AA16{{
    DefineGenericTable<MCU_Operand_Size::BYTE, Mode_Aa16>(),
    DefineGenericTable<MCU_Operand_Size::WORD, Mode_Aa16>(),
}};

D_OpcodeHandler GetDispatcherAa16(uint8_t opcode, MCU_Operand_Size size)
{
    return DECODE_TABLES_AA16[(size_t)size][opcode];
}
