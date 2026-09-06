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

#include <source_location>
#include <utility>

#include "diagnostics.h"

template <typename R, typename T>
[[nodiscard]]
inline R RangeCast(T value, const std::source_location where = std::source_location::current())
{
    if (!std::in_range<R>(value)) [[unlikely]]
    {
        Diag_Printf(Diag_Category::Warning,
                    "%s:%s:%d: Cast value out of range\n",
                    where.file_name(),
                    where.function_name(),
                    (int)where.line());
    }
    return (R)value;
}
