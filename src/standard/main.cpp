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
#include "audio.h"
#include "audio_sdl.h"
#include "bounded_vector.h"
#include "cast.h"
#include "config.h"
#include "emu.h"
#include "lcd_sdl.h"
#include "mcu.h"
#include "midi.h"
#include "output_common.h"
#include "pcm.h"
#include "ringbuffer.h"
#include <SDL.h>
#include <bit>
#include <optional>
#include <thread>

#include "output_asio.h"
#include "output_sdl.h"

#include "common/command_line.h"
#include "common/gain.h"
#include "common/path_util.h"
#include "common/rom_loader.h"

#ifdef _WIN32
#include <Windows.h>
#endif

template <typename ElemT>
size_t FE_CalcRingbufferSizeBytes(uint32_t buffer_size, uint32_t buffer_count)
{
    return std::bit_ceil<size_t>(1 + (size_t)buffer_size * (size_t)buffer_count * sizeof(ElemT));
}

struct FE_Instance
{
    Emulator emu;

    std::unique_ptr<LCD_SDL_Backend> sdl_lcd;

    GenericBuffer  sample_buffer;
    RingbufferView view;
    void*          chunk_first = nullptr;
    void*          chunk_last  = nullptr;

    std::thread thread;
    AudioFormat format;

    // read by instance thread, written by main thread
    std::atomic<bool> running = false;

    uint32_t buffer_size;
    uint32_t buffer_count;

    float gain = 1.0f;

#if NUKED_ENABLE_ASIO
    // ASIO uses an SDL_AudioStream because it needs resampling to a more conventional frequency, but putting data into
    // the stream one frame at a time is *slow* so we buffer audio in `sample_buffer` and add it all at once.
    SDL_AudioStream* stream = nullptr;
#endif

    ~FE_Instance()
    {
#if NUKED_ENABLE_ASIO
        if (stream)
        {
            SDL_FreeAudioStream(stream);
            stream = nullptr;
        }
#endif
    }

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

struct FE_Parameters;

class FE_Application : private MIDI_Output
{
public:
    FE_Application() = default;
    ~FE_Application();

    FE_Application(const FE_Application&)            = delete;
    FE_Application& operator=(const FE_Application&) = delete;

    FE_Application(FE_Application&&)            = delete;
    FE_Application& operator=(FE_Application&&) = delete;

    bool Initialize(const FE_Parameters& params);

    void Run();
    bool CreateInstance(const std::filesystem::path& base_path, const FE_Parameters& params);

    void SendMIDI(size_t n, std::span<const uint8_t> bytes);
    void BroadcastMIDI(std::span<const uint8_t> bytes);
    void RouteMIDI(std::span<const uint8_t> bytes);

private:
    void RunEventLoop();
    bool HandleGlobalEvent(const SDL_Event& ev);
    bool AllocateInstance(FE_Instance** result);
    bool OpenSDLAudio(const AudioOutputParameters& params, const char* device_name);
#if NUKED_ENABLE_ASIO
    bool OpenASIOAudio(const ASIO_OutputParameters& params, const char* name);
#endif
    bool OpenAudio(const FE_Parameters& params);

    constexpr mcu_sample_callback PickCallback(const FE_Instance& inst);

    // MIDI_Output interface
    void Write(std::span<const uint8_t> bytes) override
    {
        RouteMIDI(bytes);
    }

private:
    BoundedVector<FE_Instance, FE_MAX_INSTANCES> m_instances;

    AllRomsetInfo m_romset_info;
    Romset        m_romset;

    AudioOutput m_audio_output{};

