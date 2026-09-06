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

#include <string>
#include <string_view>
#include <unordered_map>

// Allows using string_view with unordered_map<string, ...>
struct TransparentStringHash
{
    using is_transparent = void;

    size_t operator()(const std::string& s) const
    {
        return std::hash<std::string>{}(s);
    }

    size_t operator()(std::string_view s) const
    {
        return std::hash<std::string_view>{}(s);
    }
};

template <typename ValueType>
using StringMap = std::unordered_map<std::string, ValueType, TransparentStringHash, std::equal_to<void>>;
