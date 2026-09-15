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

#include <cstdio>

#include "backend/diagnostics.h"

namespace common
{

struct TIO_Config
{
    // if true, nuked-sc55 (the library) will print messages to stderr
    bool enable_backend;

    // if true, the frontend will print messages to stderr
    bool enable_frontend;

    // where output will be written to
    FILE* output_file;

    // backend messages greater or equal to this category will be
    // printed if enable_backend is true
    Diag_Category min_backend_level;
};

// Reconfigures term_io subsystem. The defaults are as follows:
//
// - enable_backend: true
// - enable_frontend: true
// - output_file: stderr
void ConfigureTermIO(const TIO_Config& config);

void Printf(const char* format, ...);

} // namespace common
