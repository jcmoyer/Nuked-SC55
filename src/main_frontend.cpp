/*
 * Copyright (C) 2021, 2024 nukeykt
 *
 *  Redistribution and use of this code or any derivative works are permitted
 *  provided that the following conditions are met:
 *
 *   - Redistributions may not be sold, nor may they be used in a commercial
 *     product or activity.
 *
 *   - Redistributions that are modified from the original source must include the
 *     complete source code, including the source code for all components used by a
 *     binary built from the modified sources. However, as a special exception, the
 *     source code distributed need not include anything that is normally distributed
 *     (in either source or binary form) with the major components (compiler, kernel,
 *     and so on) of the operating system on which the executable runs, unless that
 *     component itself accompanies the executable.
 *
 *   - Redistributions must reproduce the above copyright notice, this list of
 *     conditions and the following disclaimer in the documentation and/or other
 *     materials provided with the distribution.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 */
#include "emu.h"
#include "midi.h"
#include "ringbuffer.h"
#include "path_util.h"
#include "command_line.h"
#include "audio.h"
#include "cast.h"
#include "pcm.h"
#include <SDL.h>
#include <cinttypes>
#include <optional>
#include <vector>

#ifdef _WIN32
#include <Windows.h>
#endif

#include "asio_config.h"

#include "output_common.h"
#include "output_sdl.h"
#include "output_asio.h"

// Workaround until all compilers implement CWG 2518 (at time of writing, MSVC
// doesn't accept a static_assert(false) in a constexpr conditional branch)
template <typename>
constexpr bool DependentFalse = false;

struct FE_Instance
{
    Emulator emu;

    GenericBuffer sample_buffer;

    RingbufferView<AudioFrame<int16_t>> view_s16;
    RingbufferView<AudioFrame<int32_t>> view_s32;
    RingbufferView<AudioFrame<float>>   view_f32;

    std::thread thread;
    bool        running;
    AudioFormat format;

    // TODO: this is getting messy, we need to revisit how we manage buffers...
    // ASIO uses an SDL_AudioStream because it needs resampling to a more conventional frequency, but putting data into
    // the stream one frame at a time is *slow* so we buffer a chunk of audio in `frames` and add it all at once.
    SDL_AudioStream *stream;
    std::vector<AudioFrame<int32_t>> frames;

    // Statically selects the correct ringbuffer field into based on SampleT.
    template <typename SampleT>
    RingbufferView<AudioFrame<SampleT>>& StaticSelectBuffer()
    {
        if constexpr (std::is_same_v<SampleT, int16_t>)
        {
            return view_s16;
        }
        else if constexpr (std::is_same_v<SampleT, int32_t>)
        {
            return view_s32;
        }
        else if constexpr (std::is_same_v<SampleT, float>)
        {
            return view_f32;
        }
        else
        {
            static_assert(DependentFalse<SampleT>, "No valid case for SampleT");
        }
    }
};

const size_t FE_MAX_INSTANCES = 16;

struct FE_Application {
    FE_Instance instances[FE_MAX_INSTANCES];
    size_t instances_in_use = 0;

    uint32_t audio_buffer_size = 0;
    uint32_t audio_page_size = 0;

    SDL_AudioDeviceID sdl_audio = 0;
    AudioOutput audio_output{};

    bool running = false;
};

struct FE_Parameters
{
    bool help = false;
    std::string midi_device;
    std::string audio_device;
    uint32_t page_size = 512;
    uint32_t page_num = 32;
    bool autodetect = true;
    EMU_SystemReset reset = EMU_SystemReset::NONE;
    size_t instances = 1;
    Romset romset = Romset::MK2;
    std::optional<std::filesystem::path> rom_directory;
    AudioFormat output_format = AudioFormat::S16;
    bool no_lcd = false;
    bool disable_oversampling = false;
};

bool FE_AllocateInstance(FE_Application& container, FE_Instance** result)
{
    if (container.instances_in_use == FE_MAX_INSTANCES)
    {
        return false;
    }

    FE_Instance& fe = container.instances[container.instances_in_use];
    ++container.instances_in_use;

    if (result)
    {
        *result = &fe;
    }

    return true;
}

void FE_SendMIDI(FE_Application& fe, size_t n, std::span<const uint8_t> bytes)
{
    fe.instances[n].emu.PostMIDI(bytes);
}

