#pragma once

#include <array>
#include <span>
#include <string_view>

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

const char* RomsetName(Romset romset);

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
