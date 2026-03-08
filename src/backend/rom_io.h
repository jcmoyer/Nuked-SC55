#pragma once

#include <array>
#include <cstring>
#include <filesystem>
#include <string_view>
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

// Contains the path and contents of a hashed file.
struct HashedFile
{
    std::filesystem::path path;
    std::vector<uint8_t>  data;
};

// Contains a list of hashed files and provides constant-time lookup by hash.
class HashedFileRegistry
{
public:
    void AddFile(SHA256Digest hash, HashedFile file);
    bool Contains(SHA256Digest hash) const;

    // The returned pointer is invalidated when the registry is modified.
    // This function returns `nullptr` when `hash` is not in the registry.
    const HashedFile* GetFile(SHA256Digest hash) const;

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

// Allows using string_view with unordered_map<string, ...>
struct TransparentStringHash
{
    using is_transparent = void;

    size_t operator()(const std::string& s) const
    {
        return std::hash<std::string>{}(s);
    }

    size_t operator()(std::string_view s) const
    {
        return std::hash<std::string_view>{}(s);
    }
};

// Contains metadata for romsets. Romset metadata is registered with instances of this type, after which this type can
// used to efficiently query which of those romsets exist on disk.
class RomsetHashRegistry
{
public:
    // Constructs an empty registry.
    RomsetHashRegistry() = default;

    // Adds `romset` to the registry.
    void AddRomset(const RomsetHashes& romset);

    // Returns all the names registered with the romset.
    void GetAllRomsetNames(StringVector& out_names) const;

    // Returns romsets identifiers that are contained in `hashed_files`. `location_mask` can be used to filter which
    // roms are considered for the completeness of the romset. The test logic works the same as in
    // `ContainsRomsetFiles`.
    void GetCompleteRomsetNames(const HashedFileRegistry& hashed_files,
                                StringVector&             out_names,
                                const RomLocationSet&     location_mask) const;

    // Returns true if there is metadata associated with `name`. Note that this does NOT return whether or not the file
    // was located on disk. For that functionality, use `ContainsRomsetFiles`.
    bool ContainsRomsetMetadata(std::string_view name) const;

    // Returns true if all the roms in the romset named `name` is in `hashed_files`.
    //
    // `location_mask` allows the caller to control which roms will be tested. For rom locations used by the romset, a
    // value of `true` enables the test and a value of `false` disables the test. The test succeeds if the hash for that
    // location is in `hashed_files`. If a rom location is not used by the romset, the value is ignored.
    bool ContainsRomsetFiles(const HashedFileRegistry& hashed_files,
                             std::string_view          name,
                             const RomLocationSet&     location_mask) const;

    // Returns true if the romset given by `name` exists and `out_family` receives the romset family.
    bool GetRomsetFamily(std::string_view name, Romset& out_family) const;

    // If the romset given by `name` exists, `out_info` receives the paths and data contained for each rom in the
    // romset.
    //
    // `location_mask` allows the caller to control which roms will be returned. The test logic works the same way as in
    // `ContainsRomsetFiles`.
    bool GetRomsetInfo(const HashedFileRegistry& hashed_files,
                       std::string_view          name,
                       const RomLocationSet&     location_mask,
                       RomsetInfo&               out_info) const;

    // Creates a registry containing standard, supported romsets.
    static RomsetHashRegistry CreateWithDefaultHashes();

private:
    std::vector<RomsetHashes> m_romsets;
    // Maps romset identifiers to index in `m_romsets`
    std::unordered_map<std::string, size_t, TransparentStringHash, std::equal_to<void>> m_name_map;
};

// Sets `romset_info.rom_paths` relative to `base_path` using filenames for `romset`. Consult the `legacy_rom_names`
// constant in `emu.cpp` for the exact filenames.
//
// `location_mask` can be used to control which rom locations are populated. A value of `true` sets the corresponding
// path if it is used by the romset; otherwise that path will be skipped.
bool SetRomsetFilenames(RomsetInfo&                  romset_info,
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
// To automatically determine elements of `rom_path`, populate a `HashedFileRegistry` and `RomsetHashRegistry` then
// use the `RomsetHashRegistry` to query
bool LoadRomset(RomsetInfo& info, RomLoadStatusSet* loaded);
