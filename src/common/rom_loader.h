#pragma once

#include "rom_io.h"

namespace common
{

using RomOverrides = std::array<std::filesystem::path, ROMLOCATION_COUNT>;

enum class LoadRomsetError
{
    // `desired_romset` should be one of the strings returned by `GetParsableRomsetNames`
    InvalidRomsetName = 1,

    DetectionFailed,

    // tried to autodetect a romset, but none of them were complete
    NoCompleteRomsets,

    // picked a romset, but it has missing roms; they will be available through `completion`
    IncompleteRomset,

    // loaded roms will be available through `loaded`
    RomLoadFailed,

    // user requested a romset family, but there are multiple suitable romsets in the rom directory
    AmbiguousRomset,
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
    Romset romset;

    // on successful load, this field contains the paths and data for each rom location
    RomsetInfo romset_info;

    RomLoadStatusSet       loaded;
    RomCompletionStatusSet completion;

    LoaderRegistries registries;

    std::string picked_name;

    // Frees any allocated buffers from loading roms and hashing files. This happens automatically when the result
    // object goes out of scope, but it is useful in cases where we keep the result object around and no longer need
    // the data.
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
// `desired_romset`: romset the user wants to load; if empty string the first romset in the directory will be returned
// `loader`: which loader to use
// `overrides`: overrides for specific roms in the romset
// `result`: receives the results of loading `desired_romset`
//
// Returns `LoadRomsetError{}` on success.
LoadRomsetError LoadRomset(const std::filesystem::path& rom_directory,
                           std::string_view             desired_romset,
                           RomLoader                    loader,
                           const RomOverrides&          overrides,
                           LoadRomsetResult&            result);

// `output`: where to write romset list
void PrintRomsets(FILE* output);

// `output`: where to write diagnostics to
// `error`: error to write diagnostics for
// `results`: results object to take diagnostics information from
void PrintLoadRomsetDiagnostics(FILE* output, LoadRomsetError error, const LoadRomsetResult& result);

} // namespace common