void FE_BroadcastMIDI(FE_Application& fe, std::span<const uint8_t> bytes)
{
    for (size_t i = 0; i < fe.instances_in_use; ++i)
    {
        FE_SendMIDI(fe, i, bytes);
    }
}

void FE_RouteMIDI(FE_Application& fe, std::span<const uint8_t> bytes)
{
    if (bytes.size() == 0)
    {
        return;
    }

    uint8_t first = bytes[0];

    if (first < 0x80)
    {
        fprintf(stderr, "FE_RouteMIDI received data byte %02x\n", first);
        return;
    }

    const bool is_sysex = first == 0xF0;
    const uint8_t channel = first & 0x0F;

    if (is_sysex)
    {
        FE_BroadcastMIDI(fe, bytes);
    }
    else
    {
        FE_SendMIDI(fe, channel % fe.instances_in_use, bytes);
    }
}

template <typename SampleT>
void FE_ReceiveSampleSDL(void* userdata, const AudioFrame<int32_t>& in)
{
    FE_Instance& fe = *(FE_Instance*)userdata;

    AudioFrame<SampleT> out;
    Normalize(in, out);

    fe.StaticSelectBuffer<SampleT>().UncheckedWriteOne(out);
}

#ifdef NUKED_ENABLE_ASIO
void FE_ReceiveSampleASIO(void* userdata, const AudioFrame<int32_t>& in)
{
    FE_Instance& fe = *(FE_Instance*)userdata;

    AudioFrame<int32_t> out;
    Normalize(in, out);

    fe.frames.push_back(out);
}
#endif

template <typename SampleT>
void FE_AudioCallback(void* userdata, Uint8* stream, int len)
{
    FE_Application& frontend = *(FE_Application*)userdata;

    const size_t num_frames = (size_t)len / sizeof(AudioFrame<SampleT>);
    memset(stream, 0, (size_t)len);

    size_t renderable_count = num_frames;
    for (size_t i = 0; i < frontend.instances_in_use; ++i)
    {
        renderable_count = Min(
            renderable_count,
            frontend.instances[i].StaticSelectBuffer<SampleT>().GetReadableCount()
        );
    }

    for (size_t i = 0; i < frontend.instances_in_use; ++i)
    {
        ReadMix<SampleT>(frontend.instances[i].StaticSelectBuffer<SampleT>(), (AudioFrame<SampleT>*)stream, renderable_count);
    }
}

static const char* FE_AudioFormatStr(SDL_AudioFormat format)
{
    switch(format)
    {
    case AUDIO_S8:
        return "S8";
    case AUDIO_U8:
        return "U8";
    case AUDIO_S16MSB:
        return "S16MSB";
    case AUDIO_S16LSB:
        return "S16LSB";
    case AUDIO_U16MSB:
        return "U16MSB";
    case AUDIO_U16LSB:
        return "U16LSB";
    case AUDIO_S32MSB:
        return "S32MSB";
    case AUDIO_S32LSB:
        return "S32LSB";
    case AUDIO_F32MSB:
        return "F32MSB";
    case AUDIO_F32LSB:
        return "F32LSB";
    }
    return "UNK";
}

enum class FE_PickOutputResult
{
    WantMatchedName,
    WantDefaultDevice,
    NoOutputDevices,
    NoMatchingName,
};

void FE_QueryAllOutputs(AudioOutputList& outputs);

FE_PickOutputResult FE_PickOutputDevice(std::string_view preferred_name, AudioOutput& out_device)
{
    AudioOutputList outputs;
    FE_QueryAllOutputs(outputs);

    const size_t num_audio_devs = outputs.size();
    if (num_audio_devs == 0)
    {
        out_device = {.name = "Default device (SDL)", .kind = AudioOutputKind::SDL};
        return FE_PickOutputResult::NoOutputDevices;
    }

    if (preferred_name.size() == 0)
    {
        out_device = {.name = "Default device (SDL)", .kind = AudioOutputKind::SDL};
        return FE_PickOutputResult::WantDefaultDevice;
    }

    for (size_t i = 0; i < num_audio_devs; ++i)
    {
        if (outputs[i].name == preferred_name)
        {
            out_device = outputs[i];
            return FE_PickOutputResult::WantMatchedName;
        }
    }

    // maybe we have an index instead of a name
    if (int out_device_id; TryParse(preferred_name, out_device_id))
    {
        if (out_device_id >= 0 && out_device_id < (int)num_audio_devs)
        {
            out_device = outputs[out_device_id];
            return FE_PickOutputResult::WantMatchedName;
        }
    }

    out_device = {.name = std::string(preferred_name), .kind = AudioOutputKind::SDL};
    return FE_PickOutputResult::NoMatchingName;
}

