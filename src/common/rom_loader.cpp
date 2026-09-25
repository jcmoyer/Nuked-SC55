/*
 * Copyright (C) 2024-2026 J.C. Moyer
 *
 * This file is part of Nuked-SC55.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include "rom_loader.h"

#include <cstddef>
#include <cstdio>

#include "common/term_io.h"

#include "rom.h"
#include "rom_io.h"

namespace common
{

enum class PickRomsetError
{
    // Found 0
    NoRomset = 1,
    // Found 2+
    AmbiguousRomset,
};

// Given a romset family, this function picks a specific version from that
// family based on what files are present on disk.
static PickRomsetError PickSpecificRomset(const LoaderRegistries& registries,
                                          const RomLocationSet&   mask,
                                          Romset                  family,
                                          std::string&            picked_name)
{
    // This requires a bit of care since we need to support rom overrides. If
    // we point -d to an empty directory and override all rom locations, all
    // romsets for `family` are ambiguous. We can disambiguate by excluding
    // overridden locations from consideration and ensuring that the remaining
    // locations have the same hash.
    //
    // As a practical example, consider cm300-v1.10 and cm300-v1.20. These
    // romsets differ by ROM2 only. If we only override ROM1, we can't decide
    // which set to load because there are two ROM2 candidates. If we only
    // override ROM2, the rest of the locations use the same roms, so there is
    // no ambiguity.
    //
    // We also need to support the case where a romset only partially exists on
    // disk and the romset is completed by overrides.
    std::array<SHA256_Digest, ROMLOCATION_COUNT> expected_hashes{};

    bool found_any = false;

    for (const RomsetDefinition& def : registries.romsets)
    {
        // User requested a specific family, so there's no point considering
        // others.
        if (def.romset != family)
        {
            continue;
        }

        // We should only consider a member of this family if all the
        // non-overriden roms exist on disk.
        bool skip = false;

        for (RomLocation location : def.GetValidLocations())
        {
            if (mask[(size_t)location] && !registries.hashes.Contains(def.GetHash(location)))
            {
                skip = true;
            }
        }

        if (skip)
        {
            continue;
        }

        if (!found_any)
        {
            for (RomLocation location : def.GetValidLocations())
            {
                if (mask[(size_t)location])
                {
                    expected_hashes[(size_t)location] = def.GetHash(location);
                }
            }
            picked_name = def.name;
            found_any   = true;
        }
        else
        {
            for (RomLocation location : def.GetValidLocations())
            {
                if (mask[(size_t)location] && def.GetHash(location) != expected_hashes[(size_t)location])
                {
                    return PickRomsetError::AmbiguousRomset;
                }
            }
        }
    }

    if (!found_any)
    {
        return PickRomsetError::NoRomset;
    }

    return PickRomsetError{};
}

// size of largest loadable rom (waverom expansion)
constexpr uintmax_t MAX_ROM_FILESIZE = (uintmax_t)0x800000;

static bool FilterToRomSize(const std::filesystem::directory_entry& ent)
{
    return ent.file_size() <= MAX_ROM_FILESIZE;
}

const char* ToCString(LoadRomsetError error)
{
    switch (error)
    {
    case LoadRomsetError::InvalidRomsetName:
        return "Invalid romset name";
    case LoadRomsetError::HashFilesIoFailure:
        return "Failed to hash files";
    case LoadRomsetError::NoCompleteRomsets:
        return "No complete romsets";
    case LoadRomsetError::IncompleteRomset:
        return "Requested romset is incomplete";
    case LoadRomsetError::RomLoadFailed:
        return "Failed to load roms";
    case LoadRomsetError::RequestedFamilyAmbiguous:
        return "Ambiguous romset family";
    case LoadRomsetError::InvalidLoader:
        return "Invalid loader";
    case LoadRomsetError::RequestedFamilyNotFound:
        return "Requested romset not found";
    }

    if (error == LoadRomsetError{})
    {
        return "No error";
    }
    else
    {
        return "Unknown error";
    }
}

void LoadRomsetResult::Purge()
{
    registries.hashes.Purge();
}

// Load roms like upstream. We only accept family names because upstream does
// not distinguish between romset versions.
LoadRomsetError LoadRomsetLegacy(const std::filesystem::path& rom_directory,
                                 std::string_view             requested_romset,
                                 RomsetInfo&                  info,
                                 const RomLocationSet&        rom_mask,
                                 LoadRomsetResult&            result)
{
    const bool is_romset_given = requested_romset.size() > 0;
    const bool is_name_valid   = ParseRomsetName(requested_romset, result.romset);

    if (is_romset_given && is_name_valid)
    {
        // we were explicitly given a valid name; result.romset contains the
        // family
    }
    else if (is_romset_given && !is_name_valid)
    {
        // user asked for an invalid name
        return LoadRomsetError::InvalidRomsetName;
    }
    else if (!is_romset_given)
    {
        // upstream defaults to mk2
        result.romset = Romset::MK2;
    }

    result.picked_name = ParsableRomsetName(result.romset);

    SetRomsetFilenames(info, rom_directory, result.romset, rom_mask);

    if (!IsCompleteRomset(info, result.romset, &result.completion))
    {
        return LoadRomsetError::IncompleteRomset;
    }

    if (!LoadRomset(info, &result.loaded))
    {
        return LoadRomsetError::RomLoadFailed;
    }

    return LoadRomsetError{};
}

LoadRomsetError LoadRomsetHashing(const std::filesystem::path& rom_directory,
                                  std::string_view             requested_romset,
                                  RomsetInfo&                  info,
                                  const RomLocationSet&        rom_mask,
                                  LoadRomsetResult&            result)
{
    const bool is_romset_given  = requested_romset.size() > 0;
    const bool is_romset_family = ParseRomsetName(requested_romset, result.romset);

    // TODO: frontend needs to be responsible for initializing the registries
    // eventually as we want to support user-defined romsets
    if (!HashDirectoryFiles(rom_directory, HashDirectoryKind::Recursive, result.registries.hashes, FilterToRomSize))
    {
        return LoadRomsetError::HashFilesIoFailure;
    }

    result.registries.romsets = RomsetRegistry::CreateWithDefaultHashes();

    if (is_romset_given && is_romset_family)
    {
        // we were given a family, so we need to pick a version that exists on disk
        std::string picked_name;

        const PickRomsetError err = PickSpecificRomset(result.registries, rom_mask, result.romset, picked_name);

        switch (err)
        {
        case PickRomsetError::NoRomset:
            return LoadRomsetError::RequestedFamilyNotFound;
        case PickRomsetError::AmbiguousRomset:
            return LoadRomsetError::RequestedFamilyAmbiguous;
        }

        result.picked_name = std::move(picked_name);

        // ignored return: name valid by PickSpecificRomset, completion status
        // cannot be determined at this point because `rom_mask` may be a
        // subset of the romset
        (void)GetRomsetInfo(result.registries.romsets, result.picked_name, result.registries.hashes, rom_mask, info);
    }
    else if (is_romset_given && !is_romset_family)
    {
        // we were given a specific name; try to convert it to family name
        if (!result.registries.romsets.GetRomsetFamily(requested_romset, result.romset))
        {
            return LoadRomsetError::InvalidRomsetName;
        }

        // ignored return: name valid by GetRomsetFamily, completion status
        // cannot be determined at this point because `rom_mask` may be a
        // subset of the romset
        (void)GetRomsetInfo(result.registries.romsets, requested_romset, result.registries.hashes, rom_mask, info);

        result.picked_name = requested_romset;
    }
    else if (!is_romset_given)
    {
        // no romset given; we will load whatever we find
        StringVector romset_names;

        GetCompleteRomsetNames(result.registries.romsets, result.registries.hashes, rom_mask, romset_names);

        if (romset_names.size())
        {
            // Use the first returned name. The loaded romset will be
            // essentially random if there is more than one in the rom
            // directory.
            //
            // TODO: We may want to make this deterministic or an error in the
            // future.
            result.picked_name = romset_names.front();

            // ignored return: name valid by GetCompleteRomsetNames, completion
            // status cannot be determined at this point because `rom_mask` may
            // be a subset of the romset
            (void)GetRomsetInfo(
                result.registries.romsets, result.picked_name, result.registries.hashes, rom_mask, info);
            (void)result.registries.romsets.GetRomsetFamily(result.picked_name, result.romset);
        }
        else
        {
            return LoadRomsetError::NoCompleteRomsets;
        }
    }

    if (!IsCompleteRomset(info, result.romset, &result.completion))
    {
        return LoadRomsetError::IncompleteRomset;
    }

    if (!LoadRomset(info, &result.loaded))
    {
        return LoadRomsetError::RomLoadFailed;
    }

    return LoadRomsetError{};
}

LoadRomsetError LoadRomset(const std::filesystem::path& rom_directory,
                           std::string_view             requested_romset,
                           RomLoader                    loader,
                           const RomOverrides&          overrides,
                           LoadRomsetResult&            result)
{
    RomsetInfo&    romset_info = result.romset_info;
    RomLocationSet rom_mask    = ROMLOCATION_ALL;

    // for roms the user has overridden, we do not want the loader to
    // even consider the roms for those locations
    for (size_t location = 0; location < ROMLOCATION_COUNT; ++location)
    {
        if (!overrides[location].empty())
        {
            rom_mask[location]              = false;
            romset_info.rom_paths[location] = overrides[location];
        }
    }

    result.rom_mask       = rom_mask;
    result.overrides      = overrides;
    result.requested_name = requested_romset;

    switch (loader)
    {
    case RomLoader::Legacy:
        return LoadRomsetLegacy(rom_directory, requested_romset, romset_info, rom_mask, result);
    case RomLoader::Hashing:
        return LoadRomsetHashing(rom_directory, requested_romset, romset_info, rom_mask, result);
    }

    return LoadRomsetError::InvalidLoader;
}

void PrintRomsets()
{
    RomsetRegistry romsets = RomsetRegistry::CreateWithDefaultHashes();
    StringVector   specific_names;

    Printf("Accepted romset names:\n");
    for (size_t i = 0; i < ROMSET_COUNT; ++i)
    {
        Romset romset = (Romset)i;
        Printf("  %s\n", ParsableRomsetName(romset));

        romsets.GetNamesForFamily(romset, specific_names);
        for (const auto& spec_name : specific_names)
        {
            Printf("      %s\n", spec_name.c_str());
        }
    }
    Printf("\n");
}

void PrintRomsetCompletion(const RomCompletionStatusSet& completion, const RomsetInfo& info)
{
    for (size_t i = 0; i < ROMLOCATION_COUNT; ++i)
    {
        if (completion[i] != RomCompletionStatus::Unused)
        {
            Printf("  * %7s: %-12s", ToCString(completion[i]), ToCString((RomLocation)i));

            if (completion[i] == RomCompletionStatus::Present)
            {
                Printf("%s\n", info.rom_paths[i].generic_string().c_str());
            }
            else
            {
                Printf("\n");
            }
        }
    }
}

void PrintLoadRomsetDiagnostics(LoadRomsetError error, const LoadRomsetResult& result)
{
    switch (error)
    {
    case LoadRomsetError::HashFilesIoFailure:
        Printf("error: %s\n", ToCString(error));
        Printf("Make sure you have permission to read the files in the rom directory and\n");
        Printf("that the drive is working.\n");
        break;
    case LoadRomsetError::InvalidRomsetName:
        Printf("error: %s\n", ToCString(error));
        PrintRomsets();
        break;
    case LoadRomsetError::RequestedFamilyNotFound:
    case LoadRomsetError::NoCompleteRomsets: {
        if (error == LoadRomsetError::RequestedFamilyNotFound)
        {
            Printf("No complete romsets found for %s.\n", result.requested_name.c_str());
        }
        else
        {
            Printf("No complete romsets found.\n");
        }

        StringVector partial_names;
        GetPartialRomsetNames(result.registries.romsets, result.registries.hashes, ROMLOCATION_ALL, partial_names);

        for (const auto& name : partial_names)
        {
            Romset                 family;
            RomsetInfo             info;
            RomCompletionStatusSet completion;

            (void)result.registries.romsets.GetRomsetFamily(name, family);
            (void)GetRomsetInfo(result.registries.romsets, name, result.registries.hashes, ROMLOCATION_ALL, info);

            bool complete = IsCompleteRomset(info, family, &completion);

            const char* format;
            if (complete)
            {
                format = "Romset %s (%s) complete:\n";
            }
            else
            {
                format = "Romset %s (%s) partially complete:\n";
            }

            Printf(format, name.c_str(), ParsableRomsetName(family));
            PrintRomsetCompletion(completion, info);
        }

        break;
    }
    case LoadRomsetError::IncompleteRomset:
        Printf("Romset %s (%s) is incomplete:\n", result.requested_name.c_str(), ParsableRomsetName(result.romset));
        PrintRomsetCompletion(result.completion, result.romset_info);
        break;
    case LoadRomsetError::RomLoadFailed:
        Printf("Failed to load some roms:\n");
        for (size_t i = 0; i < ROMLOCATION_COUNT; ++i)
        {
            if (result.loaded[i] != RomLoadStatus::Unused)
            {
                Printf("  * %s: %-12s %s\n",
                       ToCString(result.loaded[i]),
                       ToCString((RomLocation)i),
                       result.romset_info.rom_paths[i].generic_string().c_str());
            }
        }
        break;
    case LoadRomsetError::RequestedFamilyAmbiguous: {
        Printf("Requested romset %s is ambiguous:\n", result.requested_name.c_str());

        for (const RomsetDefinition& def : result.registries.romsets)
        {
            // Currently, ambiguity only happens when the user writes a family name but there are multiple romsets
            // under that family in the rom directory. e.g. "--romset jv880" with the directory containing both
            // "jv880-v1.0.0" and "jv880-v1.0.1"
            //
            // This may change in the future if we decide it is ambiguous to leave --romset unspecified when
            // multiple romsets of any kind are present.
            if (def.romset != result.romset)
            {
                continue;
            }

            RomCompletionStatusSet completion;

            if (GetDefinitionCompletion(def, result.registries.hashes, result.rom_mask, completion))
            {
                Printf("Found %s:\n", def.name);

                for (RomLocation loc : def.GetValidLocations())
                {
                    if (result.rom_mask[(size_t)loc] && completion[(size_t)loc] == RomCompletionStatus::Present)
                    {
                        Printf("  * %7s: %-12s %s\n",
                               ToCString(completion[(size_t)loc]),
                               ToCString(loc),
                               result.registries.hashes.GetFile(def.GetHash(loc))->path.generic_string().c_str());
                    }
                    else if (!result.rom_mask[(size_t)loc])
                    {
                        Printf("  * Present: %-12s %s (override)\n",
                               ToCString(loc),
                               result.overrides[(size_t)loc].generic_string().c_str());
                    }
                }
            }
        }
        break;
    }
    case LoadRomsetError::InvalidLoader:
        Printf("Invalid rom loader - this is a developer mistake!\n");
        break;
    }

    if (error == LoadRomsetError{})
    {
        if (result.picked_name.size())
        {
            Printf("Using %s romset %s:\n", ParsableRomsetName(result.romset), result.picked_name.c_str());
        }
        else
        {
            Printf("Using %s romset:\n", ParsableRomsetName(result.romset));
        }
        for (size_t i = 0; i < ROMLOCATION_COUNT; ++i)
        {
            if (result.loaded[i] == RomLoadStatus::Loaded)
            {
                Printf("  * %-12s %s\n",
                       ToCString((RomLocation)i),
                       result.romset_info.rom_paths[i].generic_string().c_str());
            }
        }
    }
}

} // namespace common
