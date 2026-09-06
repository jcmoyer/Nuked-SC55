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

#include "decoder2/address_modes.h"

#include <utility>

namespace decoder2
{

const char* ToCString(AddressMode mode)
{
    switch (mode)
    {
    case AddressMode::Rn:
        return "Rn";
    case AddressMode::ARn:
        return "ARn";
    case AddressMode::Ad8_Rn:
        return "Ad8_Rn";
    case AddressMode::Ad16_Rn:
        return "Ad16_Rn";
    case AddressMode::APreDecRn:
        return "APreDecRn";
    case AddressMode::APostIncRn:
        return "APostIncRn";
    case AddressMode::Aaa8:
        return "Aaa8";
    case AddressMode::Aaa16:
        return "Aaa16";
    case AddressMode::imm8:
        return "imm8";
    case AddressMode::imm16:
        return "imm16";
    }
    std::unreachable();
}

} // namespace decoder2