void FE_QueryAllOutputs(AudioOutputList& outputs)
{
    outputs.clear();

    if (!Out_SDL_QueryOutputs(outputs))
    {
        fprintf(stderr, "Failed to query SDL outputs: %s\n", SDL_GetError());
        return;
    }

#ifdef NUKED_ENABLE_ASIO
    if (!Out_ASIO_QueryOutputs(outputs))
    {
        fprintf(stderr, "Failed to query ASIO outputs.\n");
        return;
    }
#endif
}

void FE_PrintAudioDevices()
{
    AudioOutputList outputs;
    FE_QueryAllOutputs(outputs);

    if (outputs.size() == 0)
    {
        fprintf(stderr, "No output devices found.\n");
    }
    else
    {
        fprintf(stderr, "\nKnown output devices:\n\n");

        for (size_t i = 0; i < outputs.size(); ++i)
        {
            fprintf(stderr, "  %d: %s\n", i, outputs[i].name.c_str());
        }

        fprintf(stderr, "\n");
    }
}

bool FE_OpenSDLAudio(FE_Application& fe, const FE_Parameters& params, const char* device_name)
{
    // TODO: revisit the frontend's idea of buffer sizes
    fe.audio_page_size = (params.page_size / 2) * 2; // must be even
    fe.audio_buffer_size = fe.audio_page_size * params.page_num;

    Out_SDL_SetFormat(params.output_format);
    Out_SDL_SetFrequency((int)PCM_GetOutputFrequency(fe.instances[0].emu.GetPCM()));
    Out_SDL_SetBufferSize((int)fe.audio_page_size / 4);

    for (size_t i = 0; i < fe.instances_in_use; ++i)
    {
        FE_Instance& inst = fe.instances[i];
        switch (inst.format)
        {
        case AudioFormat::S16:
            inst.sample_buffer.Init(sizeof(int16_t) * fe.audio_buffer_size);
            inst.view_s16 = RingbufferView<AudioFrame<int16_t>>(inst.sample_buffer);
            inst.emu.SetSampleCallback(FE_ReceiveSampleSDL<int16_t>, &inst);
            Out_SDL_AddStream(&fe.instances[i].view_s16);
            break;
        case AudioFormat::S32:
            inst.sample_buffer.Init(sizeof(int32_t) * fe.audio_buffer_size);
            inst.view_s32 = RingbufferView<AudioFrame<int32_t>>(inst.sample_buffer);
            inst.emu.SetSampleCallback(FE_ReceiveSampleSDL<int32_t>, &inst);
            Out_SDL_AddStream(&fe.instances[i].view_s32);
            break;
        case AudioFormat::F32:
            inst.sample_buffer.Init(sizeof(float) * fe.audio_buffer_size);
            inst.view_f32 = RingbufferView<AudioFrame<float>>(inst.sample_buffer);
            inst.emu.SetSampleCallback(FE_ReceiveSampleSDL<float>, &inst);
            Out_SDL_AddStream(&fe.instances[i].view_f32);
            break;
        }
    }

    return Out_SDL_Start(device_name);
}

#ifdef NUKED_ENABLE_ASIO
bool FE_OpenASIOAudio(FE_Application& fe, const FE_Parameters& params, const char* name)
{
    if (!Out_ASIO_Start(name))
    {
        return false;
    }

    for (size_t i = 0; i < fe.instances_in_use; ++i)
    {
        fe.instances[i].stream = SDL_NewAudioStream(AUDIO_S32,
                                                    2,
                                                    PCM_GetOutputFrequency(fe.instances[i].emu.GetPCM()),
                                                    Out_ASIO_GetFormat(),
                                                    2,
                                                    Out_ASIO_GetFrequency());
        Out_ASIO_AddStream(fe.instances[i].stream);
        fe.instances[i].emu.SetSampleCallback(FE_ReceiveSampleASIO, &fe.instances[i]);
    }

    return true;
}
#endif

