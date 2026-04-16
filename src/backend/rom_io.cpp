#include "rom_io.h"

#include <filesystem>
#include <utility>

#include "file_hashing.h"
#include "file_io.h"

const char* legacy_rom_names[(size_t)ROMSET_COUNT][ROMLOCATION_COUNT] = {
    // MK2
    {
        // ROM1
        "rom1.bin",
        // ROM2
        "rom2.bin",
        // SMROM
        "rom_sm.bin",
        // WAVEROM1
        "waverom1.bin",
        // WAVEROM2
        "waverom2.bin",
        // WAVEROM3
        "",
        // WAVEROM_CARD
        "",
        // WAVEROM_EXP
        "",
    },
    // ST
    {
        // ROM1
        "rom1.bin",
        // ROM2
        "rom2_st.bin",
        // SMROM
        "rom_sm.bin",
        // WAVEROM1
        "waverom1.bin",
        // WAVEROM2
        "waverom2.bin",
        // WAVEROM3
        "",
        // WAVEROM_CARD
        "",
        // WAVEROM_EXP
        "",
    },
    // MK1
    {
        // ROM1
        "sc55_rom1.bin",
        // ROM2
        "sc55_rom2.bin",
        // SMROM
        "",
        // WAVEROM1
        "sc55_waverom1.bin",
        // WAVEROM2
        "sc55_waverom2.bin",
        // WAVEROM3
        "sc55_waverom3.bin",
        // WAVEROM_CARD
        "",
        // WAVEROM_EXP
        "",
    },
    // CM300
    {
        // ROM1
        "cm300_rom1.bin",
        // ROM2
        "cm300_rom2.bin",
        // SMROM
        "",
        // WAVEROM1
        "cm300_waverom1.bin",
        // WAVEROM2
        "cm300_waverom2.bin",
        // WAVEROM3
        "cm300_waverom3.bin",
        // WAVEROM_CARD
        "",
        // WAVEROM_EXP
        "",
    },
    // JV880
    {
        // ROM1
        "jv880_rom1.bin",
        // ROM2
        "jv880_rom2.bin",
        // SMROM
        "",
        // WAVEROM1
        "jv880_waverom1.bin",
        // WAVEROM2
        "jv880_waverom2.bin",
        // WAVEROM3
        "",
        // WAVEROM_CARD
        "jv880_waverom_pcmcard.bin",
        // WAVEROM_EXP
        "jv880_waverom_expansion.bin",
    },
    // SCB55
    {
        // ROM1
        "scb55_rom1.bin",
        // ROM2
        "scb55_rom2.bin",
        // SMROM
        "",
        // WAVEROM1
        "scb55_waverom1.bin",
        // WAVEROM2
        "",
        // WAVEROM3 - this file being named waverom2 is intentional
        "scb55_waverom2.bin",
        // WAVEROM_CARD
        "",
        // WAVEROM_EXP
        "",
    },
    // RLP3237
    {
        // ROM1
        "rlp3237_rom1.bin",
        // ROM2
        "rlp3237_rom2.bin",
        // SMROM
        "",
        // WAVEROM1
        "rlp3237_waverom1.bin",
        // WAVEROM2
        "",
        // WAVEROM3
        "",
        // WAVEROM_CARD
        "",
        // WAVEROM_EXP
        "",
    },
    // SC155
    {
        // ROM1
        "sc155_rom1.bin",
        // ROM2
        "sc155_rom2.bin",
        // SMROM
        "",
        // WAVEROM1
        "sc155_waverom1.bin",
        // WAVEROM2
        "sc155_waverom2.bin",
        // WAVEROM3
        "sc155_waverom3.bin",
        // WAVEROM_CARD
        "",
        // WAVEROM_EXP
        "",
    },
    // SC155MK2
    {
        // ROM1
        "rom1.bin",
        // ROM2
        "rom2.bin",
        // SMROM
        "rom_sm.bin",
        // WAVEROM1
        "waverom1.bin",
        // WAVEROM2
        "waverom2.bin",
        // WAVEROM3
        "",
        // WAVEROM_CARD
        "",
        // WAVEROM_EXP
        "",
    },
};

