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

#include "diagnostics.h"

#include <cstdarg>
#include <cstdio>

static Diag_Callback Diag_CurrentCallback = Diag_DefaultCallback;

const char* ToCString(Diag_Category category)
{
    switch (category)
    {
    case Diag_Category::Debug:
        return "Debug";
    case Diag_Category::Error:
        return "Error";
    case Diag_Category::Warning:
        return "Warning";
    }
    return "unknown";
}

void Diag_DefaultCallback(Diag_Category category, std::string_view message)
{
    fprintf(stderr, "[%s] %.*s", ToCString(category), (int)message.size(), message.data());
}

void Diag_Printf(Diag_Category category, const char* format, ...)
{
    if (!Diag_CurrentCallback)
    {
        return;
    }

    // no message we print requires a larger buffer than this
    char buf[1024] = {0};

    va_list list;
    va_start(list, format);
    int len = vsnprintf(buf, sizeof(buf), format, list);
    va_end(list);

    if (len > 0)
    {
        Diag_CurrentCallback(category, std::string_view(buf, (size_t)len));
    }
}

void Diag_SetCallback(Diag_Callback callback)
{
    Diag_CurrentCallback = callback;
}
