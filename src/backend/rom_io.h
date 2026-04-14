#pragma once

#include <array>
#include <cstring>
#include <filesystem>
#include <vector>

#include "rom.h"

enum class RomLoadStatus
{
    // rom loaded successfully
    Loaded,
    // rom could not be loaded - likely IO failure
    Failed,
    // rom not used by romset
    Unused,
};

const char* ToCString(RomLoadStatus status);

// Set of load statuses. Indexed by RomLocation.
using RomLoadStatusSet = std::array<RomLoadStatus, ROMLOCATION_COUNT>;

enum class RomCompletionStatus
{
    // rom is present
    Present,
    // rom is missing
    Missing,
    // rom is not used in this romset
    Unused,
};

const char* ToCString(RomCompletionStatus status);

// Set of completion statuses. Indexed by RomLocation.
using RomCompletionStatusSet = std::array<RomCompletionStatus, ROMLOCATION_COUNT>;

// For a single romset, this structure maps each rom in the set to a filename on disk and that file's contents.
struct RomsetInfo
{
    // Array indexed by RomLocation
    std::filesystem::path rom_paths[ROMLOCATION_COUNT]{};
    std::vector<uint8_t>  rom_data[ROMLOCATION_COUNT]{};

    // Release all rom_data for all roms in this romset.
    void PurgeRomData();

    // Returns true if at least one of `rom_path` or `rom_data` is populated for `location`.
    bool HasRom(RomLocation location) const;
};

// Sets `romset_info.rom_paths` relative to `base_path` using filenames for `romset`. Consult the `legacy_rom_names`
// constant in `rom_io.cpp` for the exact filenames.
//
// `location_mask` can be used to control which rom locations are populated. A value of `true` sets the corresponding
// path if it is used by the romset; otherwise that path will be skipped.
void SetRomsetFilenames(RomsetInfo&                  romset_info,
                        const std::filesystem::path& base_path,
                        Romset                       romset,
                        const RomLocationSet&        location_mask);

// Returns true if `info` contains all the files required to load `romset`. Missing roms will be reported in
// `missing`.
bool IsCompleteRomset(const RomsetInfo& info, Romset romset, RomCompletionStatusSet* status = nullptr);

size_t CountPresent(const RomCompletionStatusSet& status);

// For each `rom` in `info` this function loads the file referenced by `info.rom_paths[rom]` into `info.rom_data[rom]`.
// If `info.rom_data[rom]` is already populated, no data will be loaded from disk.
//
// Waveroms will be unscrambled at this point. If `info.rom_data[rom]` is provided by the caller, it should *not* be
// provided unscrambled.
//
// `rom` will only be loaded when `rom_data` is empty and `rom_path` is non-empty.
//
// To automatically determine elements of `rom_path`, populate a `HashedFileRegistry` and `RomsetRegistry` then
// use the `RomsetRegistry` to look up a specific romset in the `HashedFileRegistry`.
bool LoadRomset(RomsetInfo& info, RomLoadStatusSet* loaded);
