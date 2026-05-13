#include <array>

#include "dispatch.h"
#include "dispatch_address.h"

namespace decoder2
{

constexpr std::array<std::array<D_OpcodeHandler, 256>, 2> PREDEC_DECODE_TABLES{{
    DefineGenericTable<Size::Byte, Mode_APreDecRn>(),
    DefineGenericTable<Size::Word, Mode_APreDecRn>(),
}};

D_OpcodeHandler GetDispatcherAPreDecRn(uint8_t opcode, Size size)
{
    return PREDEC_DECODE_TABLES[(size_t)size][opcode];
}

} // namespace decoder2