bool FE_OpenAudio(FE_Application& fe, const FE_Parameters& params)
{
    AudioOutput output;
    FE_PickOutputResult output_result = FE_PickOutputDevice(params.audio_device, output);

    fe.audio_output = output;

    switch (output_result)
    {
    case FE_PickOutputResult::WantMatchedName:
        if (output.kind == AudioOutputKind::SDL)
        {
            return FE_OpenSDLAudio(fe, params, output.name.c_str());
        }
        else if (output.kind == AudioOutputKind::ASIO)
        {
#ifdef NUKED_ENABLE_ASIO
            return FE_OpenASIOAudio(fe, params, output.name.c_str());
#else
            fprintf(stderr, "Attempted to open ASIO output without ASIO support\n");
#endif
        }
        return false;
    case FE_PickOutputResult::WantDefaultDevice:
        return FE_OpenSDLAudio(fe, params, nullptr);
    case FE_PickOutputResult::NoOutputDevices:
        // in some cases this may still work
        fprintf(stderr, "No output devices found; attempting to open default device\n");
        return FE_OpenSDLAudio(fe, params, nullptr);
    case FE_PickOutputResult::NoMatchingName:
        // in some cases SDL cannot list all audio devices so we should still try
        fprintf(stderr, "No output device named '%s'; attempting to open it anyways...\n", params.audio_device.c_str());
        return FE_OpenSDLAudio(fe, params, output.name.c_str());
    }

    return true;
}

template <typename SampleT>
bool FE_IsBufferFull(FE_Instance& instance)
{
    // MCU_Step will always produce 1 or 2 samples. It's simplest to always leave enough space for oversampling because
    // then we don't need to re-align the write head every time.
    return instance.StaticSelectBuffer<SampleT>().GetWritableCount() < 2;
}

template <typename SampleT>
void FE_RunInstanceSDL(FE_Instance& instance)
{
    MCU_WorkThread_Lock(instance.emu.GetMCU());
    while (instance.running)
    {
        if (FE_IsBufferFull<SampleT>(instance))
        {
            MCU_WorkThread_Unlock(instance.emu.GetMCU());
            while (FE_IsBufferFull<SampleT>(instance))
            {
                SDL_Delay(1);
            }
            MCU_WorkThread_Lock(instance.emu.GetMCU());
        }

        MCU_Step(instance.emu.GetMCU());
    }
    MCU_WorkThread_Unlock(instance.emu.GetMCU());
}

#ifdef NUKED_ENABLE_ASIO
void FE_RunInstanceASIO(FE_Instance& instance)
{
    // TODO: get from out_asio
    const size_t preferredSize = 1024;

    while (instance.running)
    {
        // TODO: how to pick flush size? make configurable?
        if (instance.frames.size() >= preferredSize*2)
        {
            SDL_AudioStreamPut(instance.stream, instance.frames.data(), instance.frames.size() * sizeof(AudioFrame<int32_t>));
            instance.frames.clear();
        }

        while (SDL_AudioStreamAvailable(instance.stream) >= preferredSize*8*2)
        {
            SDL_Delay(1);
        }

        MCU_Step(instance.emu.GetMCU());
    }
}
#endif

bool FE_HandleGlobalEvent(FE_Application& fe, const SDL_Event& ev)
{
    switch (ev.type)
    {
        case SDL_QUIT:
            fe.running = false;
            return true;
        default:
            return false;
    }
}

void FE_EventLoop(FE_Application& fe)
{
    while (fe.running)
    {
#ifdef NUKED_ENABLE_ASIO
        if (Out_ASIO_IsResetRequested())
        {
            Out_ASIO_Reset();
        }
#endif

        for (size_t i = 0; i < fe.instances_in_use; ++i)
        {
            if (fe.instances[i].emu.IsLCDEnabled())
            {
                if (LCD_QuitRequested(fe.instances[i].emu.GetLCD()))
                {
                    fe.running = false;
                }

                LCD_Update(fe.instances[i].emu.GetLCD());
            }
        }

        SDL_Event ev;
        while (SDL_PollEvent(&ev))
        {
            if (FE_HandleGlobalEvent(fe, ev))
            {
                // not directed at any particular window; don't let LCDs
                // handle this one
                continue;
            }

            for (size_t i = 0; i < fe.instances_in_use; ++i)
            {
                if (fe.instances[i].emu.IsLCDEnabled())
                {
                    LCD_HandleEvent(fe.instances[i].emu.GetLCD(), ev);
                }
            }
        }

        SDL_Delay(15);
    }
}

