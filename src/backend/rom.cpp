#include "rom.h"

const char* rs_name[ROMSET_COUNT] = {
    "SC-55mk2",
    "SC-55st",
    "SC-55mk1",
    "CM-300/SCC-1",
    "JV-880",
    "SCB-55",
    "RLP-3237",
    "SC-155",
    "SC-155mk2"
};

const char* rs_name_simple[ROMSET_COUNT] = {
    "mk2",
    "st",
    "mk1",
    "cm300",
    "jv880",
    "scb55",
    "rlp3237",
    "sc155",
    "sc155mk2"
};

// This is a matrix where each row is a romset, and each column is a RomLocation
constexpr RomLocationSet REQUIRED_ROMS[ROMSET_COUNT] = {
    // MK2
    {true, true, true, true, true, false, false, false},
    // ST
    {true, true, true, true, true, false, false, false},
    // MK1
    {true, true, false, true, true, true, false, false},
    // CM300
    {true, true, false, true, true, true, false, false},
    // JV880
    {true, true, false, true, true, false, false, false},
    // SCB55
    {true, true, false, true, false, true, false, false},
    // RLP3237
    {true, true, false, true, false, false, false, false},
    // SC155
    {true, true, false, true, true, true, false, false},
    // SC155MK2
    {true, true, true, true, true, false, false, false},
};

const char* RomsetName(Romset romset)
{
    return rs_name[(size_t)romset];
}

bool ParseRomsetName(std::string_view name, Romset& romset)
{
    for (size_t i = 0; i < ROMSET_COUNT; ++i)
    {
        if (rs_name_simple[i] == name)
        {
            romset = (Romset)i;
            return true;
        }
    }
    return false;
}

std::span<const char*> GetParsableRomsetNames()
{
    return rs_name_simple;
}

bool IsWaverom(RomLocation location)
{
    switch (location)
    {
    case RomLocation::WAVEROM1:
    case RomLocation::WAVEROM2:
    case RomLocation::WAVEROM3:
    case RomLocation::WAVEROM_CARD:
    case RomLocation::WAVEROM_EXP:
        return true;
    default:
        return false;
    }
}

const char* ToCString(RomLocation location)
{
    switch (location)
    {
    case RomLocation::ROM1:
        return "ROM1";
    case RomLocation::ROM2:
        return "ROM2";
    case RomLocation::SMROM:
        return "SMROM";
    case RomLocation::WAVEROM1:
        return "WAVEROM1";
    case RomLocation::WAVEROM2:
        return "WAVEROM2";
    case RomLocation::WAVEROM3:
        return "WAVEROM3";
    case RomLocation::WAVEROM_CARD:
        return "WAVEROM_CARD";
    case RomLocation::WAVEROM_EXP:
        return "WAVEROM_EXP";
    }
    return "invalid location";
}

bool IsOptionalRom(Romset romset, RomLocation location)
{
    return romset == Romset::JV880 &&
           (location == RomLocation::WAVEROM_CARD || location == RomLocation::WAVEROM_EXP);
}

bool IsRequiredRom(Romset romset, RomLocation location)
{
    return REQUIRED_ROMS[(size_t)romset][(size_t)location];
}
