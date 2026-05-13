#include <array>

#include "dispatch.h"
#include "dispatch_address.h"

namespace decoder2
{

constexpr std::array<std::array<D_OpcodeHandler, 256>, 2> POSTINC_DECODE_TABLES{{
    DefineGenericTable<Size::Byte, Mode_ARn>(),
    DefineGenericTable<Size::Word, Mode_ARn>(),
}};

D_OpcodeHandler GetDispatcherARn(uint8_t opcode, Size size)
{
    return POSTINC_DECODE_TABLES[(size_t)size][opcode];
}

} // namespace decoder2