void FE_Run(FE_Application& fe)
{
    fe.running = true;

    for (size_t i = 0; i < fe.instances_in_use; ++i)
    {
        fe.instances[i].running = true;
        if (fe.audio_output.kind == AudioOutputKind::SDL)
        {
            switch (fe.instances[i].format)
            {
            case AudioFormat::S16:
                fe.instances[i].thread = std::thread(FE_RunInstanceSDL<int16_t>, std::ref(fe.instances[i]));
                break;
            case AudioFormat::S32:
                fe.instances[i].thread = std::thread(FE_RunInstanceSDL<int32_t>, std::ref(fe.instances[i]));
                break;
            case AudioFormat::F32:
                fe.instances[i].thread = std::thread(FE_RunInstanceSDL<float>, std::ref(fe.instances[i]));
                break;
            }
        }
        else if (fe.audio_output.kind == AudioOutputKind::ASIO)
        {
#ifdef NUKED_ENABLE_ASIO
            fe.instances[i].thread = std::thread(FE_RunInstanceASIO, std::ref(fe.instances[i]));
#else
            fprintf(stderr, "Attempted to start ASIO instance without ASIO support\n");
#endif
        }
    }

    FE_EventLoop(fe);

    for (size_t i = 0; i < fe.instances_in_use; ++i)
    {
        fe.instances[i].running = false;
        fe.instances[i].thread.join();
    }
}

#ifdef _WIN32
// On Windows we install a Ctrl-C handler to make sure that the event loop always receives an SDL_QUIT event. This
// is what normally happens on other platforms but only some Windows environments (for instance, a mingw64 shell).
// If the program is run from cmd or Windows explorer, SDL_QUIT is never sent and the program hangs.
BOOL WINAPI FE_CtrlCHandler(DWORD dwCtrlType)
{
    (void)dwCtrlType;
    SDL_Event quit_event{};
    quit_event.type = SDL_QUIT;
    SDL_PushEvent(&quit_event);
    return TRUE;
}
#endif

bool FE_Init()
{
    // TODO: no longer initializing audio here
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0)
    {
        fprintf(stderr, "FATAL ERROR: Failed to initialize the SDL2: %s.\n", SDL_GetError());
        fflush(stderr);
        return false;
    }

#ifdef _WIN32
    SetConsoleCtrlHandler(FE_CtrlCHandler, TRUE);
#endif

    return true;
}

bool FE_CreateInstance(FE_Application& container, const std::filesystem::path& base_path, const FE_Parameters& params)
{
    FE_Instance* fe = nullptr;

    if (!FE_AllocateInstance(container, &fe))
    {
        fprintf(stderr, "ERROR: Failed to allocate instance.\n");
        return false;
    }

    fe->format = params.output_format;

    if (!fe->emu.Init(EMU_Options { .enable_lcd = !params.no_lcd }))
    {
        fprintf(stderr, "ERROR: Failed to init emulator.\n");
        return false;
    }

    LCD_LoadBack(fe->emu.GetLCD(), base_path / "back.data");

    if (!fe->emu.LoadRoms(params.romset, *params.rom_directory))
    {
        fprintf(stderr, "ERROR: Failed to load roms.\n");
        return false;
    }
    fe->emu.Reset();
    fe->emu.GetPCM().disable_oversampling = params.disable_oversampling;

    if (!params.no_lcd && !LCD_CreateWindow(fe->emu.GetLCD()))
    {
        fprintf(stderr, "ERROR: Failed to create window.\n");
        return false;
    }

    return true;
}

void FE_DestroyInstance(FE_Instance& fe)
{
    fe.running = false;
}

void FE_Quit(FE_Application& container)
{
    if (container.audio_output.kind == AudioOutputKind::ASIO)
    {
#ifdef NUKED_ENABLE_ASIO
        Out_ASIO_Stop();
#else
        fprintf(stderr, "Out_ASIO_Stop() called without ASIO support\n");
#endif
    }
    else
    {
        Out_SDL_Stop();
    }

    for (size_t i = 0; i < container.instances_in_use; ++i)
    {
        FE_DestroyInstance(container.instances[i]);
    }

    MIDI_Quit();
    SDL_Quit();
}

enum class FE_ParseError
{
    Success,
    InstancesInvalid,
    InstancesOutOfRange,
    UnexpectedEnd,
    PageSizeInvalid,
    PageCountInvalid,
    UnknownArgument,
    RomDirectoryNotFound,
    FormatInvalid,
};

