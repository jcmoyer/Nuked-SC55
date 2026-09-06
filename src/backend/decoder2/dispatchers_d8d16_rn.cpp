/*
 * Copyright (C) 2024-2026 J.C. Moyer
 *
 * This file is part of Nuked-SC55.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

// clang-format off
#include "decoder2/dispatchers.h"
// clang-format on

#include <array>

#include "decoder2/dispatchers_address.h"

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
