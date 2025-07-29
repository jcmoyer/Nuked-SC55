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
};

// `error`: error code to convert to string
const char* ToCString(LoadRomsetError error);

struct LoadRomsetResult
{
    Romset romset;

    RomLoadStatusSet       loaded;
    RomCompletionStatusSet completion;
};

// `romset_info`: receives rom paths and rom data
// `rom_directory`: directory containing complete romset(s)
// `desired_romset`: romset the user wants to load; if empty string the first romset in the directory will be returned
// `legacy_loader`: use the same logic as nukeykt/Nuked-SC55
// `result`: receives the loaded romset and information about which roms were loaded
LoadRomsetError LoadRomset(AllRomsetInfo&               romset_info,
                           const std::filesystem::path& rom_directory,
                           std::string_view             desired_romset,
                           bool                         legacy_loader,
                           const RomOverrides&          overrides,
                           LoadRomsetResult&            result);

// `output`: where to write romset list
void PrintRomsets(FILE* output);

// `output`: where to write diagnostics to
// `error`: error to write diagnostics for
// `results`: results object to take diagnostics information from
// `info`: romset info passed to `LoadRomset`
void PrintLoadRomsetDiagnostics(FILE*                   output,
                                LoadRomsetError         error,
                                const LoadRomsetResult& result,
                                const AllRomsetInfo&    info);

} // namespace common