    bool m_running = false;
};

struct FE_AdvancedParameters
{
    common::RomOverrides rom_overrides;
};

struct FE_Parameters
{
    bool help = false;
    bool version = false;
    std::string midi_device;
    std::string audio_device;
    uint32_t buffer_size = 512;
    uint32_t buffer_count = 16;
    std::optional<EMU_SystemReset> reset;
    size_t instances = 1;
    std::string_view romset_name;
    bool legacy_romset_detection = false;
    std::optional<std::filesystem::path> rom_directory;
    AudioFormat output_format = AudioFormat::S16;
    bool no_lcd = false;
    bool disable_oversampling = false;
    std::optional<uint32_t> asio_sample_rate;
    std::string asio_left_channel;
    std::string asio_right_channel;
    std::filesystem::path nvram_filename;
    FE_AdvancedParameters adv;
    float gain = 1.0f;
};

bool FE_Application::AllocateInstance(FE_Instance** result)
{
    if (m_instances.IsFull())
    {
        return false;
    }

    FE_Instance& fe = m_instances.EmplaceBack();

    if (result)
    {
        *result = &fe;
    }

    return true;
}

void FE_Application::SendMIDI(size_t instance_id, std::span<const uint8_t> bytes)
{
    m_instances[instance_id].emu.PostMIDI(bytes);
}

void FE_Application::BroadcastMIDI(std::span<const uint8_t> bytes)
{
    for (size_t i = 0; i < m_instances.Count(); ++i)
    {
        SendMIDI(i, bytes);
    }
}

void FE_Application::RouteMIDI(std::span<const uint8_t> bytes)
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
        BroadcastMIDI(bytes);
    }
    else
    {
        SendMIDI(channel % m_instances.Count(), bytes);
    }
}

template <typename SampleT, bool ApplyGain>
void FE_ReceiveSampleSDL(void* userdata, const AudioFrame<int32_t>& in)
{
    FE_Instance& fe = *(FE_Instance*)userdata;

    AudioFrame<SampleT>* out = (AudioFrame<SampleT>*)fe.chunk_first;
    Normalize(in, *out);

    if constexpr (ApplyGain)
    {
        Scale(*out, fe.gain);
    }

    fe.chunk_first = out + 1;

    if (fe.chunk_first == fe.chunk_last)
    {
        fe.Finish<SampleT>();
        fe.Prepare<SampleT>();
    }
}

#if NUKED_ENABLE_ASIO
template <typename SampleT, bool ApplyGain>
void FE_ReceiveSampleASIO(void* userdata, const AudioFrame<int32_t>& in)
{
    FE_Instance& fe = *(FE_Instance*)userdata;

    AudioFrame<SampleT>* out = (AudioFrame<SampleT>*)fe.chunk_first;
    Normalize(in, *out);

    if constexpr (ApplyGain)
    {
        Scale(*out, fe.gain);
    }

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

constexpr mcu_sample_callback FE_Application::PickCallback(const FE_Instance& inst)
{
    if (m_audio_output.kind == AudioOutputKind::SDL)
    {
        if (inst.gain != 1.f)
        {
            switch (inst.format)
            {
            case AudioFormat::S16:
                return FE_ReceiveSampleSDL<int16_t, true>;
            case AudioFormat::S32:
                return FE_ReceiveSampleSDL<int32_t, true>;
            case AudioFormat::F32:
                return FE_ReceiveSampleSDL<float, true>;
            }
        }
        else
        {
            switch (inst.format)
            {
            case AudioFormat::S16:
                return FE_ReceiveSampleSDL<int16_t, false>;
            case AudioFormat::S32:
                return FE_ReceiveSampleSDL<int32_t, false>;
            case AudioFormat::F32:
                return FE_ReceiveSampleSDL<float, false>;
            }
        }
    }
    else
    {
#if NUKED_ENABLE_ASIO
        if (inst.gain != 1.f)
        {
            switch (inst.format)
            {
            case AudioFormat::S16:
                return FE_ReceiveSampleASIO<int16_t, true>;
            case AudioFormat::S32:
                return FE_ReceiveSampleASIO<int32_t, true>;
            case AudioFormat::F32:
                return FE_ReceiveSampleASIO<float, true>;
            }
        }
        else
        {
            switch (inst.format)
            {
            case AudioFormat::S16:
                return FE_ReceiveSampleASIO<int16_t, false>;
            case AudioFormat::S32:
                return FE_ReceiveSampleASIO<int32_t, false>;
            case AudioFormat::F32:
                return FE_ReceiveSampleASIO<float, false>;
            }
        }
#else
        fprintf(stderr, "PANIC: FE_PickCallback tried to select ASIO output without ASIO support\n");
        std::abort();
#endif
    }

    fprintf(stderr, "output kind = %d\n", (int)m_audio_output.kind);
    fprintf(stderr, "gain = %f\n", inst.gain);
    fprintf(stderr, "format = %d\n", (int)inst.format);
    return nullptr;
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
    if (size_t out_device_id; common::TryParse(preferred_name, out_device_id))
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

#if NUKED_ENABLE_ASIO
    if (!Out_ASIO_QueryOutputs(outputs))
    {
        fprintf(stderr, "Failed to query ASIO outputs.\n");
        return;
    }
#endif
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
#if NUKED_ENABLE_ASIO
            fprintf(stderr, "  %s %zu: %s\n", FE_AudioOutputMarkerString(outputs[i].kind), i, outputs[i].name.c_str());
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

                        fprintf(stderr,
                                "%c-- channel %ld: ",
                                FE_ChannelsTreeChar(channel == channels.size() - 1),
                                channels[channel].id);

                        FE_WriteSpaces((int)(max_digits - this_digits));

                        fprintf(stderr, "%s\n", channels[channel].name.c_str());
                    }
                }
                else
                {
                    // align under first character of output name
                    // 2 space indent, 6 marker string, 1 space, variable width number, ': '
                    FE_WriteSpaces(2 + 6 + 1 + (int)NDigits((int)i) + 2);
                    fprintf(stderr, "(failed to query channels)\n");
                }
            }
