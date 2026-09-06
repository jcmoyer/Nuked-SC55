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

constexpr std::array<std::array<Dispatcher, 256>, 2> POSTINC_DECODE_TABLES{{
    DefineGenericTable<Size::Byte, Mode_APostIncRn>(),
    DefineGenericTable<Size::Word, Mode_APostIncRn>(),
}};

Dispatcher GetDispatcherAPostIncRn(uint8_t opcode, Size size)
{
    return POSTINC_DECODE_TABLES[(size_t)size][opcode];
}

} // namespace decoder2
