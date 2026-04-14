#pragma once

#include <array>
#include <span>
#include <string_view>

#include "sha256.h"
#include "string_map.h"
#include "string_vector.h"

// A Romset represents a family of roms. Each Romset may have multiple valid sets of roms. This is the case when a
// Romset has multiple versions.
enum class Romset
{
    MK2,
    ST,
    MK1,
    CM300,
    JV880,
    SCB55,
    RLP3237,
    SC155,
    SC155MK2,
};

constexpr size_t ROMSET_COUNT = 9;

// Returns a formatted romset name suitable for displaying to users.
const char* RomsetName(Romset romset);

// Returns a parsable romset name. Parsable romset names are also the canonical
// representation for a romset family.
const char* ParsableRomsetName(Romset romset);

bool ParseRomsetName(std::string_view name, Romset& romset);

std::span<const char*> GetParsableRomsetNames();

// Symbolic name for the various roms used by the emulator. A Romset consists of several roms each at a distinct
// RomLocation. A Romset does not require all RomLocations to be populated.
enum class RomLocation
{
    // MCU roms
    ROM1,
    ROM2,

    // Sub-MCU roms
    SMROM,

    // PCM roms
    WAVEROM1,
    WAVEROM2,
    WAVEROM3,
    WAVEROM_CARD,
    WAVEROM_EXP,
};

constexpr size_t ROMLOCATION_COUNT = 8;

const char* ToCString(RomLocation location);

// Set of rom locations. Indexed by RomLocation.
using RomLocationSet = std::array<bool, ROMLOCATION_COUNT>;

constexpr RomLocationSet ROMLOCATION_ALL{true, true, true, true, true, true, true, true};

// Returns true if `location` represents a waverom location.
bool IsWaverom(RomLocation location);

bool IsOptionalRom(Romset romset, RomLocation location);
bool IsRequiredRom(Romset romset, RomLocation location);

struct RomHash
{
    SHA256_Digest hash;
    RomLocation   location;

    auto operator<=>(const RomHash&) const = default;
};

constexpr RomHash NULL_HASH{{}, {}};

struct RomsetDefinition
{
    const char* name;
    Romset      romset;
    RomHash     hashes[ROMLOCATION_COUNT];

    constexpr RomHash* begin()
    {
        return &hashes[0];
    }

    constexpr const RomHash* begin() const
    {
        return &hashes[0];
    }

    constexpr RomHash* end()
    {
        for (auto& h : hashes)
        {
            if (h == NULL_HASH)
            {
                return &h;
            }
        }
        return &hashes[ROMLOCATION_COUNT];
    }

    constexpr const RomHash* end() const
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

    constexpr void ReplaceHash(RomLocation rom, const SHA256_Digest& hash)
    {
        for (RomHash& h : *this)
        {
            if (h.location == rom)
            {
                h.hash = hash;
            }
        }
    }
};

// Contains metadata for romsets. Romsets are registered with instances of this type by name (e.g. "mk1-v1.00") along
// with the hashes of any roms in that romset. After registration, this type can be used to efficiently query which of
// those romsets exist on disk.
class RomsetRegistry
{
public:
    // Constructs an empty registry.
    RomsetRegistry() = default;

    // Returns the definition for `name` if it exists or `nullptr` otherwise.
    // The returned pointer is invalidated if the registry is modified.
    const RomsetDefinition* GetDefinition(std::string_view name) const;

    // Adds `romset` to the registry.
    void AddRomset(const RomsetDefinition& romset);

    // Returns all of the names registered with the registry. `out_names` will be cleared before receiving the names.
    void GetAllRomsetNames(StringVector& out_names) const;

    // Returns all of the names under a specific romset family. `out_names` will be cleared before receiving the names.
    void GetNamesForFamily(Romset romset, StringVector& out_names) const;

    // Returns true if there is metadata associated with `name`. Note that this does NOT return whether or not the file
    // was located on disk. For that functionality, use `ContainsRomsetFiles`.
    bool ContainsRomset(std::string_view name) const;

    // Returns true if the romset given by `name` exists and `out_family` receives the romset family.
    bool GetRomsetFamily(std::string_view name, Romset& out_family) const;

    // Iterators over the contained definitions. Iterators are invalidated if the registry is modified.
    const RomsetDefinition* begin() const
    {
        return m_romsets.data();
    }

    const RomsetDefinition* end() const
    {
        return m_romsets.data() + m_romsets.size();
    }

    // Creates a registry containing standard, supported romsets.
    static RomsetRegistry CreateWithDefaultHashes();

private:
    std::vector<RomsetDefinition> m_romsets;
    // Maps romset identifiers to index in `m_romsets`
    StringMap<size_t> m_name_map;
};