#else
            fprintf(stderr, "  %zu: %s\n", i, outputs[i].name.c_str());
#endif
        }

        fprintf(stderr, "\n");
    }
}

bool FE_Application::OpenSDLAudio(const AudioOutputParameters& params, const char* device_name)
{
    if (!Out_SDL_Create(device_name, params))
    {
        fprintf(stderr, "Failed to create SDL audio output\n");
        return false;
    }

    for (size_t id = 0; id < m_instances.Count(); ++id)
    {
        FE_Instance& inst = m_instances[id];
        inst.emu.SetSampleCallback(PickCallback(inst), &inst);
        switch (inst.format)
        {
        case AudioFormat::S16:
            inst.CreateAndPrepareBuffer<int16_t>();
            break;
        case AudioFormat::S32:
            inst.CreateAndPrepareBuffer<int32_t>();
            break;
        case AudioFormat::F32:
            inst.CreateAndPrepareBuffer<float>();
            break;
        }
        Out_SDL_AddSource(m_instances[id].view);
        fprintf(stderr, "#%02zu: allocated %zu bytes for audio\n", id, inst.sample_buffer.GetByteLength());
    }

    if (!Out_SDL_Start())
    {
        fprintf(stderr, "Failed to start SDL audio output\n");
        return false;
    }

    return true;
}

#if NUKED_ENABLE_ASIO
bool FE_Application::OpenASIOAudio(const ASIO_OutputParameters& params, const char* name)
{
    if (!Out_ASIO_Create(name, params))
    {
        fprintf(stderr, "Failed to create ASIO output\n");
        return false;
    }

    for (size_t id = 0; id < m_instances.Count(); ++id)
    {
        FE_Instance& inst = m_instances[id];

        inst.stream = SDL_NewAudioStream(AudioFormatToSDLAudioFormat(inst.format),
                                         2,
                                         (int)PCM_GetOutputFrequency(inst.emu.GetPCM()),
                                         Out_ASIO_GetFormat(),
                                         2,
                                         Out_ASIO_GetFrequency());
        Out_ASIO_AddSource(inst.stream);

        inst.emu.SetSampleCallback(PickCallback(inst), &inst);

        switch (inst.format)
        {
        case AudioFormat::S16:
            inst.CreateAndPrepareBuffer<int16_t>();
            break;
        case AudioFormat::S32:
            inst.CreateAndPrepareBuffer<int32_t>();
            break;
        case AudioFormat::F32:
            inst.CreateAndPrepareBuffer<float>();
            break;
        }
        fprintf(stderr, "#%02zu: allocated %zu bytes for audio\n", id, inst.sample_buffer.GetByteLength());
    }

    if (!Out_ASIO_Start())
    {
        fprintf(stderr, "Failed to create ASIO output\n");
        return false;
    }

    return true;
}
#endif

void FE_FixupParameters(FE_Parameters& params)
{
    if (!std::has_single_bit(params.buffer_size))
    {
        const uint32_t next_low  = std::bit_floor(params.buffer_size);
        const uint32_t next_high = std::bit_ceil(params.buffer_size);
        const uint32_t closer =
            (uint32_t)PickCloser<int64_t>((int64_t)params.buffer_size, (int64_t)next_low, (int64_t)next_high);
        fprintf(stderr, "WARNING: Audio buffer size must be a power-of-two; got %d\n", params.buffer_size);
        fprintf(stderr, "         The next valid values are %d and %d\n", next_low, next_high);
        fprintf(stderr, "         Continuing with the closer value %d\n", closer);
        params.buffer_size = closer;
    }
}

