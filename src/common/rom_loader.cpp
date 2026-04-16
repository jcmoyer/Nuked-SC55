#include "rom_loader.h"

#include <cstddef>
#include <cstdio>

#include "rom.h"
#include "rom_io.h"

namespace common
{

const char* ToCString(LoadRomsetError error)
{
    switch (error)
    {
    case LoadRomsetError::InvalidRomsetName:
        return "Invalid romset name";
    case LoadRomsetError::DetectionFailed:
        return "Failed to detect romsets";
    case LoadRomsetError::NoCompleteRomsets:
        return "No complete romsets";
    case LoadRomsetError::IncompleteRomset:
        return "Requested romset is incomplete";
    case LoadRomsetError::RomLoadFailed:
        return "Failed to load roms";
    case LoadRomsetError::AmbiguousRomset:
        return "Ambiguous romset";
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

LoadRomsetError LoadRomset(const std::filesystem::path& rom_directory,
                           std::string_view             desired_romset,
                           RomLoader                    loader,
                           const RomOverrides&          overrides,
                           LoadRomsetResult&            result)
{
    RomsetInfo&    romset_info = result.romset_info;
    RomLocationSet desired     = ROMLOCATION_ALL;

    // exclude roms with overrides - it doesn't matter if they're not on disk because the user has a different file to
    // put there
    for (size_t location = 0; location < ROMLOCATION_COUNT; ++location)
    {
        if (!overrides[location].empty())
        {
            desired[location]               = false;
            romset_info.rom_paths[location] = overrides[location];
        }
    }

    const bool is_romset_given  = desired_romset.size() > 0;
    const bool is_romset_family = ParseRomsetName(desired_romset, result.romset);

    switch (loader)
    {
    case RomLoader::Legacy:
        if (is_romset_given && is_romset_family)
        {
            // we were explicitly given a valid name
        }
        else if (is_romset_given && !is_romset_family)
        {
            // user asked for an invalid name
            return LoadRomsetError::InvalidRomsetName;
        }
        else if (!is_romset_given)
        {
            // upstream defaults to mk2
            result.romset = Romset::MK2;
        }

        SetRomsetFilenames(romset_info, rom_directory, result.romset, desired);

        break;

    case RomLoader::Hashing: {
        // size of largest loadable rom (waverom expansion)
        constexpr uintmax_t MAX_ROM_FILESIZE = (uintmax_t)0x800000;

        constexpr auto FILTER_FILESIZE = [](const std::filesystem::directory_entry& ent) {
            return ent.file_size() <= MAX_ROM_FILESIZE;
        };

        if (!HashDirectoryFiles(rom_directory, result.registries.hashes, FILTER_FILESIZE))
        {
            return LoadRomsetError::DetectionFailed;
        }
        result.registries.romsets = RomsetRegistry::CreateWithDefaultHashes();

        StringVector romset_names;

        if (is_romset_given && is_romset_family)
        {
            // we were given a family, so we need to pick a specific version from that family
            GetCompleteRomsetNames(result.registries.romsets, result.registries.hashes, desired, romset_names);
            if (romset_names.size() == 0)
            {
                return LoadRomsetError::NoCompleteRomsets;
            }

            bool        did_find_romset = false;
            std::string picked_name;

            for (const auto& name : romset_names)
            {
                Romset rs_family;
                // ignored return: this function cannot fail because the name comes from GetCompleteRomsetNames, so it
                // must be in the registry
                (void)result.registries.romsets.GetRomsetFamily(name, rs_family);
                if (rs_family == result.romset)
                {
                    if (did_find_romset)
                    {
                        return LoadRomsetError::AmbiguousRomset;
                    }
                    did_find_romset = true;
                    picked_name     = name;
                }
            }

            if (!did_find_romset)
            {
                return LoadRomsetError::NoCompleteRomsets;
            }

            if (!GetRomsetInfo(result.registries.romsets, picked_name, result.registries.hashes, desired, romset_info))
            {
                return LoadRomsetError::InvalidRomsetName;
            }

            result.picked_name = picked_name;
        }
        else if (is_romset_given && !is_romset_family)
        {
            // we were given a specific name
            if (!GetRomsetInfo(
                    result.registries.romsets, desired_romset, result.registries.hashes, desired, romset_info))
            {
                return LoadRomsetError::InvalidRomsetName;
            }
            // convert specific name to family name
            if (!result.registries.romsets.GetRomsetFamily(desired_romset, result.romset))
            {
                return LoadRomsetError::InvalidRomsetName;
            }

            result.picked_name = desired_romset;
        }
        else if (!is_romset_given)
        {
            GetCompleteRomsetNames(result.registries.romsets, result.registries.hashes, ROMLOCATION_ALL, romset_names);

            if (romset_names.size())
            {
                // Use the first returned name. The loaded romset will be essentially random if there is more than one
                // in the rom directory.
                // TODO: We may want to make this deterministic or an error in the future.

                result.picked_name = romset_names.front();

                // ignored returns: these names were returned by GetCompleteRomsetNames so the lookup cannot fail
                (void)GetRomsetInfo(
                    result.registries.romsets, result.picked_name, result.registries.hashes, desired, romset_info);
                (void)result.registries.romsets.GetRomsetFamily(result.picked_name, result.romset);
            }
            else
            {
                return LoadRomsetError::NoCompleteRomsets;
            }
        }
        break;
    }
    default:
        return LoadRomsetError::DetectionFailed;
    }

    if (!IsCompleteRomset(romset_info, result.romset, &result.completion))
    {
        return LoadRomsetError::IncompleteRomset;
    }

    if (!LoadRomset(romset_info, &result.loaded))
    {
        return LoadRomsetError::RomLoadFailed;
    }

    return LoadRomsetError{};
}

void PrintRomsets(FILE* output)
{
    RomsetRegistry romsets = RomsetRegistry::CreateWithDefaultHashes();
    StringVector   specific_names;

    fprintf(output, "Accepted romset names:\n");
    for (size_t i = 0; i < ROMSET_COUNT; ++i)
    {
        Romset romset = (Romset)i;
        fprintf(output, "  %s\n", ParsableRomsetName(romset));

        romsets.GetNamesForFamily(romset, specific_names);
        for (const auto& spec_name : specific_names)
        {
            // deduplicate - some romset families only have one specific romset
            // in that case we give them the same name
            if (spec_name != ParsableRomsetName(romset))
            {
                fprintf(output, "      %s\n", spec_name.c_str());
            }
        }
    }
    fprintf(output, "\n");
}

void PrintLoadRomsetDiagnostics(FILE* output, LoadRomsetError error, const LoadRomsetResult& result)
{
    switch (error)
    {
    case LoadRomsetError::DetectionFailed:
        // TODO: DetectRomsets* will print its own diagnostics
        break;
    case LoadRomsetError::InvalidRomsetName:
        fprintf(output, "error: %s\n", ToCString(error));
        PrintRomsets(output);
        break;
    case LoadRomsetError::NoCompleteRomsets: {
        fprintf(output, "No complete romsets found.\n");

        StringVector partial_names;
        GetPartialRomsetNames(result.registries.romsets, result.registries.hashes, ROMLOCATION_ALL, partial_names);

        for (const auto& name : partial_names)
        {
            Romset                 family;
            RomsetInfo             info;
            RomCompletionStatusSet completion;

            (void)result.registries.romsets.GetRomsetFamily(name, family);
            (void)GetRomsetInfo(result.registries.romsets, name, result.registries.hashes, ROMLOCATION_ALL, info);

            (void)IsCompleteRomset(info, family, &completion);

            fprintf(output, "Romset %s (%s) partially complete:\n", name.c_str(), RomsetName(family));
            for (size_t i = 0; i < ROMLOCATION_COUNT; ++i)
            {
                if (completion[i] != RomCompletionStatus::Unused)
                {
                    fprintf(output, "  * %7s: %-12s", ToCString(completion[i]), ToCString((RomLocation)i));

                    if (completion[i] == RomCompletionStatus::Present)
                    {
                        fprintf(output, "%s\n", info.rom_paths[i].generic_string().c_str());
                    }
                    else
                    {
                        fprintf(output, "\n");
                    }
                }
            }
        }

        break;
    }
    case LoadRomsetError::IncompleteRomset:
        fprintf(output, "Romset %s is incomplete:\n", RomsetName(result.romset));
        for (size_t i = 0; i < ROMLOCATION_COUNT; ++i)
        {
            if (result.completion[i] != RomCompletionStatus::Unused)
            {
                fprintf(output, "  * %7s: %-12s", ToCString(result.completion[i]), ToCString((RomLocation)i));

                if (result.completion[i] == RomCompletionStatus::Present)
                {
                    fprintf(output, "%s\n", result.romset_info.rom_paths[i].generic_string().c_str());
                }
                else
                {
                    fprintf(output, "\n");
                }
            }
        }
        break;
    case LoadRomsetError::RomLoadFailed:
        fprintf(output, "Failed to load some %s roms:\n", RomsetName(result.romset));
        for (size_t i = 0; i < ROMLOCATION_COUNT; ++i)
        {
            if (result.loaded[i] != RomLoadStatus::Unused)
            {
                fprintf(output,
                        "  * %s: %-12s %s\n",
                        ToCString(result.loaded[i]),
                        ToCString((RomLocation)i),
                        result.romset_info.rom_paths[i].generic_string().c_str());
            }
        }
        break;
    case LoadRomsetError::AmbiguousRomset: {
        fprintf(output, "Requested romset `%s` is ambiguous:\n", ParsableRomsetName(result.romset));

        StringVector names;
        for (const RomsetDefinition& def : result.registries.romsets)
        {
            // Currently, ambiguity only happens when the user writes a family name but there are multiple romsets
            // under that family in the rom directory. e.g. "--romset jv880" with the directory containing both
            // "jv880-v1.0.0" and "jv880-v1.0.1"
            //
            // This may change in the future if we decide it is ambiguous to leave --romset unspecified when multiple
            // romsets of any kind are present.
            if (def.romset != result.romset)
            {
                continue;
            }

            RomCompletionStatusSet completion;
            if (GetDefinitionCompletion(def, result.registries.hashes, ROMLOCATION_ALL, completion))
            {
                fprintf(output, "Found %s:\n", def.name);

                for (size_t i = 0; i < ROMLOCATION_COUNT; ++i)
                {
                    if (completion[i] == RomCompletionStatus::Present)
                    {
                        fprintf(output,
                                "  * %7s: %-12s %s\n",
                                ToCString(completion[i]),
                                ToCString((RomLocation)i),
                                result.registries.hashes.GetFile(def.GetHash((RomLocation)i))
                                    ->path.generic_string()
                                    .c_str());
                    }
                }
            }
        }
    }
    }

    if (error == LoadRomsetError{})
    {
        if (result.picked_name.size())
        {
            fprintf(output, "Using %s romset %s:\n", RomsetName(result.romset), result.picked_name.c_str());
        }
        else
        {
            fprintf(output, "Using %s romset:\n", RomsetName(result.romset));
        }
        for (size_t i = 0; i < ROMLOCATION_COUNT; ++i)
        {
            if (result.loaded[i] == RomLoadStatus::Loaded)
            {
                fprintf(output,
                        "  * %-12s %s\n",
                        ToCString((RomLocation)i),
                        result.romset_info.rom_paths[i].generic_string().c_str());
            }
        }
    }
}

} // namespace common
