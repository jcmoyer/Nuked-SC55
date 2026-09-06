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

enum class Diag_Category
{
    Debug,
    Error,
    Warning,
};

const char* ToCString(Diag_Category category);

using Diag_Callback = void(*)(Diag_Category category, std::string_view message);

// The default callback handler which prints messages to `stderr`.
void Diag_DefaultCallback(Diag_Category category, std::string_view message);

// Sends a message from the backend to the callback.
void Diag_Printf(Diag_Category category, const char* format, ...);

// Sets the callback. This should be called before creating any emulators.
// Callbacks can run on any thread that calls functions in the backend. If
// there are multiple threads making such calls, the callback may need
// synchronization.
//
// Passing `nullptr` will disable logging entirely.
void Diag_SetCallback(Diag_Callback callback);
