#include "rom_loader.h"

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

LoadRomsetError LoadRomset(RomsetInfo&                  romset_info,
                           const std::filesystem::path& rom_directory,
                           std::string_view             desired_romset,
                           RomLoader                    loader,
                           const RomOverrides&          overrides,
                           LoadRomsetResult&            result)
{
    RomLocationSet desired = ROMLOCATION_ALL;

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

        if (!SetRomsetFilenames(romset_info, rom_directory, result.romset, desired))
        {
            return LoadRomsetError::DetectionFailed;
        }
        break;

    case RomLoader::Hashing: {
        HashedFileRegistry hashed_files;
        if (!HashAllFiles(rom_directory, hashed_files))
        {
            return LoadRomsetError::DetectionFailed;
        }
        RomsetHashRegistry romsets = RomsetHashRegistry::CreateWithDefaultHashes();
        StringVector       romset_names;

        if (is_romset_given && is_romset_family)
        {
            // we were given a family, so we need to pick a specific version from that family
            romsets.GetCompleteRomsetNames(hashed_files, romset_names, desired);
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
                (void)romsets.GetRomsetFamily(name, rs_family);
                if (rs_family == result.romset)
                {
                    if (did_find_romset)
                    {
                        return LoadRomsetError::AmbiguousRomset;
                    }
                    did_find_romset = true;
                    picked_name     = name;
                    break;
                }
            }

            if (!did_find_romset)
            {
                return LoadRomsetError::NoCompleteRomsets;
            }

            if (!romsets.GetRomsetInfo(hashed_files, picked_name, desired, romset_info))
            {
                return LoadRomsetError::InvalidRomsetName;
            }
        }
        else if (is_romset_given && !is_romset_family)
        {
            // we were given a specific name
            if (!romsets.GetRomsetInfo(hashed_files, desired_romset, desired, romset_info))
            {
                return LoadRomsetError::InvalidRomsetName;
            }
            // convert specific name to family name
            if (!romsets.GetRomsetFamily(desired_romset, result.romset))
            {
                return LoadRomsetError::InvalidRomsetName;
            }
        }
        else if (!is_romset_given)
        {
            romsets.GetCompleteRomsetNames(hashed_files, romset_names, ROMLOCATION_ALL);

            if (romset_names.size())
            {
                // Use the first returned name. The loaded romset will be essentially random if there is more than one
                // in the rom directory.
                // TODO: We may want to make this deterministic or an error in the future.

                // ignored returns: these names were returned by GetCompleteRomsetNames so the lookup cannot fail
                (void)romsets.GetRomsetInfo(hashed_files, romset_names.front(), desired, romset_info);
                (void)romsets.GetRomsetFamily(romset_names.front(), result.romset);
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

    // TODO: this function consults the hardcoded romset list. This is awkward and probably slower than it needs to be.
    // We should know which rom locations are necessary given only a Romset.
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
    fprintf(output, "Accepted romset names:\n");
    fprintf(output, "  ");
    for (const char* name : GetParsableRomsetNames())
    {
        fprintf(output, "%s ", name);
    }
    fprintf(output, "\n");

    RomsetHashRegistry romsets = RomsetHashRegistry::CreateWithDefaultHashes();
    StringVector       specific_names;
    romsets.GetAllRomsetNames(specific_names);

    fprintf(output, "  ");
    for (const auto& name : specific_names)
    {
        fprintf(output, "%s ", name.c_str());
    }
    fprintf(output, "\n\n");
}

void PrintLoadRomsetDiagnostics(FILE*                   output,
                                LoadRomsetError         error,
                                const LoadRomsetResult& result,
                                const RomsetInfo&       info)
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
    case LoadRomsetError::NoCompleteRomsets:
        fprintf(output, "No complete romsets found.\n");
        for (size_t rs = 0; rs < ROMSET_COUNT; ++rs)
        {
            RomCompletionStatusSet completion;

            (void)IsCompleteRomset(info, (Romset)rs, &completion);

            if (CountPresent(completion))
            {
                fprintf(output, "Romset %s partially complete:\n", RomsetName((Romset)rs));
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
        }
        break;
    case LoadRomsetError::IncompleteRomset:
        fprintf(output, "Romset %s is incomplete:\n", RomsetName(result.romset));
        for (size_t i = 0; i < ROMLOCATION_COUNT; ++i)
        {
            if (result.completion[i] != RomCompletionStatus::Unused)
            {
                fprintf(output, "  * %7s: %-12s", ToCString(result.completion[i]), ToCString((RomLocation)i));

                if (result.completion[i] == RomCompletionStatus::Present)
                {
                    fprintf(output, "%s\n", info.rom_paths[i].generic_string().c_str());
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
                        info.rom_paths[i].generic_string().c_str());
            }
        }
        break;
    case LoadRomsetError::AmbiguousRomset:
        fprintf(output, "Requested romset `%s` is ambiguous:\n", RomsetName(result.romset));
        // TODO: print candidates
        break;
    }

    if (error == LoadRomsetError{})
    {
        fprintf(output, "Using %s romset:\n", RomsetName(result.romset));
        for (size_t i = 0; i < ROMLOCATION_COUNT; ++i)
        {
            if (result.loaded[i] == RomLoadStatus::Loaded)
            {
                fprintf(
                    output, "  * %-12s %s\n", ToCString((RomLocation)i), info.rom_paths[i].generic_string().c_str());
            }
        }
    }
}

} // namespace common