bool FE_Application::OpenAudio(const FE_Parameters& params)
{
    AudioOutput         output;
    FE_PickOutputResult output_result = FE_PickOutputDevice(params.audio_device, output);

    m_audio_output = output;

    AudioOutputParameters out_params;
    out_params.frequency = PCM_GetOutputFrequency(m_instances[0].emu.GetPCM());
    switch (output.kind)
    {
    case AudioOutputKind::SDL:
        // explicitly do nothing
        break;
    case AudioOutputKind::ASIO:
        if (params.asio_sample_rate.has_value())
        {
            out_params.frequency = params.asio_sample_rate.value();
        }
        break;
    }
    out_params.buffer_size = params.buffer_size;
    out_params.format      = params.output_format;

    switch (output_result)
    {
    case FE_PickOutputResult::WantMatchedName:
        if (output.kind == AudioOutputKind::SDL)
        {
            return OpenSDLAudio(out_params, output.name.c_str());
        }
        else if (output.kind == AudioOutputKind::ASIO)
        {
#if NUKED_ENABLE_ASIO
            ASIO_OutputParameters asio_params;
            asio_params.common = out_params;
            asio_params.left_channel  = params.asio_left_channel;
            asio_params.right_channel = params.asio_right_channel;
            return OpenASIOAudio(asio_params, output.name.c_str());
#else
            fprintf(stderr, "Attempted to open ASIO output without ASIO support\n");
#endif
        }
        return false;
    case FE_PickOutputResult::WantDefaultDevice:
        return OpenSDLAudio(out_params, nullptr);
    case FE_PickOutputResult::NoOutputDevices:
        // in some cases this may still work
        fprintf(stderr, "No output devices found; attempting to open default device\n");
        return OpenSDLAudio(out_params, nullptr);
    case FE_PickOutputResult::NoMatchingName:
        // in some cases SDL cannot list all audio devices so we should still try
        fprintf(stderr, "No output device named '%s'; attempting to open it anyways...\n", params.audio_device.c_str());
        return OpenSDLAudio(out_params, output.name.c_str());
    }

    return false;
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

        instance.emu.Step();
    }
}

#if NUKED_ENABLE_ASIO
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

        instance.emu.Step();
    }
}
#endif

bool FE_Application::HandleGlobalEvent(const SDL_Event& ev)
{
    switch (ev.type)
    {
        case SDL_QUIT:
            m_running = false;
            return true;
        default:
            return false;
    }
}

void FE_Application::RunEventLoop()
{
    while (m_running)
    {
#if NUKED_ENABLE_ASIO
        if (Out_ASIO_IsResetRequested())
        {
            Out_ASIO_Reset();
        }
#endif

        for (FE_Instance& inst : m_instances)
        {
            if (inst.sdl_lcd)
            {
                if (inst.sdl_lcd->IsQuitRequested())
                {
                    m_running = false;
                }
            }
            LCD_Render(inst.emu.GetLCD());
        }

        SDL_Event ev;
        while (SDL_PollEvent(&ev))
        {
            if (HandleGlobalEvent(ev))
            {
                // not directed at any particular window; don't let LCDs
                // handle this one
                continue;
            }

            for (FE_Instance& inst : m_instances)
            {
                if (inst.sdl_lcd)
                {
                    inst.sdl_lcd->HandleEvent(ev);
                }
            }
        }

        SDL_Delay(15);
    }
}

void FE_Application::Run()
{
    m_running = true;

    for (FE_Instance& inst : m_instances)
    {
        inst.running = true;
        if (m_audio_output.kind == AudioOutputKind::SDL)
        {
            switch (inst.format)
            {
            case AudioFormat::S16:
                inst.thread = std::thread(FE_RunInstanceSDL<int16_t>, std::ref(inst));
                break;
            case AudioFormat::S32:
                inst.thread = std::thread(FE_RunInstanceSDL<int32_t>, std::ref(inst));
                break;
            case AudioFormat::F32:
                inst.thread = std::thread(FE_RunInstanceSDL<float>, std::ref(inst));
                break;
            }
        }
        else if (m_audio_output.kind == AudioOutputKind::ASIO)
        {
#if NUKED_ENABLE_ASIO
            inst.thread = std::thread(FE_RunInstanceASIO, std::ref(inst));
#else
            fprintf(stderr, "Attempted to start ASIO instance without ASIO support\n");
#endif
        }
    }

    RunEventLoop();

    for (FE_Instance& inst : m_instances)
    {
        inst.running = false;
        inst.thread.join();
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

bool FE_GlobalInit()
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0)
    {
        fprintf(stderr, "FATAL ERROR: Failed to initialize SDL: %s.\n", SDL_GetError());
        fflush(stderr);
        return false;
    }

#ifdef _WIN32
    SetConsoleCtrlHandler(FE_CtrlCHandler, TRUE);
#endif

    return true;
}

