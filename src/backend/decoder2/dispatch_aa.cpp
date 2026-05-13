#include <array>

#include "dispatch_address.h"
#include "dispatchers.h"

namespace decoder2
{

constexpr std::array<std::array<Dispatcher, 256>, 2> DECODE_TABLES_AA8{{
    DefineGenericTable<Size::Byte, Mode_Aaa8>(),
    DefineGenericTable<Size::Word, Mode_Aaa8>(),
}};

Dispatcher GetDispatcherAaa8(uint8_t opcode, Size size)
{
    return DECODE_TABLES_AA8[(size_t)size][opcode];
}

constexpr std::array<std::array<Dispatcher, 256>, 2> DECODE_TABLES_AA16{{
    DefineGenericTable<Size::Byte, Mode_Aaa16>(),
    DefineGenericTable<Size::Word, Mode_Aaa16>(),
}};

Dispatcher GetDispatcherAaa16(uint8_t opcode, Size size)
{
    return DECODE_TABLES_AA16[(size_t)size][opcode];
}

} // namespace decoder2
