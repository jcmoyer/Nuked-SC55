#include "rom_loader.h"

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

LoadRomsetError LoadRomset(AllRomsetInfo&               romset_info,
                           const std::filesystem::path& rom_directory,
                           std::string_view             desired_romset,
                           bool                         legacy_loader,
                           const RomOverrides&          overrides,
                           LoadRomsetResult&            result)
{
    if (desired_romset.size())
    {
        if (!ParseRomsetName(desired_romset, result.romset))
        {
            return LoadRomsetError::InvalidRomsetName;
        }

        // When the user specifies a romset, we can speed up the loading process a bit.
        RomLocationSet desired{};
        desired[(size_t)result.romset] = true;

        if (legacy_loader)
        {
            if (!DetectRomsetsByFilename(rom_directory, romset_info, &desired))
            {
                return LoadRomsetError::DetectionFailed;
            }
        }
        else
        {
            if (!DetectRomsetsByHash(rom_directory, romset_info, &desired))
            {
                return LoadRomsetError::DetectionFailed;
            }
        }
    }
    else
    {
        // No user-specified romset; we'll use whatever romset we can find.
        if (legacy_loader)
        {
            if (!DetectRomsetsByFilename(rom_directory, romset_info))
            {
                return LoadRomsetError::DetectionFailed;
            }
        }
        else
        {
            if (!DetectRomsetsByHash(rom_directory, romset_info))
            {
                return LoadRomsetError::DetectionFailed;
            }
        }

        if (!PickCompleteRomset(romset_info, result.romset))
        {
            return LoadRomsetError::NoCompleteRomsets;
        }
    }

    for (size_t i = 0; i < ROMSET_COUNT; ++i)
    {
        for (size_t j = 0; j < ROMLOCATION_COUNT; ++j)
        {
            if (!overrides[j].empty())
            {
                romset_info.romsets[i].rom_paths[j] = overrides[j];
                romset_info.romsets[i].rom_data[j].clear();
            }
        }
    }

    if (!IsCompleteRomset(romset_info, result.romset, &result.completion))
    {
        return LoadRomsetError::IncompleteRomset;
    }

    if (!LoadRomset(result.romset, romset_info, &result.loaded))
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
    fprintf(output, "\n\n");
}

void PrintLoadRomsetDiagnostics(FILE*                   output,
                                LoadRomsetError         error,
                                const LoadRomsetResult& result,
                                const AllRomsetInfo&    info)
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
                            fprintf(output, "%s\n", info.romsets[rs].rom_paths[i].generic_string().c_str());
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
                    fprintf(output, "%s\n", info.romsets[(size_t)result.romset].rom_paths[i].generic_string().c_str());
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
                        info.romsets[(size_t)result.romset].rom_paths[i].generic_string().c_str());
            }
        }
        break;
    }

    if (error == LoadRomsetError{})
    {
        fprintf(output, "Using %s romset:\n", RomsetName(result.romset));
        for (size_t i = 0; i < ROMLOCATION_COUNT; ++i)
        {
            if (result.loaded[i] == RomLoadStatus::Loaded)
            {
                fprintf(output,
                        "  * %-12s %s\n",
                        ToCString((RomLocation)i),
                        info.romsets[(size_t)result.romset].rom_paths[i].generic_string().c_str());
            }
        }
    }
}

} // namespace common