void FE_GlobalQuit()
{
    SDL_Quit();
}

bool FE_Application::CreateInstance(const std::filesystem::path& base_path, const FE_Parameters& params)
{
    (void)base_path;

    FE_Instance* fe = nullptr;

    const size_t instance_id = m_instances.Count();

    if (!AllocateInstance(&fe))
    {
        fprintf(stderr, "ERROR: Failed to allocate instance.\n");
        return false;
    }

    fe->format       = params.output_format;
    fe->buffer_size  = params.buffer_size;
    fe->buffer_count = params.buffer_count;
    fe->gain         = params.gain;

    if (!params.no_lcd)
    {
        fe->sdl_lcd = std::make_unique<LCD_SDL_Backend>();
    }

    std::filesystem::path this_nvram = params.nvram_filename;
    if (!this_nvram.empty())
    {
        // append instance number so that multiple instances don't clobber each other's nvram
        this_nvram += std::to_string(instance_id);
    }

    if (!fe->emu.Init({.lcd_backend = fe->sdl_lcd.get(), .nvram_filename = this_nvram}))
    {
        fprintf(stderr, "ERROR: Failed to init emulator.\n");
        return false;
    }

    if (!fe->emu.LoadRoms(m_romset, m_romset_info))
    {
        fprintf(stderr, "ERROR: Failed to load roms for instance %02zu\n", instance_id);
        return false;
    }

    fe->emu.Reset();
    fe->emu.GetPCM().disable_oversampling = params.disable_oversampling;

    if (!fe->emu.StartLCD())
    {
        fprintf(stderr, "ERROR: Failed to start LCD.\n");
        return false;
    }

    return true;
}

FE_Application::~FE_Application()
{
    switch (m_audio_output.kind)
    {
    case AudioOutputKind::ASIO:
#if NUKED_ENABLE_ASIO
        Out_ASIO_Stop();
        Out_ASIO_Destroy();
#else
        fprintf(stderr, "Out_ASIO_Stop() called without ASIO support\n");
#endif
        break;
    case AudioOutputKind::SDL:
        Out_SDL_Stop();
        Out_SDL_Destroy();
        break;
    }

    MIDI_Quit();
}

bool FE_Application::Initialize(const FE_Parameters& params)
{
    std::filesystem::path base_path = common::GetProcessPath().parent_path();

    if (std::filesystem::exists(base_path / "../share/nuked-sc55"))
        base_path = base_path / "../share/nuked-sc55";

    fprintf(stderr, "Base path is: %s\n", base_path.generic_string().c_str());

    std::filesystem::path rom_directory;

    if (params.rom_directory)
    {
        rom_directory = *params.rom_directory;
    }
    else
    {
        rom_directory = base_path;
    }

    fprintf(stderr, "ROM directory is: %s\n", rom_directory.generic_string().c_str());

    common::LoadRomsetResult load_result;

    common::LoadRomsetError err = common::LoadRomset(m_romset_info,
                                                     rom_directory,
                                                     params.romset_name,
                                                     params.legacy_romset_detection,
                                                     params.adv.rom_overrides,
                                                     load_result);

    common::PrintLoadRomsetDiagnostics(stderr, err, load_result, m_romset_info);

    if (err != common::LoadRomsetError{})
    {
        return false;
    }

    m_romset = load_result.romset;

    EMU_SystemReset reset = EMU_SystemReset::NONE;
    if (params.reset)
    {
        reset = *params.reset;
    }
    else if (!params.reset && m_romset == Romset::MK2)
    {
        // user didn't explicitly pass a reset and we're using a buggy romset
        fprintf(stderr, "WARNING: No reset specified with mk2 romset; using gs\n");
        reset = EMU_SystemReset::GS_RESET;
    }

    fprintf(stderr, "Gain set to %.2fdb\n", common::ScalarToDb(params.gain));

    for (size_t i = 0; i < params.instances; ++i)
    {
        if (!CreateInstance(base_path, params))
        {
            fprintf(stderr, "FATAL ERROR: Failed to create instance %zu\n", i);
            return false;
        }
    }

    m_romset_info.PurgeRomData();

    if (!OpenAudio(params))
    {
        fprintf(stderr, "FATAL ERROR: Failed to open the audio stream.\n");
        fflush(stderr);
        return false;
    }

    if (!MIDI_Init(*this, params.midi_device))
    {
        fprintf(stderr, "ERROR: Failed to initialize the MIDI Input.\nWARNING: Continuing without MIDI Input...\n");
        fflush(stderr);
    }

    for (FE_Instance& inst : m_instances)
    {
        inst.emu.PostSystemReset(reset);
    }

    return true;
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
    ASIOChannelInvalid,
    ResetInvalid,
    GainInvalid,
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
        case FE_ParseError::ASIOChannelInvalid:
            return "ASIO channel invalid";
        case FE_ParseError::ResetInvalid:
            return "Reset invalid (should be none, gs, or gm)";
        case FE_ParseError::GainInvalid:
            return "Gain invalid (should be a number optionally ending in 'db')";
        }
    return "Unknown error";
}

