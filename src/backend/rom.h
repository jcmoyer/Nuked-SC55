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

// Helper type for constructing RomsetDefinitions at compile time. An array of
// `RomHashLocationPair` has a dense representation, but can be converted to
// the sparse representation required by `RomsetHashes` at compile time. This
// is provided for convenience because writing arrays with holes is
// error-prone.
struct RomHashLocationPair
{
    SHA256_Digest hash;
    RomLocation   location;
};

// This is a sparse array containing SHA256 hashes for each possible rom in a
// romset. Most romsets will only use 5 slots in the array, so it is necessary
// to check that a rom is present before attempting to use the hash for a rom
// location. This can be done manually or by using `GetValidLocations` to skip
// holes in the array.
class RomsetHashes
{
public:
    constexpr RomsetHashes() = default;

    // Expands a `RomHashLocationPair` list into a `RomsetHashes`.
    constexpr RomsetHashes(std::initializer_list<RomHashLocationPair> pairs)
    {
        for (const RomHashLocationPair& pair : pairs)
        {
            m_hashes[(size_t)pair.location] = pair.hash;
        }
    }

    constexpr const SHA256_Digest& operator[](RomLocation index) const
    {
        return m_hashes[(size_t)index];
    }

    constexpr SHA256_Digest& operator[](RomLocation index)
    {
        return m_hashes[(size_t)index];
    }

    constexpr bool ContainsHash(RomLocation rom) const
    {
        return (*this)[rom] != SHA256_Digest{};
    }

    class LocationRange;

    class LocationIterator
    {
    public:
        constexpr LocationIterator& operator++()
        {
            m_raw_location = m_parent.FindNextLocation(m_raw_location + 1);
            return *this;
        }

        constexpr bool operator==(const LocationIterator& rhs)
        {
            return &m_parent == &rhs.m_parent && m_raw_location == rhs.m_raw_location;
        }

        constexpr bool operator!=(const LocationIterator& rhs)
        {
            return !(*this == rhs);
        }

        constexpr RomLocation operator*() const
        {
            return (RomLocation)m_raw_location;
        }

    private:
        friend class LocationRange;

        constexpr LocationIterator(const RomsetHashes& parent)
            : m_parent(parent),
              m_raw_location(ROMLOCATION_COUNT)
        {
        }

        constexpr LocationIterator(const RomsetHashes& parent, size_t pos)
            : m_parent(parent),
              m_raw_location(pos)
        {
        }

        const RomsetHashes& m_parent;
        size_t              m_raw_location;
    };

    class LocationRange
    {
    public:
        constexpr LocationIterator begin() const
        {
            return m_first;
        }

        constexpr LocationIterator end() const
        {
            return m_last;
        }

    private:
        friend class RomsetHashes;

        constexpr LocationRange(const RomsetHashes& parent)
            : m_first(parent, parent.FindNextLocation(0)),
              m_last(parent)
        {
        }

        LocationIterator m_first, m_last;
    };

    constexpr LocationRange GetValidLocations() const
    {
        return LocationRange(*this);
    }

private:
    constexpr size_t FindNextLocation(size_t start) const
    {
        for (size_t i = start; i < ROMLOCATION_COUNT; ++i)
        {
            if (ContainsHash((RomLocation)i))
            {
                return i;
            }
        }
        return ROMLOCATION_COUNT;
    }

    std::array<SHA256_Digest, ROMLOCATION_COUNT> m_hashes{};
};

// Contains a complete description of a romset.
struct RomsetDefinition
{
    const char*  name;
    Romset       romset;
    RomsetHashes hashes;

    constexpr const SHA256_Digest& GetHash(RomLocation rom) const
    {
        return hashes[rom];
    }

    constexpr bool ContainsHash(RomLocation rom) const
    {
        return hashes.ContainsHash(rom);
    }

    constexpr void ReplaceHash(RomLocation rom, const SHA256_Digest& hash)
    {
        hashes[rom] = hash;
    }

    // Returns a range that iterates over only the RomLocations used by this set.
    constexpr RomsetHashes::LocationRange GetValidLocations() const
    {
        return hashes.GetValidLocations();
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

    // Adds `romset` to the registry. Returns true if successful and false if
    // the registry already contains a definition with the same name.
    bool AddRomset(const RomsetDefinition& romset);

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
