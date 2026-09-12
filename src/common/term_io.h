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
};

// Reconfigures term_io subsystem. The defaults are as follows:
//
// - enable_backend: true
// - enable_frontend: true
// - output_file: stderr
void ConfigureTermIO(const TIO_Config& config);

void Printf(const char* format, ...);

} // namespace common
