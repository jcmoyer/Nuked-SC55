#pragma once

#include <array>
#include <cstring>
#include <filesystem>
#include <unordered_map>
#include <vector>

#include "rom.h"

using SHA256Digest = std::array<uint8_t, 32>;

template <>
struct std::hash<SHA256Digest>
{
    size_t operator()(const SHA256Digest& digest) const
    {
        size_t result = 0;
        for (size_t i = 0; i < sizeof(SHA256Digest) / sizeof(size_t); ++i)
        {
            size_t block;
            memcpy(&block, &digest[i * sizeof(size_t)], sizeof(size_t));
            result ^= block;
        }
        return result;
    }
};

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

// Contains RomsetInfo for all supported romsets.
struct AllRomsetInfo
{
    // Array indexed by Romset
    RomsetInfo romsets[ROMSET_COUNT]{};

    // Release all rom_data for all romsets.
    void PurgeRomData();
};

// The first step for rom detection is to hash all the files in the rom directory under 4MB and retain their contents in
// memory until we've decided which ones to keep.
struct HashedFile
{
    std::filesystem::path path;
    std::vector<uint8_t>  data;
};

class HashedFileRegistry
{
public:
    void AddFile(SHA256Digest hash, HashedFile file);
    bool Contains(SHA256Digest hash) const;

    void Purge();

private:
    std::vector<HashedFile> m_files;
    // SHA256 to index in `files`
    std::unordered_map<SHA256Digest, size_t> m_hash_map;
};

bool HashAllFiles(const std::filesystem::path& base_path, HashedFileRegistry& registry);

struct RomHash
{
    SHA256Digest hash;
    RomLocation  location;

    auto operator<=>(const RomHash&) const = default;
};

constexpr RomHash NULL_HASH{{}, {}};

struct RomsetHashes
{
    const char* name;
    Romset      romset;
    RomHash     hashes[ROMLOCATION_COUNT];

    const RomHash* begin() const
    {
        return &hashes[0];
    }

    const RomHash* end() const
    {
        for (const auto& h : hashes)
        {
            if (h == NULL_HASH)
            {
                return &h;
            }
        }
        return &hashes[ROMLOCATION_COUNT];
    }
};

using StringVector = std::vector<std::string>;

class RomsetHashRegistry
{
public:
    RomsetHashRegistry() = default;

    void AddRomset(const RomsetHashes& romset);
    void GetCompleteRomsetNames(const HashedFileRegistry& hashed_files, StringVector& out_names);

    static RomsetHashRegistry CreateWithDefaultHashes();

private:
    std::vector<RomsetHashes> m_romsets;
    // Maps romset identifiers to index in `m_romsets`
    std::unordered_map<std::string, size_t> m_name_map;
};

// Scans files in `base_path` for specific rom filenames. Consult the `legacy_rom_names` constant in `emu.cpp` for the
// exact filename requirements.
//
// If `desired` is non-null, this function will use it as a hint to determine what filenames to examine. This function
// may also load `rom_data` for desired roms.
bool DetectRomsetsByFilename(const std::filesystem::path& base_path,
                             AllRomsetInfo&               all_info,
                             RomLocationSet*              desired = nullptr);

// Scans files in `base_path` for roms by hashing them. The locations of each rom will be made available in `info`. This
// will return *all* romsets in `base_path`.
//
// If any of the rom locations in `all_info` are already populated with a path or data, this function will not overwrite
// them.
//
// If `desired` is non-null, this function will use it as a hint to determine what hashes to consider. This function may
// also load `rom_data` for desired roms.
bool DetectRomsetsByHash(const std::filesystem::path& base_path,
                         AllRomsetInfo&               all_info,
                         RomLocationSet*              desired = nullptr);

// Returns true if `all_info` contains all the files required to load `romset`. Missing roms will be reported in
// `missing`.
bool IsCompleteRomset(const AllRomsetInfo& all_info, Romset romset, RomCompletionStatusSet* status = nullptr);

size_t CountPresent(const RomCompletionStatusSet& status);

// Picks the first complete romset in `all_info` and writes it to `out_romset`. If multiple romsets are present, the one
// returned is unspecified. Returns true if successful, or false if there are no complete romsets.
bool PickCompleteRomset(const AllRomsetInfo& all_info, Romset& out_romset);

// For each `rom` in `romset`, this function loads the file referenced by `all_info.romsets[romset].rom_paths[rom]` into
// the corresponding `rom_data`. Waveroms will be unscrambled at this point.
//
// `rom` will only be loaded when `rom_data` is empty and `rom_path` is non-empty.
//
// To automatically determine rom_paths, call `DetectRomsetsByHash` with a directory containing roms.
//
// Roms that were loaded successfully will be marked as true in `loaded`.
bool LoadRomset(Romset romset, AllRomsetInfo& all_info, RomLoadStatusSet* loaded = nullptr);