void unscramble(const uint8_t *src, uint8_t *dst, int len)
{
    for (int i = 0; i < len; i++)
    {
        int address = i & ~0xfffff;
        static const int aa[] = {
            2, 0, 3, 4, 1, 9, 13, 10, 18, 17, 6, 15, 11, 16, 8, 5, 12, 7, 14, 19
        };
        for (int j = 0; j < 20; j++)
        {
            if (i & (1 << j))
                address |= 1<<aa[j];
        }
        uint8_t srcdata = src[address];
        uint8_t data = 0;
        static const int dd[] = {
            2, 0, 4, 5, 7, 6, 3, 1
        };
        for (int j = 0; j < 8; j++)
        {
            if (srcdata & (1 << dd[j]))
                data |= 1<<j;
        }
        dst[i] = data;
    }
}

bool IsCompleteRomset(const RomsetInfo& info, Romset romset, RomCompletionStatusSet* status)
{
    if (status)
    {
        status->fill(RomCompletionStatus::Unused);
    }

    bool is_complete = true;

    for (size_t i = 0; i < ROMLOCATION_COUNT; ++i)
    {
        const RomLocation location = (RomLocation)i;
        if (IsRequiredRom(romset, location) && info.HasRom(location))
        {
            (*status)[i] = RomCompletionStatus::Present;
        }
        else if (IsRequiredRom(romset, location) && !info.HasRom(location))
        {
            (*status)[i] = RomCompletionStatus::Missing;
            is_complete  = false;
        }
    }

    return is_complete;
}

bool GetDefinitionCompletion(const RomsetDefinition&   def,
                             const HashedFileRegistry& hashed_files,
                             const RomLocationSet&     location_mask,
                             RomCompletionStatusSet&   completion)
{
    completion.fill(RomCompletionStatus::Unused);

    bool is_complete = true;

    for (RomLocation rom : def.GetValidLocations())
    {
        if (!location_mask[(size_t)rom])
        {
            continue;
        }

        if (hashed_files.Contains(def.GetHash(rom)))
        {
            completion[(size_t)rom] = RomCompletionStatus::Present;
        }
        else
        {
            completion[(size_t)rom] = RomCompletionStatus::Missing;
            is_complete             = false;
        }
    }

    return is_complete;
}

size_t CountPresent(const RomCompletionStatusSet& status)
{
    size_t count = 0;
    for (auto s : status)
    {
        if (s == RomCompletionStatus::Present)
        {
            ++count;
        }
    }
    return count;
}

void SetRomsetFilenames(RomsetInfo&                  romset_info,
                        const std::filesystem::path& base_path,
                        Romset                       romset,
                        const RomLocationSet&        location_mask)
{
    for (size_t rom = 0; rom < ROMLOCATION_COUNT; ++rom)
    {
        if (legacy_rom_names[(size_t)romset][rom][0] == '\0')
        {
            continue;
        }

        if (!location_mask[rom])
        {
            continue;
        }

        romset_info.rom_paths[rom] = base_path / legacy_rom_names[(size_t)romset][rom];
    }
}

void RomsetInfo::PurgeRomData()
{
    for (auto& vec : rom_data)
    {
        vec = {};
    }
}

bool RomsetInfo::HasRom(RomLocation location) const
{
    return !(rom_paths[(size_t)location].empty() && rom_data[(size_t)location].empty());
}

bool LoadRomset(RomsetInfo& info, RomLoadStatusSet* loaded)
{
    bool all_loaded = true;

    // We cannot unscramble in-place.
    std::vector<uint8_t> on_demand_buffer;

    for (size_t i = 0; i < ROMLOCATION_COUNT; ++i)
    {
        const RomLocation location = (RomLocation)i;

        if (info.rom_paths[i].empty() && info.rom_data[i].empty())
        {
            if (loaded)
            {
                (*loaded)[i] = RomLoadStatus::Unused;
            }
            continue;
        }
        else if (!info.rom_paths[i].empty() && info.rom_data[i].empty())
        {
            if (!FIO_ReadAllBytes(info.rom_paths[i], on_demand_buffer))
            {
                all_loaded = false;
                if (loaded)
                {
                    (*loaded)[i] = RomLoadStatus::Failed;
                }
                continue;
            }

            if (IsWaverom(location))
            {
                info.rom_data[i].resize(on_demand_buffer.size());
                unscramble(on_demand_buffer.data(), info.rom_data[i].data(), (int)on_demand_buffer.size());
            }
            else
            {
                info.rom_data[i] = std::move(on_demand_buffer);
                on_demand_buffer = {};
            }

            if (loaded)
            {
                (*loaded)[i] = RomLoadStatus::Loaded;
            }
        }
        else if (!info.rom_data[i].empty())
        {
            if (IsWaverom(location))
            {
                on_demand_buffer.resize(info.rom_data[i].size());
                unscramble(info.rom_data[i].data(), on_demand_buffer.data(), (int)on_demand_buffer.size());
                std::swap(info.rom_data[i], on_demand_buffer);
            }

            if (loaded)
            {
                (*loaded)[i] = RomLoadStatus::Loaded;
            }
        }
    }

    return all_loaded;
}

