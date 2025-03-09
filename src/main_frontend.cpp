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
#include <optional>
#include <bit>

#ifdef _WIN32
#include <Windows.h>
#endif

#include "asio_config.h"

#include "output_common.h"
#include "output_sdl.h"
#include "output_asio.h"

template <typename ElemT>
size_t FE_CalcRingbufferSizeBytes(uint32_t buffer_size, uint32_t buffer_count)
{
    return std::bit_ceil<size_t>(1 + (size_t)buffer_size * (size_t)buffer_count * sizeof(ElemT));
}

struct FE_Instance
{
    Emulator emu;

    GenericBuffer  sample_buffer;
    RingbufferView view;
    void*          chunk_first = nullptr;
    void*          chunk_last  = nullptr;

    std::thread thread;
    bool        running;
    AudioFormat format;

    uint32_t buffer_size;
    uint32_t buffer_count;

#ifdef NUKED_ENABLE_ASIO
    // ASIO uses an SDL_AudioStream because it needs resampling to a more conventional frequency, but putting data into
    // the stream one frame at a time is *slow* so we buffer audio in `sample_buffer` and add it all at once.
    SDL_AudioStream* stream = nullptr;
#endif

    template <typename SampleT>
    void Prepare()
    {
        auto span   = view.UncheckedPrepareWrite<AudioFrame<SampleT>>(buffer_size);
        chunk_first = span.data();
        chunk_last  = span.data() + span.size();
    }

    template <typename SampleT>
    void Finish()
    {
        view.UncheckedFinishWrite<AudioFrame<SampleT>>(buffer_size);
    }

    template <typename SampleT>
    void CreateAndPrepareBuffer()
    {
        sample_buffer.Init(FE_CalcRingbufferSizeBytes<AudioFrame<SampleT>>(buffer_size, buffer_count));
        view = RingbufferView(sample_buffer);
        Prepare<SampleT>();
    }
};

const size_t FE_MAX_INSTANCES = 16;

struct FE_Application {
    FE_Instance instances[FE_MAX_INSTANCES];
    size_t instances_in_use = 0;

    SDL_AudioDeviceID sdl_audio = 0;
    AudioOutput audio_output{};

    bool running = false;
};

struct FE_Parameters
{
    bool help = false;
    std::string midi_device;
    std::string audio_device;
    uint32_t buffer_size = 512;
    uint32_t buffer_count = 16;
    bool autodetect = true;
    EMU_SystemReset reset = EMU_SystemReset::NONE;
    size_t instances = 1;
    Romset romset = Romset::MK2;
    std::optional<std::filesystem::path> rom_directory;
    AudioFormat output_format = AudioFormat::S16;
    bool no_lcd = false;
    bool disable_oversampling = false;
    uint32_t asio_sample_rate = 44100;
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

    AudioFrame<SampleT>* out = (AudioFrame<SampleT>*)fe.chunk_first;
    Normalize(in, *out);
    fe.chunk_first = out + 1;

    if (fe.chunk_first == fe.chunk_last)
    {
        fe.Finish<SampleT>();
        fe.Prepare<SampleT>();
    }
}

#ifdef NUKED_ENABLE_ASIO
template <typename SampleT>
void FE_ReceiveSampleASIO(void* userdata, const AudioFrame<int32_t>& in)
{
    FE_Instance& fe = *(FE_Instance*)userdata;

    AudioFrame<SampleT>* out = (AudioFrame<SampleT>*)fe.chunk_first;
    Normalize(in, *out);
    fe.chunk_first = out + 1;

    if (fe.chunk_first == fe.chunk_last)
    {
        fe.Finish<SampleT>();
        fe.Prepare<SampleT>();

        auto span = fe.view.UncheckedPrepareRead<AudioFrame<SampleT>>(fe.buffer_size);
        SDL_AudioStreamPut(fe.stream, span.data(), (int)(span.size() * sizeof(AudioFrame<SampleT>)));
        fe.view.UncheckedFinishRead<AudioFrame<SampleT>>(fe.buffer_size);
    }
}
#endif

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
    if (size_t out_device_id; TryParse(preferred_name, out_device_id))
    {
        if (out_device_id < num_audio_devs)
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
            fprintf(stderr, "  %zu: %s\n", i, outputs[i].name.c_str());
        }

        fprintf(stderr, "\n");
    }
}

