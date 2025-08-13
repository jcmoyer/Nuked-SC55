#include "application.h"

#include "common/command_line.h"
#include "common/gain.h"

const char* ParseErrorStr(CliParseError err)
{
    switch (err)
    {
    case CliParseError::Success:
        return "Success";
    case CliParseError::InstancesInvalid:
        return "Instances couldn't be parsed (should be 1-16)";
    case CliParseError::InstancesOutOfRange:
        return "Instances out of range (should be 1-16)";
    case CliParseError::UnexpectedEnd:
        return "Expected another argument";
    case CliParseError::BufferSizeInvalid:
        return "Buffer size invalid";
    case CliParseError::BufferCountInvalid:
        return "Buffer count invalid (should be greater than zero)";
    case CliParseError::UnknownArgument:
        return "Unknown argument";
    case CliParseError::RomDirectoryNotFound:
        return "Rom directory doesn't exist";
    case CliParseError::FormatInvalid:
        return "Output format invalid";
    case CliParseError::ASIOSampleRateOutOfRange:
        return "ASIO sample rate out of range";
    case CliParseError::ASIOChannelInvalid:
        return "ASIO channel invalid";
    case CliParseError::ResetInvalid:
        return "Reset invalid (should be none, gs, or gm)";
    case CliParseError::GainInvalid:
        return "Gain invalid (should be a number optionally ending in 'db')";
    }
    return "Unknown error";
}

