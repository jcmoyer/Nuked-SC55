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

#include "common/term_io.h"

#include <cstdarg>

#include "backend/diagnostics.h"

namespace common
{

static TIO_Config g_config = {
    .enable_backend    = true,
    .enable_frontend   = true,
    .output_file       = stderr,
    .min_backend_level = Diag_Category::Warning,
};

static void BackendCallback(Diag_Category cat, std::string_view message)
{
    if (!g_config.enable_backend)
    {
        return;
    }

    if ((int)cat >= (int)g_config.min_backend_level)
    {
        Printf("%.*s", (int)message.size(), message.data());
    }
}

void InitTermIO()
{
    Diag_SetCallback(BackendCallback);
}

void ConfigureTermIO(const TIO_Config& config)
{
    g_config = config;
}

void SetMinBackendLevel(Diag_Category new_min)
{
    g_config.min_backend_level = new_min;
}

void Printf(const char* format, ...)
{
    if (!g_config.enable_frontend)
    {
        return;
    }

    // no message we print requires a larger buffer than this
    char buf[1024] = {0};

    va_list list;
    va_start(list, format);
    int len = vsnprintf(buf, sizeof(buf), format, list);
    va_end(list);

    fprintf(g_config.output_file, "%.*s", len, buf);
}

} // namespace common