bool FE_OpenSDLAudio(FE_Application& fe, const AudioOutputParameters& params, const char* device_name)
{
    if (!Out_SDL_Create(device_name, params))
    {
        fprintf(stderr, "Failed to create SDL audio output\n");
        return false;
    }

    for (size_t i = 0; i < fe.instances_in_use; ++i)
    {
        FE_Instance& inst = fe.instances[i];
        switch (inst.format)
        {
        case AudioFormat::S16:
            inst.emu.SetSampleCallback(FE_ReceiveSampleSDL<int16_t>, &inst);
            inst.CreateAndPrepareBuffer<int16_t>();
            break;
        case AudioFormat::S32:
            inst.emu.SetSampleCallback(FE_ReceiveSampleSDL<int32_t>, &inst);
            inst.CreateAndPrepareBuffer<int32_t>();
            break;
        case AudioFormat::F32:
            inst.emu.SetSampleCallback(FE_ReceiveSampleSDL<float>, &inst);
            inst.CreateAndPrepareBuffer<float>();
            break;
        }
        Out_SDL_AddStream(fe.instances[i].view);
        fprintf(stderr, "#%02zu: allocated %zu bytes for audio\n", i, inst.sample_buffer.GetByteLength());
    }

    if (!Out_SDL_Start())
    {
        fprintf(stderr, "Failed to start SDL audio output\n");
        return false;
    }

    return true;
}

SDL_AudioFormat FE_ToSDLFormat(AudioFormat internal)
{
    switch (internal)
    {
    case AudioFormat::S16:
        return AUDIO_S16;
    case AudioFormat::S32:
        return AUDIO_S32;
    case AudioFormat::F32:
        return AUDIO_F32;
    default:
        fprintf(stderr, "Invalid audio format conversion\n");
        exit(1);
    }
}

#ifdef NUKED_ENABLE_ASIO
bool FE_OpenASIOAudio(FE_Application& fe, const AudioOutputParameters& params, const char* name)
{
    if (!Out_ASIO_Create(name, params))
    {
        fprintf(stderr, "Failed to create ASIO output\n");
        return false;
    }

    if (!Out_ASIO_Start())
    {
        fprintf(stderr, "Failed to create ASIO output\n");
        return false;
    }

    for (size_t i = 0; i < fe.instances_in_use; ++i)
    {
        FE_Instance& inst = fe.instances[i];

        inst.stream = SDL_NewAudioStream(FE_ToSDLFormat(inst.format),
                                         2,
                                         (int)PCM_GetOutputFrequency(inst.emu.GetPCM()),
                                         Out_ASIO_GetFormat(),
                                         2,
                                         Out_ASIO_GetFrequency());
        Out_ASIO_AddStream(inst.stream);

        switch (inst.format)
        {
        case AudioFormat::S16:
            inst.CreateAndPrepareBuffer<int16_t>();
            inst.emu.SetSampleCallback(FE_ReceiveSampleASIO<int16_t>, &inst);
            break;
        case AudioFormat::S32:
            inst.CreateAndPrepareBuffer<int32_t>();
            inst.emu.SetSampleCallback(FE_ReceiveSampleASIO<int32_t>, &inst);
            break;
        case AudioFormat::F32:
            inst.CreateAndPrepareBuffer<float>();
            inst.emu.SetSampleCallback(FE_ReceiveSampleASIO<float>, &inst);
            break;
        }
        fprintf(
            stderr, "#%02zu: allocated %zu bytes for audio\n", i, inst.sample_buffer.GetByteLength());
    }

    return true;
}
#endif

uint32_t FE_PickCloser(uint32_t to, uint32_t a, uint32_t b)
{
    if (abs((int32_t)(to - a)) < abs((int32_t)(to - b)))
    {
        return a;
    }
    else
    {
        return b;
    }
}

void FE_FixupParameters(FE_Parameters& params)
{
    if (!std::has_single_bit(params.buffer_size))
    {
        const uint32_t next_low  = std::bit_floor(params.buffer_size);
        const uint32_t next_high = std::bit_ceil(params.buffer_size);
        const uint32_t closer    = FE_PickCloser(params.buffer_size, next_low, next_high);
        fprintf(stderr, "WARNING: Audio buffer size must be a power-of-two; got %d\n", params.buffer_size);
        fprintf(stderr, "         The next valid values are %d and %d\n", next_low, next_high);
        fprintf(stderr, "         Continuing with the closer value %d\n", closer);
        params.buffer_size = closer;
    }
}

