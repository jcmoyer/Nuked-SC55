#include <array>

#include "dispatch_address.h"
#include "dispatchers.h"

namespace decoder2
{

constexpr std::array<std::array<Dispatcher, 256>, 2> POSTINC_DECODE_TABLES{{
    DefineGenericTable<Size::Byte, Mode_APostIncRn>(),
    DefineGenericTable<Size::Word, Mode_APostIncRn>(),
}};

Dispatcher GetDispatcherAPostIncRn(uint8_t opcode, Size size)
{
    return POSTINC_DECODE_TABLES[(size_t)size][opcode];
}

} // namespace decoder2
