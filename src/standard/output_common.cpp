#include "output_common.h"
#include "output_asio.h"
#include "output_sdl.h"

#include "config.h"

#include "common/command_line.h"

void QueryAllOutputs(AudioOutputList& outputs)
{
    outputs.clear();

    if (!Out_SDL_QueryOutputs(outputs))
    {
        fprintf(stderr, "Failed to query SDL outputs: %s\n", SDL_GetError());
        return;
    }

#if NUKED_ENABLE_ASIO
    if (!Out_ASIO_QueryOutputs(outputs))
    {
        fprintf(stderr, "Failed to query ASIO outputs.\n");
        return;
    }
#endif
}

PickOutputResult PickOutputDevice(std::string_view preferred_name, AudioOutput& out_device)
{
    AudioOutputList outputs;
    QueryAllOutputs(outputs);

    const size_t num_audio_devs = outputs.size();
    if (num_audio_devs == 0)
    {
        out_device = {.name = "Default device (SDL)", .kind = AudioOutputKind::SDL};
        return PickOutputResult::NoOutputDevices;
    }

    if (preferred_name.size() == 0)
    {
        out_device = {.name = "Default device (SDL)", .kind = AudioOutputKind::SDL};
        return PickOutputResult::WantDefaultDevice;
    }

    for (size_t i = 0; i < num_audio_devs; ++i)
    {
        if (outputs[i].name == preferred_name)
        {
            out_device = outputs[i];
            return PickOutputResult::WantMatchedName;
        }
    }

    // maybe we have an index instead of a name
    if (size_t out_device_id; common::TryParse(preferred_name, out_device_id))
    {
        if (out_device_id < num_audio_devs)
        {
            out_device = outputs[out_device_id];
            return PickOutputResult::WantMatchedName;
        }
    }

    out_device = {.name = std::string(preferred_name), .kind = AudioOutputKind::SDL};
    return PickOutputResult::NoMatchingName;
}

const char* FE_AudioOutputMarkerString(AudioOutputKind kind)
{
    switch (kind)
    {
    case AudioOutputKind::SDL:
        // extra space is intentional; width of this string should match in all cases
        return "(SDL) ";
    case AudioOutputKind::ASIO:
        return "(ASIO)";
    }
    fprintf(stderr, "PANIC: FE_AudioOutputMarkerString got invalid kind");
    std::abort();
}

char FE_ChannelsTreeChar(bool is_last)
{
    return is_last ? '`' : '|';
}

void FE_WriteSpaces(int count)
{
    for (int i = 0; i < count; ++i)
    {
        fprintf(stderr, " ");
    }
}

void PrintAudioDevices(FILE* output)
{
    AudioOutputList outputs;
    QueryAllOutputs(outputs);

    if (outputs.size() == 0)
    {
        fprintf(output, "No output devices found.\n");
    }
    else
    {
        fprintf(output, "\nKnown output devices:\n\n");

        for (size_t i = 0; i < outputs.size(); ++i)
        {
#if NUKED_ENABLE_ASIO
            fprintf(output, "  %s %zu: %s\n", FE_AudioOutputMarkerString(outputs[i].kind), i, outputs[i].name.c_str());
            if (outputs[i].kind == AudioOutputKind::ASIO)
            {
                ASIO_OutputChannelList channels;
                if (Out_ASIO_QueryChannels(outputs[i].name.c_str(), channels))
                {
                    const size_t max_digits = NDigits((int32_t)(channels.size() - 1));

                    for (size_t channel = 0; channel < channels.size(); ++channel)
                    {
                        const size_t this_digits = NDigits((int32_t)channel);

                        // align under first character of output name
                        // 2 space indent, 6 marker string, 1 space, variable width number, ': '
                        FE_WriteSpaces(2 + 6 + 1 + (int)NDigits((int)i) + 2);

                        fprintf(output,
                                "%c-- channel %ld: ",
                                FE_ChannelsTreeChar(channel == channels.size() - 1),
                                channels[channel].id);

                        FE_WriteSpaces((int)(max_digits - this_digits));

                        fprintf(output, "%s\n", channels[channel].name.c_str());
                    }
                }
                else
                {
                    // align under first character of output name
                    // 2 space indent, 6 marker string, 1 space, variable width number, ': '
                    FE_WriteSpaces(2 + 6 + 1 + (int)NDigits((int)i) + 2);
                    fprintf(output, "(failed to query channels)\n");
                }
            }
#else
            fprintf(output, "  %zu: %s\n", i, outputs[i].name.c_str());
#endif
        }

        fprintf(output, "\n");
    }
}