const char* FE_ParseErrorStr(FE_ParseError err)
{
    switch (err)
    {
        case FE_ParseError::Success:
            return "Success";
        case FE_ParseError::InstancesInvalid:
            return "Instances couldn't be parsed (should be 1-16)";
        case FE_ParseError::InstancesOutOfRange:
            return "Instances out of range (should be 1-16)";
        case FE_ParseError::UnexpectedEnd:
            return "Expected another argument";
        case FE_ParseError::PageSizeInvalid:
            return "Page size invalid";
        case FE_ParseError::PageCountInvalid:
            return "Page count invalid";
        case FE_ParseError::UnknownArgument:
            return "Unknown argument";
        case FE_ParseError::RomDirectoryNotFound:
            return "Rom directory doesn't exist";
        case FE_ParseError::FormatInvalid:
            return "Output format invalid";
    }
    return "Unknown error";
}

FE_ParseError FE_ParseCommandLine(int argc, char* argv[], FE_Parameters& result)
{
    CommandLineReader reader(argc, argv);

    while (reader.Next())
    {
        if (reader.Any("-h", "--help", "-?"))
        {
            result.help = true;
            return FE_ParseError::Success;
        }
        else if (reader.Any("-p", "--port"))
        {
            if (!reader.Next())
            {
                return FE_ParseError::UnexpectedEnd;
            }

            result.midi_device = reader.Arg();
        }
        else if (reader.Any("-a", "--audio-device"))
        {
            if (!reader.Next())
            {
                return FE_ParseError::UnexpectedEnd;
            }

            result.audio_device = reader.Arg();
        }
        else if (reader.Any("-f", "--format"))
        {
            if (!reader.Next())
            {
                return FE_ParseError::UnexpectedEnd;
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
                return FE_ParseError::FormatInvalid;
            }
        }
        else if (reader.Any("-b", "--buffer-size"))
        {
            if (!reader.Next())
            {
                return FE_ParseError::UnexpectedEnd;
            }

            std::string_view arg = reader.Arg();
            if (size_t colon = arg.find(':'); colon != std::string_view::npos)
            {
                auto page_size_sv = arg.substr(0, colon);
                auto page_num_sv  = arg.substr(colon + 1);

                if (!TryParse(page_size_sv, result.page_size))
                {
                    return FE_ParseError::PageSizeInvalid;
                }

                if (!TryParse(page_num_sv, result.page_num))
                {
                    return FE_ParseError::PageCountInvalid;
                }
            }
            else if (!reader.TryParse(result.page_size))
            {
                return FE_ParseError::PageSizeInvalid;
            }
        }
        else if (reader.Any("-r", "--reset"))
        {
            if (!reader.Next())
            {
                return FE_ParseError::UnexpectedEnd;
            }

            if (reader.Arg() == "gm")
            {
                result.reset = EMU_SystemReset::GM_RESET;
            }
            else if (reader.Arg() == "gs")
            {
                result.reset = EMU_SystemReset::GS_RESET;
            }
            else
            {
                result.reset = EMU_SystemReset::NONE;
            }
        }
        else if (reader.Any("-n", "--instances"))
        {
            if (!reader.Next())
            {
                return FE_ParseError::UnexpectedEnd;
            }

            if (!reader.TryParse(result.instances))
            {
                return FE_ParseError::InstancesInvalid;
            }

            if (result.instances < 1 || result.instances > 16)
            {
                return FE_ParseError::InstancesOutOfRange;
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
        else if (reader.Any("-d", "--rom-directory"))
        {
            if (!reader.Next())
            {
                return FE_ParseError::UnexpectedEnd;
            }

            result.rom_directory = reader.Arg();

            if (!std::filesystem::exists(*result.rom_directory))
            {
                return FE_ParseError::RomDirectoryNotFound;
            }
        }
        else if (reader.Any("--mk2"))
        {
            result.romset = Romset::MK2;
            result.autodetect = false;
        }
        else if (reader.Any("--st"))
        {
            result.romset = Romset::ST;
            result.autodetect = false;
        }
        else if (reader.Any("--mk1"))
        {
            result.romset = Romset::MK1;
            result.autodetect = false;
        }
        else if (reader.Any("--cm300"))
        {
            result.romset = Romset::CM300;
            result.autodetect = false;
        }
        else if (reader.Any("--jv880"))
        {
            result.romset = Romset::JV880;
            result.autodetect = false;
        }
        else if (reader.Any("--scb55"))
        {
            result.romset = Romset::SCB55;
            result.autodetect = false;
        }
        else if (reader.Any("--rlp3237"))
        {
            result.romset = Romset::RLP3237;
            result.autodetect = false;
        }
        else if (reader.Any("--sc155"))
        {
            result.romset = Romset::SC155;
            result.autodetect = false;
        }
        else if (reader.Any("--sc155mk2"))
        {
            result.romset = Romset::SC155MK2;
            result.autodetect = false;
        }
        else
        {
            return FE_ParseError::UnknownArgument;
        }
    }

    return FE_ParseError::Success;
}

void FE_Usage()
{
    constexpr const char* USAGE_STR = R"(Usage: %s [options]

General options:
  -?, -h, --help                                Display this information.

Audio options:
  -p, --port         <device_name_or_number>    Set MIDI input port.
  -a, --audio-device <device_name_or_number>    Set output audio device.
  -b, --buffer-size  <page_size>[:page_count]   Set Audio Buffer size.
  -f, --format       s16|s32|f32                Set output format.
  --disable-oversampling                        Halves output frequency.

Emulator options:
  -r, --reset     gs|gm                         Reset system in GS or GM mode.
  -n, --instances <count>                       Set number of emulator instances.
  --no-lcd                                      Run without LCDs.

ROM management options:
  -d, --rom-directory <dir>                     Sets the directory to load roms from.
  --mk2                                         Use SC-55mk2 ROM set.
  --st                                          Use SC-55st ROM set.
  --mk1                                         Use SC-55mk1 ROM set.
  --cm300                                       Use CM-300/SCC-1 ROM set.
  --jv880                                       Use JV-880 ROM set.
  --scb55                                       Use SCB-55 ROM set.
  --rlp3237                                     Use RLP-3237 ROM set.

)";

    std::string name = P_GetProcessPath().stem().generic_string();
    fprintf(stderr, USAGE_STR, name.c_str());
    MIDI_PrintDevices();
    FE_PrintAudioDevices();
}

int main(int argc, char *argv[])
{
    FE_Parameters params;
    FE_ParseError result = FE_ParseCommandLine(argc, argv, params);
    if (result != FE_ParseError::Success)
    {
        fprintf(stderr, "error: %s\n", FE_ParseErrorStr(result));
        return 1;
    }

    if (params.help)
    {
        FE_Usage();
        return 0;
    }

    FE_Application frontend;

    std::filesystem::path base_path = P_GetProcessPath().parent_path();

    if (std::filesystem::exists(base_path / "../share/nuked-sc55"))
        base_path = base_path / "../share/nuked-sc55";

    fprintf(stderr, "Base path is: %s\n", base_path.generic_string().c_str());

    if (!params.rom_directory)
    {
        params.rom_directory = base_path;
    }

    fprintf(stderr, "ROM directory is: %s\n", params.rom_directory->generic_string().c_str());

    if (params.autodetect)
    {
        params.romset = EMU_DetectRomset(*params.rom_directory);
        fprintf(stderr, "ROM set autodetect: %s\n", EMU_RomsetName(params.romset));
    }

    if (!FE_Init())
    {
        fprintf(stderr, "FATAL ERROR: Failed to initialize frontend\n");
        return 1;
    }

    for (size_t i = 0; i < params.instances; ++i)
    {
        if (!FE_CreateInstance(frontend, base_path, params))
        {
            fprintf(stderr, "FATAL ERROR: Failed to create instance %" PRIu64 "\n", i);
            return 1;
        }
    }

    if (!FE_OpenAudio(frontend, params))
    {
        fprintf(stderr, "FATAL ERROR: Failed to open the audio stream.\n");
        fflush(stderr);
        return 1;
    }

    if (!MIDI_Init(frontend, params.midi_device))
    {
        fprintf(stderr, "ERROR: Failed to initialize the MIDI Input.\nWARNING: Continuing without MIDI Input...\n");
        fflush(stderr);
    }

    for (size_t i = 0; i < frontend.instances_in_use; ++i)
    {
        frontend.instances[i].emu.PostSystemReset(params.reset);
    }

    FE_Run(frontend);

    FE_Quit(frontend);

    return 0;
}
