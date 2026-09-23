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

#include "file_hashing.h"
#include "rom_io.h"

namespace common
{

using RomOverrides = std::array<std::filesystem::path, ROMLOCATION_COUNT>;

enum class LoadRomsetError
{
    // User passed a romset name that we don't recognize.
    InvalidRomsetName = 1,

    // Hashing files failed due to an IO error.
    HashFilesIoFailure,

    // User gave a valid romset family, but we couldn't find a romset for
    // it. `romset` will contain the family.
    RequestedFamilyNotFound,

    // User requested a romset family, but there are multiple suitable romsets
    // in the rom directory. `romset` will contain the family.
    RequestedFamilyAmbiguous,

    // Tried to pick a romset automatically, but none of them were
    // complete. `romset` will be invalid.
    NoCompleteRomsets,

    // User requested a valid romset, but there are missing roms. `romset` and
    // `completion` will be valid.
    IncompleteRomset,

    // User requested a valid romset, but one or more roms could not be
    // read. `romset`, `completion`, and `loaded` will be valid.
    RomLoadFailed,

    // Caller passed an invalid loader.
    InvalidLoader,
};

// `error`: error code to convert to string
const char* ToCString(LoadRomsetError error);

struct LoaderRegistries
{
    HashedFileRegistry hashes;
    RomsetRegistry     romsets;
};

struct LoadRomsetResult
{
    // Contains the romset family. Depending on what the user provides as
    // `requested_romset`, this value may or may not be valid.
    Romset romset;

    // On successful load, this field contains the paths and data for each rom
    // location.
    RomsetInfo romset_info;

    // True for each location the loader needed to determine a rom for. False
    // implies the rom is overridden. Values for locations not used by the
    // romset are not meaningful.
    //
    // This value will always be populated.
    RomLocationSet rom_mask;

    // This is the value passed to LoadRomset.
    //
    // This value will always be populated.
    RomOverrides overrides;

    RomLoadStatusSet       loaded;
    RomCompletionStatusSet completion;

    LoaderRegistries registries;

    // User-requested name. This is the string passed to LoadRomset.
    //
    // This value will always be populated.
    std::string requested_name;

    // Name the loader decided on.
    std::string picked_name;

    // Frees any allocated buffers from loading roms and hashing files. This
    // happens automatically when the result object goes out of scope, but it
    // is useful in cases where we keep the result object around and no longer
    // need the data.
    void Purge();
};

enum class RomLoader
{
    // takes the SHA256 hash of files in the rom directory and locates roms to load regardless of filename
    Hashing,

    // load specific filenames using the same logic as nukeykt/Nuked-SC55
    Legacy,
};

// `rom_directory`: directory containing complete romset(s)
// `requested_romset`: romset the user wants to load; if empty this defaults to
//                     mk2 for the legacy loader and the first detected romset
//                     in `rom_directory` for the hashing loader
// `loader`: which loader to use
// `overrides`: overrides for specific roms in the romset
// `result`: receives the results of loading `desired_romset`
//
// Returns `LoadRomsetError{}` on success.
LoadRomsetError LoadRomset(const std::filesystem::path& rom_directory,
                           std::string_view             requested_romset,
                           RomLoader                    loader,
                           const RomOverrides&          overrides,
                           LoadRomsetResult&            result);

void PrintRomsets();

// `error`: error to write diagnostics for
// `results`: results object to take diagnostics information from
void PrintLoadRomsetDiagnostics(LoadRomsetError error, const LoadRomsetResult& result);

} // namespace common