CliParseError ParseCommandLine(int argc, char* argv[], CliParameters& result)
{
    common::CommandLineReader reader(argc, argv);

    while (reader.Next())
    {
        if (reader.Any("-h", "--help", "-?"))
        {
            result.help = true;
            return CliParseError::Success;
        }
        else if (reader.Any("-v", "--version"))
        {
            result.version = true;
            return CliParseError::Success;
        }
        else if (reader.Any("-p", "--port"))
        {
            if (!reader.Next())
            {
                return CliParseError::UnexpectedEnd;
            }

            result.midi_device = reader.Arg();
        }
        else if (reader.Any("-a", "--audio-device"))
        {
            if (!reader.Next())
            {
                return CliParseError::UnexpectedEnd;
            }

            result.audio_device = reader.Arg();
        }
        else if (reader.Any("-f", "--format"))
        {
            if (!reader.Next())
            {
                return CliParseError::UnexpectedEnd;
            }

            if (reader.Arg() == "s16")
            {
                result.output_format = AudioFormat::S16;
            }
            else if (reader.Arg() == "s32")
            {
                result.output_format = AudioFormat::S32;
            }
            else if (reader.Arg() == "f32")
            {
                result.output_format = AudioFormat::F32;
            }
            else
            {
                return CliParseError::FormatInvalid;
            }
        }
        else if (reader.Any("-b", "--buffer-size"))
        {
            if (!reader.Next())
            {
                return CliParseError::UnexpectedEnd;
            }

            std::string_view arg = reader.Arg();
            if (size_t colon = arg.find(':'); colon != std::string_view::npos)
            {
                auto buffer_size_sv  = arg.substr(0, colon);
                auto buffer_count_sv = arg.substr(colon + 1);

                if (!common::TryParse(buffer_size_sv, result.buffer_size))
                {
                    return CliParseError::BufferSizeInvalid;
                }

                if (!common::TryParse(buffer_count_sv, result.buffer_count))
                {
                    return CliParseError::BufferCountInvalid;
                }

                if (result.buffer_count == 0)
                {
                    return CliParseError::BufferCountInvalid;
                }
            }
            else if (!reader.TryParse(result.buffer_size))
            {
                return CliParseError::BufferSizeInvalid;
            }
        }
        else if (reader.Any("-r", "--reset"))
        {
            if (!reader.Next())
            {
                return CliParseError::UnexpectedEnd;
            }

            if (reader.Arg() == "gm")
            {
                result.reset = EMU_SystemReset::GM_RESET;
            }
            else if (reader.Arg() == "gs")
            {
                result.reset = EMU_SystemReset::GS_RESET;
            }
            else if (reader.Arg() == "none")
            {
                result.reset = EMU_SystemReset::NONE;
            }
            else
            {
                return CliParseError::ResetInvalid;
            }
        }
        else if (reader.Any("-n", "--instances"))
        {
            if (!reader.Next())
            {
                return CliParseError::UnexpectedEnd;
            }

            if (!reader.TryParse(result.instances))
            {
                return CliParseError::InstancesInvalid;
            }

            if (result.instances < 1 || result.instances > 16)
            {
                return CliParseError::InstancesOutOfRange;
            }
        }
        else if (reader.Any("--no-lcd"))
        {
            result.no_lcd = true;
        }
        else if (reader.Any("--disable-oversampling"))
        {
            result.disable_oversampling = true;
        }
        else if (reader.Any("--gain"))
        {
            if (!reader.Next())
            {
                return CliParseError::UnexpectedEnd;
            }

            if (common::ParseGain(reader.Arg(), result.gain) != common::ParseGainResult{})
            {
                return CliParseError::GainInvalid;
            }
        }
        else if (reader.Any("-d", "--rom-directory"))
        {
            if (!reader.Next())
            {
                return CliParseError::UnexpectedEnd;
            }

            result.rom_directory = reader.Arg();

            if (!std::filesystem::exists(*result.rom_directory))
            {
                return CliParseError::RomDirectoryNotFound;
            }
        }
        else if (reader.Any("--nvram"))
        {
            if (!reader.Next())
            {
                return CliParseError::UnexpectedEnd;
            }

            result.nvram_filename = reader.Arg();
        }
        else if (reader.Any("--romset"))
        {
            if (!reader.Next())
            {
                return CliParseError::UnexpectedEnd;
            }

            result.romset_name = reader.Arg();
        }
        else if (reader.Any("--legacy-romset-detection"))
        {
            result.legacy_romset_detection = true;
        }
        else if (reader.Any("--override-rom1"))
        {
            if (!reader.Next())
            {
                return CliParseError::UnexpectedEnd;
            }

            result.adv.rom_overrides[(size_t)RomLocation::ROM1] = reader.Arg();
        }
        else if (reader.Any("--override-rom2"))
        {
            if (!reader.Next())
            {
                return CliParseError::UnexpectedEnd;
            }

            result.adv.rom_overrides[(size_t)RomLocation::ROM2] = reader.Arg();
        }
        else if (reader.Any("--override-smrom"))
        {
            if (!reader.Next())
            {
                return CliParseError::UnexpectedEnd;
            }

            result.adv.rom_overrides[(size_t)RomLocation::SMROM] = reader.Arg();
        }
        else if (reader.Any("--override-waverom1"))
        {
            if (!reader.Next())
            {
                return CliParseError::UnexpectedEnd;
            }

            result.adv.rom_overrides[(size_t)RomLocation::WAVEROM1] = reader.Arg();
        }
        else if (reader.Any("--override-waverom2"))
        {
            if (!reader.Next())
            {
                return CliParseError::UnexpectedEnd;
            }

            result.adv.rom_overrides[(size_t)RomLocation::WAVEROM2] = reader.Arg();
        }
        else if (reader.Any("--override-waverom3"))
        {
            if (!reader.Next())
            {
                return CliParseError::UnexpectedEnd;
            }

            result.adv.rom_overrides[(size_t)RomLocation::WAVEROM3] = reader.Arg();
        }
        else if (reader.Any("--override-waverom-card"))
        {
            if (!reader.Next())
            {
                return CliParseError::UnexpectedEnd;
            }

            result.adv.rom_overrides[(size_t)RomLocation::WAVEROM_CARD] = reader.Arg();
        }
        else if (reader.Any("--override-waverom-exp"))
        {
            if (!reader.Next())
            {
                return CliParseError::UnexpectedEnd;
            }

            result.adv.rom_overrides[(size_t)RomLocation::WAVEROM_EXP] = reader.Arg();
        }
#if NUKED_ENABLE_ASIO
        else if (reader.Any("--asio-sample-rate"))
        {
            if (!reader.Next())
            {
                return CliParseError::UnexpectedEnd;
            }

            uint32_t asio_sample_rate = 0;
            if (!reader.TryParse(asio_sample_rate))
            {
                return CliParseError::ASIOSampleRateOutOfRange;
            }

            result.asio_sample_rate = asio_sample_rate;
        }
        else if (reader.Any("--asio-left-channel"))
        {
            if (!reader.Next())
            {
                return CliParseError::UnexpectedEnd;
            }

            result.asio_left_channel = reader.Arg();
        }
        else if (reader.Any("--asio-right-channel"))
        {
            if (!reader.Next())
            {
                return CliParseError::UnexpectedEnd;
            }

            result.asio_right_channel = reader.Arg();
        }
#endif
        else
        {
            return CliParseError::UnknownArgument;
        }
    }

    return CliParseError::Success;
}
