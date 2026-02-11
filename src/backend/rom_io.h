#pragma once

#include <array>
#include <filesystem>
#include <vector>

#include "rom.h"

class HashedFileRegistry;

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

// Similar to `IsCompleteRomset`, except this function operates on `RomsetDefinition` which may be known earlier than a
// `RomsetInfo` is available.
bool GetDefinitionCompletion(const RomsetDefinition&   def,
                             const HashedFileRegistry& hashed_files,
                             const RomLocationSet&     location_mask,
                             RomCompletionStatusSet&   completion);

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

// Returns romsets identifiers whose complete romsets are contained in `hashed_files`. `location_mask` can be used
// to filter which roms are considered for the completeness of the romset. The test logic works the same as in
// `ContainsRomsetFiles`. `out_names` will be cleared before receiving the names.
void GetCompleteRomsetNames(const RomsetRegistry&     romsets,
                            const HashedFileRegistry& hashed_files,
                            const RomLocationSet&     location_mask,
                            StringVector&             out_names);

// Returns romsets identifiers whose partial romsets are contained in `hashed_files`. `location_mask` can be used
// to filter which roms are considered for the completeness of the romset. The test logic works the same as in
// `ContainsRomsetFiles`. `out_names` will be cleared before receiving the names.
void GetPartialRomsetNames(const RomsetRegistry&     romsets,
                           const HashedFileRegistry& hashed_files,
                           const RomLocationSet&     location_mask,
                           StringVector&             out_names);

// Returns true if all the roms in the romset named `name` is in `hashed_files`.
//
// `location_mask` allows the caller to control which roms will be tested. For rom locations used by the romset, a
// value of `true` enables the test and a value of `false` disables the test. The test succeeds if the hash for
// that location is in `hashed_files`. If a rom location is not used by the romset, the value is ignored.
bool ContainsRomsetFiles(const RomsetRegistry&     romsets,
                         std::string_view          name,
                         const HashedFileRegistry& hashed_files,
                         const RomLocationSet&     location_mask);

// If the romset given by `name` exists, `out_info` receives the paths and data contained for each rom in the
// romset.
//
// `location_mask` allows the caller to control which roms will be returned. The test logic works the same way as
// in `ContainsRomsetFiles`.
//
// Returns true if the definition exists and all the required roms exist in `hashed_files`. If this function returns
// false, `out_info` will still be populated with the contents of any roms that were found.
bool GetRomsetInfo(const RomsetRegistry&     romsets,
                   std::string_view          name,
                   const HashedFileRegistry& hashed_files,
                   const RomLocationSet&     location_mask,
                   RomsetInfo&               out_info);
