#include "rom.h"

#include "standard_romsets.h"

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

const char* ParsableRomsetName(Romset romset)
{
    return rs_name_simple[(size_t)romset];
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

const RomsetDefinition* RomsetRegistry::GetDefinition(std::string_view name) const
{
    const auto it = m_name_map.find(name);

    if (it == m_name_map.end())
    {
        return nullptr;
    }

    return &m_romsets[it->second];
}

void RomsetRegistry::AddRomset(const RomsetDefinition& romset)
{
    const size_t index = m_romsets.size();
    m_name_map.insert(std::make_pair(romset.name, index));
    m_romsets.push_back(romset);
}

void RomsetRegistry::GetAllRomsetNames(StringVector& out_names) const
{
    out_names.clear();
    for (const auto& romset : m_romsets)
    {
        out_names.emplace_back(romset.name);
    }
}

// Returns all the names under a specific romset family.
void RomsetRegistry::GetNamesForFamily(Romset romset, StringVector& out_names) const
{
    out_names.clear();
    for (const auto& def : m_romsets)
    {
        if (def.romset == romset)
        {
            out_names.emplace_back(def.name);
        }
    }
}

bool RomsetRegistry::ContainsRomset(std::string_view name) const
{
    return m_name_map.contains(name);
}

bool RomsetRegistry::GetRomsetFamily(std::string_view name, Romset& out_family) const
{
    const auto it = m_name_map.find(name);

    if (it == m_name_map.end())
    {
        return false;
    }

    const size_t index = it->second;

    out_family = m_romsets[index].romset;
    return true;
}

RomsetRegistry RomsetRegistry::CreateWithDefaultHashes()
{
    RomsetRegistry registry;

    for (auto& def : GetStandardRomsetDefinitions())
    {
        registry.AddRomset(def);
    }

    return registry;
}