FE_ParseError FE_ParseCommandLine(int argc, char* argv[], FE_Parameters& result)
{
    common::CommandLineReader reader(argc, argv);

    while (reader.Next())
    {
        if (reader.Any("-h", "--help", "-?"))
        {
            result.help = true;
            return FE_ParseError::Success;
        }
        else if (reader.Any("-v", "--version"))
        {
            result.version = true;
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

                if (!common::TryParse(buffer_size_sv, result.buffer_size))
                {
                    return FE_ParseError::BufferSizeInvalid;
                }

                if (!common::TryParse(buffer_count_sv, result.buffer_count))
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
            else if (reader.Arg() == "none")
            {
                result.reset = EMU_SystemReset::NONE;
            }
            else
            {
                return FE_ParseError::ResetInvalid;
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
        else if (reader.Any("--gain"))
        {
            if (!reader.Next())
            {
                return FE_ParseError::UnexpectedEnd;
            }

            if (common::ParseGain(reader.Arg(), result.gain) != common::ParseGainResult{})
            {
                return FE_ParseError::GainInvalid;
            }
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
        else if (reader.Any("--nvram"))
        {
            if (!reader.Next())
            {
                return FE_ParseError::UnexpectedEnd;
            }

            result.nvram_filename = reader.Arg();
        }
        else if (reader.Any("--romset"))
        {
            if (!reader.Next())
            {
                return FE_ParseError::UnexpectedEnd;
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
                return FE_ParseError::UnexpectedEnd;
            }

            result.adv.rom_overrides[(size_t)RomLocation::ROM1] = reader.Arg();
        }
        else if (reader.Any("--override-rom2"))
        {
            if (!reader.Next())
            {
                return FE_ParseError::UnexpectedEnd;
            }

            result.adv.rom_overrides[(size_t)RomLocation::ROM2] = reader.Arg();
        }
        else if (reader.Any("--override-smrom"))
        {
            if (!reader.Next())
            {
                return FE_ParseError::UnexpectedEnd;
            }

            result.adv.rom_overrides[(size_t)RomLocation::SMROM] = reader.Arg();
        }
        else if (reader.Any("--override-waverom1"))
        {
            if (!reader.Next())
            {
                return FE_ParseError::UnexpectedEnd;
            }

            result.adv.rom_overrides[(size_t)RomLocation::WAVEROM1] = reader.Arg();
        }
        else if (reader.Any("--override-waverom2"))
        {
            if (!reader.Next())
            {
                return FE_ParseError::UnexpectedEnd;
            }

            result.adv.rom_overrides[(size_t)RomLocation::WAVEROM2] = reader.Arg();
        }
        else if (reader.Any("--override-waverom3"))
        {
            if (!reader.Next())
            {
                return FE_ParseError::UnexpectedEnd;
            }

            result.adv.rom_overrides[(size_t)RomLocation::WAVEROM3] = reader.Arg();
        }
        else if (reader.Any("--override-waverom-card"))
        {
            if (!reader.Next())
            {
                return FE_ParseError::UnexpectedEnd;
            }

            result.adv.rom_overrides[(size_t)RomLocation::WAVEROM_CARD] = reader.Arg();
        }
        else if (reader.Any("--override-waverom-exp"))
        {
            if (!reader.Next())
            {
                return FE_ParseError::UnexpectedEnd;
            }

            result.adv.rom_overrides[(size_t)RomLocation::WAVEROM_EXP] = reader.Arg();
        }
#if NUKED_ENABLE_ASIO
        else if (reader.Any("--asio-sample-rate"))
        {
            if (!reader.Next())
            {
                return FE_ParseError::UnexpectedEnd;
            }

            uint32_t asio_sample_rate = 0;
            if (!reader.TryParse(asio_sample_rate))
            {
                return FE_ParseError::ASIOSampleRateOutOfRange;
            }

            result.asio_sample_rate = asio_sample_rate;
        }
        else if (reader.Any("--asio-left-channel"))
        {
            if (!reader.Next())
            {
                return FE_ParseError::UnexpectedEnd;
            }

            result.asio_left_channel = reader.Arg();
        }
        else if (reader.Any("--asio-right-channel"))
        {
            if (!reader.Next())
            {
                return FE_ParseError::UnexpectedEnd;
            }

            result.asio_right_channel = reader.Arg();
        }
#endif
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
  -v, --version                                 Display version information.

Audio options:
  -p, --port         <device_name_or_number>    Set MIDI input port.
  -a, --audio-device <device_name_or_number>    Set output audio device.
  -b, --buffer-size  <size>[:count]             Set buffer size, number of buffers.
  -f, --format       s16|s32|f32                Set output format.
  --disable-oversampling                        Halves output frequency.
  --gain <amount>                               Apply gain to the output.

Emulator options:
  -r, --reset     none|gs|gm                    Reset system in GS or GM mode.
  -n, --instances <count>                       Set number of emulator instances.
  --no-lcd                                      Run without LCDs.
  --nvram <filename>                            Saves and loads NVRAM to/from disk. JV-880 only.

ROM management options:
  -d, --rom-directory <dir>                     Sets the directory to load roms from.
  --romset <name>                               Sets the romset to load.
  --legacy-romset-detection                     Load roms using specific filenames like upstream.

)";

#if NUKED_ENABLE_ASIO
    constexpr const char* EXTRA_ASIO_STR = R"(ASIO options:
  --asio-sample-rate <freq>                     Request frequency from the ASIO driver.
  --asio-left-channel <channel_name_or_number>  Set left channel for ASIO output.
  --asio-right-channel <channel_name_or_number> Set right channel for ASIO output.

)";
#endif

    std::string name = common::GetProcessPath().stem().generic_string();
    fprintf(stderr, USAGE_STR, name.c_str());
    common::PrintRomsets(stderr);
#if NUKED_ENABLE_ASIO
    fprintf(stderr, EXTRA_ASIO_STR);
#endif
    MIDI_PrintDevices();
    FE_PrintAudioDevices();
}

int main(int argc, char* argv[])
{
    FE_Parameters params;
    FE_ParseError result = FE_ParseCommandLine(argc, argv, params);
    if (result != FE_ParseError::Success)
    {
        fprintf(stderr, "error: %s\n", FE_ParseErrorStr(result));
        FE_Usage();
        return 1;
    }

    if (params.help)
    {
        FE_Usage();
        return 0;
    }

    if (params.version)
    {
        // we'll explicitly use stdout for this - often tools want to parse
        // version information and we want to be able to support that use case
        // without requiring stream redirection
        Cfg_WriteVersionInfo(stdout);
        return 0;
    }

    FE_FixupParameters(params);

    if (!FE_GlobalInit())
    {
        fprintf(stderr, "FATAL ERROR: Failed to initialize global state\n");
        return 1;
    }

    // It is important that the application gets its own scope so it can be
    // destroyed before SDL is deinitialized. This is required for destructors
    // to be able to clean up SDL objects safely.
    {
        FE_Application frontend;

        if (!frontend.Initialize(params))
        {
            fprintf(stderr, "FATAL ERROR: Failed to initialize application\n");
            return 1;
        }

        frontend.Run();
    }

    FE_GlobalQuit();

    return 0;
}
