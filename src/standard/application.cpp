#include "application.h"

#include "common/gain.h"
#include "common/path_util.h"

Application::~Application()
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

bool Application::Initialize(const CliParameters& params)
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
        if (!CreateInstance(params))
        {
            fprintf(stderr, "FATAL ERROR: Failed to create instance %zu\n", i);
            return false;
        }
    }

    m_romset_info.PurgeRomData();

    for (Instance& inst : m_instances)
    {
        inst.GetEmulator().PostSystemReset(reset);
    }

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

    return true;
}

bool Application::AllocateInstance(Instance** result)
{
    if (m_instances.IsFull())
    {
        return false;
    }

    Instance& fe = m_instances.EmplaceBack();

    if (result)
    {
        *result = &fe;
    }

    return true;
}

void Application::SendMIDI(size_t instance_id, std::span<const uint8_t> bytes)
{
    m_instances[instance_id].GetEmulator().PostMIDI(bytes);
}

void Application::BroadcastMIDI(std::span<const uint8_t> bytes)
{
    for (size_t i = 0; i < m_instances.Count(); ++i)
    {
        SendMIDI(i, bytes);
    }
}

void Application::RouteMIDI(std::span<const uint8_t> bytes)
{
    if (bytes.size() == 0)
    {
        return;
    }

    uint8_t first = bytes[0];

    if (first < 0x80)
    {
        fprintf(stderr, "Application::RouteMIDI received data byte %02x\n", first);
        return;
    }

    const bool    is_sysex = first == 0xF0;
    const uint8_t channel  = first & 0x0F;

    if (is_sysex)
    {
        BroadcastMIDI(bytes);
    }
    else
    {
        SendMIDI(channel % m_instances.Count(), bytes);
    }
}

bool Application::OpenSDLAudio(const AudioOutputParameters& params, const char* device_name)
{
    if (!Out_SDL_Create(device_name, params))
    {
        fprintf(stderr, "Failed to create SDL audio output\n");
        return false;
    }

    for (size_t id = 0; id < m_instances.Count(); ++id)
    {
        Instance& inst = m_instances[id];
        inst.OpenSDLAudio();
    }

    if (!Out_SDL_Start())
    {
        fprintf(stderr, "Failed to start SDL audio output\n");
        return false;
    }

    return true;
}

#if NUKED_ENABLE_ASIO
bool Application::OpenASIOAudio(const ASIO_OutputParameters& params, const char* name)
{
    if (!Out_ASIO_Create(name, params))
    {
        fprintf(stderr, "Failed to create ASIO output\n");
        return false;
    }

    for (Instance& inst : m_instances)
    {
        inst.OpenASIOAudio();
    }

    if (!Out_ASIO_Start())
    {
        fprintf(stderr, "Failed to create ASIO output\n");
        return false;
    }

    return true;
}
#endif

void FixupParameters(CliParameters& params)
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

bool Application::OpenAudio(const CliParameters& params)
{
    AudioOutput      output;
    PickOutputResult output_result = PickOutputDevice(params.audio_device, output);

    m_audio_output = output;

    AudioOutputParameters out_params;
    out_params.frequency = PCM_GetOutputFrequency(m_instances[0].GetEmulator().GetPCM());
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
    case PickOutputResult::WantMatchedName:
        if (output.kind == AudioOutputKind::SDL)
        {
            return OpenSDLAudio(out_params, output.name.c_str());
        }
        else if (output.kind == AudioOutputKind::ASIO)
        {
#if NUKED_ENABLE_ASIO
            ASIO_OutputParameters asio_params;
            asio_params.common        = out_params;
            asio_params.left_channel  = params.asio_left_channel;
            asio_params.right_channel = params.asio_right_channel;
            return OpenASIOAudio(asio_params, output.name.c_str());
#else
            fprintf(stderr, "Attempted to open ASIO output without ASIO support\n");
#endif
        }
        return false;
    case PickOutputResult::WantDefaultDevice:
        return OpenSDLAudio(out_params, nullptr);
    case PickOutputResult::NoOutputDevices:
        // in some cases this may still work
        fprintf(stderr, "No output devices found; attempting to open default device\n");
        return OpenSDLAudio(out_params, nullptr);
    case PickOutputResult::NoMatchingName:
        // in some cases SDL cannot list all audio devices so we should still try
        fprintf(stderr, "No output device named '%s'; attempting to open it anyways...\n", params.audio_device.c_str());
        return OpenSDLAudio(out_params, output.name.c_str());
    }

    return false;
}

bool Application::HandleGlobalEvent(const SDL_Event& ev)
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

void Application::RunEventLoop()
{
    while (m_running)
    {
#if NUKED_ENABLE_ASIO
        if (Out_ASIO_IsResetRequested())
        {
            Out_ASIO_Reset();
        }
#endif

        for (Instance& inst : m_instances)
        {
            if (inst.IsQuitRequested())
            {
                m_running = false;
            }
            inst.Render();
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

            for (Instance& inst : m_instances)
            {
                inst.HandleEvent(ev);
            }
        }

        SDL_Delay(15);
    }
}

void Application::Run()
{
    m_running = true;

    for (Instance& inst : m_instances)
    {
        inst.StartThread();
    }

    RunEventLoop();

    for (Instance& inst : m_instances)
    {
        inst.JoinThread();
    }
}

bool Application::CreateInstance(const CliParameters& app_params)
{
    Instance* inst = nullptr;

    const size_t instance_id = m_instances.Count();

    if (!AllocateInstance(&inst))
    {
        fprintf(stderr, "ERROR: Failed to allocate instance.\n");
        return false;
    }

    InstanceParameters inst_params{
        .instance_id         = instance_id,
        .output_format       = app_params.output_format,
        .buffer_size         = app_params.buffer_size,
        .buffer_count        = app_params.buffer_count,
        .gain                = app_params.gain,
        .enable_lcd          = !app_params.no_lcd,
        .enable_oversampling = !app_params.disable_oversampling,
        .nvram_filename      = app_params.nvram_filename,
        .romset_info         = &m_romset_info,
        .romset              = m_romset,
    };

    if (!inst->Initialize(inst_params))
    {
        fprintf(stderr, "ERROR: Failed to initialize instance #%02zu.\n", instance_id);
        return false;
    }

    return true;
}
