#include <array>

#include "decoder2/dispatch_address.h"
#include "decoder2/dispatchers.h"

namespace decoder2
{

constexpr std::array<std::array<Dispatcher, 256>, 2> POSTINC_DECODE_TABLES{{
    DefineGenericTable<Size::Byte, Mode_ARn>(),
    DefineGenericTable<Size::Word, Mode_ARn>(),
}};

Dispatcher GetDispatcherARn(uint8_t opcode, Size size)
{
    return POSTINC_DECODE_TABLES[(size_t)size][opcode];
}

} // namespace decoder2
