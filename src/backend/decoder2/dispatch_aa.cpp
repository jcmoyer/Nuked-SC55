#include <array>

#include "dispatch.h"
#include "dispatch_address.h"

namespace decoder2
{

constexpr std::array<std::array<D_OpcodeHandler, 256>, 2> DECODE_TABLES_AA8{{
    DefineGenericTable<Size::Byte, Mode_Aaa8>(),
    DefineGenericTable<Size::Word, Mode_Aaa8>(),
}};

D_OpcodeHandler GetDispatcherAaa8(uint8_t opcode, Size size)
{
    return DECODE_TABLES_AA8[(size_t)size][opcode];
}

constexpr std::array<std::array<D_OpcodeHandler, 256>, 2> DECODE_TABLES_AA16{{
    DefineGenericTable<Size::Byte, Mode_Aaa16>(),
    DefineGenericTable<Size::Word, Mode_Aaa16>(),
}};

D_OpcodeHandler GetDispatcherAaa16(uint8_t opcode, Size size)
{
    return DECODE_TABLES_AA16[(size_t)size][opcode];
}

} // namespace decoder2