const char* ToCString(RomLoadStatus status)
{
    switch (status)
    {
    case RomLoadStatus::Loaded:
        return "Loaded";
    case RomLoadStatus::Failed:
        return "Failed";
    case RomLoadStatus::Unused:
        return "Unused";
    }
    return "Unknown status";
}

const char* ToCString(RomCompletionStatus status)
{
    switch (status)
    {
    case RomCompletionStatus::Present:
        return "Present";
    case RomCompletionStatus::Missing:
        return "Missing";
    case RomCompletionStatus::Unused:
        return "Unused";
    }
    return "Unknown status";
}

void GetCompleteRomsetNames(const RomsetRegistry&     romsets,
                            const HashedFileRegistry& hashed_files,
                            const RomLocationSet&     location_mask,
                            StringVector&             out_names)
{
    out_names.clear();
    for (const auto& romset : romsets)
    {
        bool is_complete = true;
        for (RomLocation location : romset.GetValidLocations())
        {
            if (location_mask[(size_t)location] && !hashed_files.Contains(romset.GetHash(location)))
            {
                is_complete = false;
                break;
            }
        }
        if (is_complete)
        {
            out_names.push_back(romset.name);
        }
    }
}

void GetPartialRomsetNames(const RomsetRegistry&     romsets,
                           const HashedFileRegistry& hashed_files,
                           const RomLocationSet&     location_mask,
                           StringVector&             out_names)
{
    out_names.clear();
    for (const auto& romset : romsets)
    {
        bool is_partial = false;
        for (RomLocation location : romset.GetValidLocations())
        {
            if (location_mask[(size_t)location] && hashed_files.Contains(romset.GetHash(location)))
            {
                is_partial = true;
                break;
            }
        }
        if (is_partial)
        {
            out_names.push_back(romset.name);
        }
    }
}

bool ContainsRomsetFiles(const RomsetRegistry&     romsets,
                         std::string_view          name,
                         const HashedFileRegistry& hashed_files,
                         const RomLocationSet&     location_mask)
{
    const RomsetDefinition* def = romsets.GetDefinition(name);

    if (!def)
    {
        return false;
    }

    bool is_complete = true;

    for (RomLocation location : def->GetValidLocations())
    {
        if (location_mask[(size_t)location] && !hashed_files.Contains(def->GetHash(location)))
        {
            is_complete = false;
            break;
        }
    }

    return is_complete;
}

bool GetRomsetInfo(const RomsetRegistry&     romsets,
                   std::string_view          name,
                   const HashedFileRegistry& hashed_files,
                   const RomLocationSet&     location_mask,
                   RomsetInfo&               out_info)
{
    const RomsetDefinition* def = romsets.GetDefinition(name);

    if (!def)
    {
        return false;
    }

    for (RomLocation location : def->GetValidLocations())
    {
        const HashedFile* hf = hashed_files.GetFile(def->GetHash(location));

        if (!hf)
        {
            return false;
        }

        if (location_mask[(size_t)location])
        {
            out_info.rom_paths[(size_t)location] = hf->path;
            // TODO: This is a large buffer copy. In practice this probably doesn't matter because we should only have
            // ~5 roms on average and most of them will be small. It is difficult to avoid this for two reasons: 1) we
            // need to unscramble waveroms which needs a second buffer allocation anyways, and 2) RomsetInfo is also
            // acting as storage for roms which will likely be shared between emulator instances in the future. We will
            // not be able to point into the file registry when sharing is enabled, because again, we need to
            // unscramble waveroms. The final design should probably allocate one contiguous buffer to hold all roms
            // for better locality. Then we copy all of the roms into that buffer, unscrambling the waveroms at that
            // point.
            out_info.rom_data[(size_t)location] = hf->data;
        }
    }

    return true;
}
