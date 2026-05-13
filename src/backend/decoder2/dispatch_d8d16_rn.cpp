#include <array>

#include "dispatch_address.h"
#include "dispatchers.h"

namespace decoder2
{

constexpr std::array<std::array<Dispatcher, 256>, 2> DECODE_TABLES_D8{{
    DefineGenericTable<Size::Byte, Mode_Ad8_Rn>(),
    DefineGenericTable<Size::Word, Mode_Ad8_Rn>(),
}};

constexpr std::array<std::array<Dispatcher, 256>, 2> DECODE_TABLES_D16{{
    DefineGenericTable<Size::Byte, Mode_Ad16_Rn>(),
    DefineGenericTable<Size::Word, Mode_Ad16_Rn>(),
}};

Dispatcher GetDispatcherAd8Rn(uint8_t opcode, Size size)
{
    return DECODE_TABLES_D8[(size_t)size][opcode];
}

Dispatcher GetDispatcherAd16Rn(uint8_t opcode, Size size)
{
    return DECODE_TABLES_D16[(size_t)size][opcode];
}

} // namespace decoder2
