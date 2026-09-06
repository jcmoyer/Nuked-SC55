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

#pragma once

#include <string_view>

namespace common
{

enum class ParseGainResult
{
    TooShort = 1,
    InvalidNumber,
    ParseFailed,
    OutOfRange,
};

ParseGainResult ParseGain(std::string_view str, float& out_gain);

float DbToScalar(float db);
float ScalarToDb(float scalar);

} // namespace common