bool FE_OpenAudio(FE_Application& fe, const FE_Parameters& params)
{
    AudioOutput         output;
    FE_PickOutputResult output_result = FE_PickOutputDevice(params.audio_device, output);

    fe.audio_output = output;

    AudioOutputParameters out_params;
    out_params.frequency   = PCM_GetOutputFrequency(fe.instances[0].emu.GetPCM());
    out_params.buffer_size = params.buffer_size;
    out_params.format      = params.output_format;

    switch (output_result)
    {
    case FE_PickOutputResult::WantMatchedName:
        if (output.kind == AudioOutputKind::SDL)
        {
            return FE_OpenSDLAudio(fe, out_params, output.name.c_str());
        }
        else if (output.kind == AudioOutputKind::ASIO)
        {
#ifdef NUKED_ENABLE_ASIO
            // ASIO will probably not support the emulator's native sample rate
            out_params.frequency = params.asio_sample_rate;
            return FE_OpenASIOAudio(fe, out_params, output.name.c_str());
#else
            fprintf(stderr, "Attempted to open ASIO output without ASIO support\n");
#endif
        }
        return false;
    case FE_PickOutputResult::WantDefaultDevice:
        return FE_OpenSDLAudio(fe, out_params, nullptr);
    case FE_PickOutputResult::NoOutputDevices:
        // in some cases this may still work
        fprintf(stderr, "No output devices found; attempting to open default device\n");
        return FE_OpenSDLAudio(fe, out_params, nullptr);
    case FE_PickOutputResult::NoMatchingName:
        // in some cases SDL cannot list all audio devices so we should still try
        fprintf(stderr, "No output device named '%s'; attempting to open it anyways...\n", params.audio_device.c_str());
        return FE_OpenSDLAudio(fe, out_params, output.name.c_str());
    }

    return true;
}

template <typename SampleT>
void FE_RunInstanceSDL(FE_Instance& instance)
{
    const size_t max_byte_count = instance.buffer_count * instance.buffer_size * sizeof(AudioFrame<SampleT>);

    while (instance.running)
    {
        while (instance.view.GetReadableBytes() >= max_byte_count)
        {
            SDL_Delay(1);
        }

        MCU_Step(instance.emu.GetMCU());
    }
}

#ifdef NUKED_ENABLE_ASIO
void FE_RunInstanceASIO(FE_Instance& instance)
{
    while (instance.running)
    {
        // we recalc every time because ASIO reset might change this
        const size_t buffer_size = (size_t)Out_ASIO_GetBufferSize();

        // note that this is the byte count coming out of the stream; it won't line up with the amount of data we put in
        // so be careful not to confuse the two!!
        const size_t max_byte_count = instance.buffer_count * buffer_size * Out_ASIO_GetFormatFrameSizeBytes();

        while ((size_t)SDL_AudioStreamAvailable(instance.stream) >= max_byte_count)
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

    fe->format       = params.output_format;
    fe->buffer_size  = params.buffer_size;
    fe->buffer_count = params.buffer_count;

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

void FE_DestroyInstance(FE_Instance& instance)
{
#ifdef NUKED_ENABLE_ASIO
    if (instance.stream)
    {
        SDL_FreeAudioStream(instance.stream);
        instance.stream = nullptr;
    }
#else
    (void)instance;
#endif
}

void FE_Quit(FE_Application& container)
{
    if (container.audio_output.kind == AudioOutputKind::ASIO)
    {
#ifdef NUKED_ENABLE_ASIO
        Out_ASIO_Stop();
        Out_ASIO_Destroy();
#else
        fprintf(stderr, "Out_ASIO_Stop() called without ASIO support\n");
#endif
    }
    else
    {
        Out_SDL_Stop();
        Out_SDL_Destroy();
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
    BufferSizeInvalid,
    BufferCountInvalid,
    UnknownArgument,
    RomDirectoryNotFound,
    FormatInvalid,
    ASIOSampleRateOutOfRange,
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
        case FE_ParseError::BufferSizeInvalid:
            return "Buffer size invalid";
        case FE_ParseError::BufferCountInvalid:
            return "Buffer count invalid (should be greater than zero)";
        case FE_ParseError::UnknownArgument:
            return "Unknown argument";
        case FE_ParseError::RomDirectoryNotFound:
            return "Rom directory doesn't exist";
        case FE_ParseError::FormatInvalid:
            return "Output format invalid";
        case FE_ParseError::ASIOSampleRateOutOfRange:
            return "ASIO sample rate out of range";
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
                auto buffer_size_sv  = arg.substr(0, colon);
                auto buffer_count_sv = arg.substr(colon + 1);

                if (!TryParse(buffer_size_sv, result.buffer_size))
                {
                    return FE_ParseError::BufferSizeInvalid;
                }

                if (!TryParse(buffer_count_sv, result.buffer_count))
                {
                    return FE_ParseError::BufferCountInvalid;
                }

                if (result.buffer_count == 0)
                {
                    return FE_ParseError::BufferCountInvalid;
                }
            }
            else if (!reader.TryParse(result.buffer_size))
            {
                return FE_ParseError::BufferSizeInvalid;
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
        else if (reader.Any("--asio-sample-rate"))
        {
            if (!reader.Next())
            {
                return FE_ParseError::UnexpectedEnd;
            }

            if (!reader.TryParse(result.asio_sample_rate))
            {
                return FE_ParseError::ASIOSampleRateOutOfRange;
            }
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
  -b, --buffer-size  <size>[:count]             Set buffer size, number of buffers.
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

    FE_FixupParameters(params);

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
            fprintf(stderr, "FATAL ERROR: Failed to create instance %zu\n", i);
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
