#include <array>

#include "decoder2/dispatch_address.h"
#include "decoder2/dispatchers.h"

namespace decoder2
{

constexpr std::array<std::array<Dispatcher, 256>, 2> PREDEC_DECODE_TABLES{{
    DefineGenericTable<Size::Byte, Mode_APreDecRn>(),
    DefineGenericTable<Size::Word, Mode_APreDecRn>(),
}};

Dispatcher GetDispatcherAPreDecRn(uint8_t opcode, Size size)
{
    return PREDEC_DECODE_TABLES[(size_t)size][opcode];
}

} // namespace decoder2
