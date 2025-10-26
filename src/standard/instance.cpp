#include "instance.h"

#include <bit>

#include "audio_sdl.h"
#include "output_asio.h"
#include "output_sdl.h"

template <typename ElemT>
size_t CalcRingbufferSizeBytes(uint32_t buffer_size, uint32_t buffer_count)
{
    return std::bit_ceil<size_t>(1 + (size_t)buffer_size * (size_t)buffer_count * sizeof(ElemT));
}

Instance::~Instance()
{
#if NUKED_ENABLE_ASIO
    if (m_stream)
    {
        SDL_FreeAudioStream(m_stream);
        m_stream = nullptr;
    }
#endif
}

bool Instance::Initialize(const InstanceParameters& params)
{
    if (!params.romset_info)
    {
        fprintf(stderr, "FATAL: romset_info not provided to instance %02zu\n", params.instance_id);
        return false;
    }

    m_instance_id  = params.instance_id;
    m_format       = params.output_format;
    m_buffer_size  = params.buffer_size;
    m_buffer_count = params.buffer_count;
    m_gain         = params.gain;

    if (params.enable_lcd)
    {
        m_sdl_lcd = std::make_unique<LCD_SDL_Backend>();
    }

    std::filesystem::path this_nvram = params.nvram_filename;
    if (!this_nvram.empty())
    {
        // append instance number so that multiple instances don't clobber each other's nvram
        this_nvram += std::to_string(params.instance_id);
    }

    if (!m_emu.Init({.lcd_backend = m_sdl_lcd.get(), .nvram_filename = this_nvram}))
    {
        fprintf(stderr, "ERROR: Failed to init emulator.\n");
        return false;
    }

    if (!m_emu.LoadRoms(params.romset, *params.romset_info))
    {
        fprintf(stderr, "ERROR: Failed to load roms for instance %02zu\n", params.instance_id);
        return false;
    }

    m_emu.Reset();
    m_emu.GetPCM().enable_oversampling = params.enable_oversampling;

    if (!m_emu.StartLCD())
    {
        fprintf(stderr, "ERROR: Failed to start LCD.\n");
        return false;
    }

    return true;
}

template <typename SampleT>
void Instance::Prepare()
{
    auto span     = m_view.UncheckedPrepareWrite<AudioFrame<SampleT>>(m_buffer_size);
    m_chunk_first = span.data();
    m_chunk_last  = span.data() + span.size();
}

template <typename SampleT>
void Instance::Finish()
{
    m_view.UncheckedFinishWrite<AudioFrame<SampleT>>(m_buffer_size);
}

template <typename SampleT>
void Instance::CreateAndPrepareBuffer()
{
    m_sample_buffer.Init(CalcRingbufferSizeBytes<AudioFrame<SampleT>>(m_buffer_size, m_buffer_count));
    m_view = RingbufferView(m_sample_buffer);
    Prepare<SampleT>();
}

#if NUKED_ENABLE_ASIO
void Instance::RunInstanceASIO(Instance& self)
{
    while (self.m_running)
    {
        // we recalc every time because ASIO reset might change this
        const size_t buffer_size = (size_t)Out_ASIO_GetBufferSize();

        // note that this is the byte count coming out of the stream; it won't line up with the amount of data we
        // put in so be careful not to confuse the two!!
        const size_t max_byte_count = self.m_buffer_count * buffer_size * Out_ASIO_GetFormatFrameSizeBytes();

        while ((size_t)SDL_AudioStreamAvailable(self.m_stream) >= max_byte_count)
        {
            SDL_Delay(1);
        }

        self.m_emu.Step();
    }
}

template <typename SampleT, bool ApplyGain>
void Instance::ReceiveSampleASIO(void* userdata, const AudioFrame<int32_t>& in)
{
    Instance& inst = *(Instance*)userdata;

    AudioFrame<SampleT>* out = (AudioFrame<SampleT>*)inst.m_chunk_first;
    Normalize(in, *out);

    if constexpr (ApplyGain)
    {
        Scale(*out, inst.m_gain);
    }

    inst.m_chunk_first = out + 1;

    if (inst.m_chunk_first == inst.m_chunk_last)
    {
        inst.Finish<SampleT>();
        inst.Prepare<SampleT>();

        auto span = inst.m_view.UncheckedPrepareRead<AudioFrame<SampleT>>(inst.m_buffer_size);
        SDL_AudioStreamPut(inst.m_stream, span.data(), (int)(span.size() * sizeof(AudioFrame<SampleT>)));
        inst.m_view.UncheckedFinishRead<AudioFrame<SampleT>>(inst.m_buffer_size);
    }
}
#endif

template <typename SampleT>
void Instance::RunInstanceSDL(Instance& self)
{
    const size_t max_byte_count = self.m_buffer_count * self.m_buffer_size * sizeof(AudioFrame<SampleT>);

    while (self.m_running)
    {
        while (self.m_view.GetReadableBytes() >= max_byte_count)
        {
            SDL_Delay(1);
        }

        self.m_emu.Step();
    }
}

template <typename SampleT, bool ApplyGain>
void Instance::ReceiveSampleSDL(void* userdata, const AudioFrame<int32_t>& in)
{
    Instance& fe = *(Instance*)userdata;

    AudioFrame<SampleT>* out = (AudioFrame<SampleT>*)fe.m_chunk_first;
    Normalize(in, *out);

    if constexpr (ApplyGain)
    {
        Scale(*out, fe.m_gain);
    }

    fe.m_chunk_first = out + 1;

    if (fe.m_chunk_first == fe.m_chunk_last)
    {
        fe.Finish<SampleT>();
        fe.Prepare<SampleT>();
    }
}

mcu_sample_callback Instance::PickSampleCallback(AudioOutputKind kind) const
{
    if (kind == AudioOutputKind::SDL)
    {
        if (m_gain != 1.f)
        {
            switch (m_format)
            {
            case AudioFormat::S16:
                return ReceiveSampleSDL<int16_t, true>;
            case AudioFormat::S32:
                return ReceiveSampleSDL<int32_t, true>;
            case AudioFormat::F32:
                return ReceiveSampleSDL<float, true>;
            }
        }
        else
        {
            switch (m_format)
            {
            case AudioFormat::S16:
                return ReceiveSampleSDL<int16_t, false>;
            case AudioFormat::S32:
                return ReceiveSampleSDL<int32_t, false>;
            case AudioFormat::F32:
                return ReceiveSampleSDL<float, false>;
            }
        }
    }
    else
    {
#if NUKED_ENABLE_ASIO
        if (m_gain != 1.f)
        {
            switch (m_format)
            {
            case AudioFormat::S16:
                return ReceiveSampleASIO<int16_t, true>;
            case AudioFormat::S32:
                return ReceiveSampleASIO<int32_t, true>;
            case AudioFormat::F32:
                return ReceiveSampleASIO<float, true>;
            }
        }
        else
        {
            switch (m_format)
            {
            case AudioFormat::S16:
                return ReceiveSampleASIO<int16_t, false>;
            case AudioFormat::S32:
                return ReceiveSampleASIO<int32_t, false>;
            case AudioFormat::F32:
                return ReceiveSampleASIO<float, false>;
            }
        }
#else
        fprintf(stderr, "PANIC: Instance::PickSampleCallback tried to select ASIO output without ASIO support\n");
        std::abort();
#endif
    }

    fprintf(stderr, "output kind = %d\n", (int)kind);
    fprintf(stderr, "gain = %f\n", m_gain);
    fprintf(stderr, "format = %d\n", (int)m_format);
    return nullptr;
}

void Instance::OpenSDLAudio()
{
    m_output_kind = AudioOutputKind::SDL;
    m_emu.SetSampleCallback(PickSampleCallback(m_output_kind), this);
    switch (m_format)
    {
    case AudioFormat::S16:
        CreateAndPrepareBuffer<int16_t>();
        break;
    case AudioFormat::S32:
        CreateAndPrepareBuffer<int32_t>();
        break;
    case AudioFormat::F32:
        CreateAndPrepareBuffer<float>();
        break;
    }
    Out_SDL_AddSource(m_view);
    fprintf(stderr, "#%02zu: allocated %zu bytes for audio\n", m_instance_id, m_sample_buffer.GetByteLength());
}

#if NUKED_ENABLE_ASIO
void Instance::OpenASIOAudio()
{
    m_stream = SDL_NewAudioStream(AudioFormatToSDLAudioFormat(m_format),
                                  2,
                                  (int)PCM_GetOutputFrequency(m_emu.GetPCM()),
                                  Out_ASIO_GetFormat(),
                                  2,
                                  Out_ASIO_GetFrequency());
    Out_ASIO_AddSource(m_stream);

    m_output_kind = AudioOutputKind::ASIO;
    m_emu.SetSampleCallback(PickSampleCallback(m_output_kind), this);

    switch (m_format)
    {
    case AudioFormat::S16:
        CreateAndPrepareBuffer<int16_t>();
        break;
    case AudioFormat::S32:
        CreateAndPrepareBuffer<int32_t>();
        break;
    case AudioFormat::F32:
        CreateAndPrepareBuffer<float>();
        break;
    }
    fprintf(stderr, "#%02zu: allocated %zu bytes for audio\n", m_instance_id, m_sample_buffer.GetByteLength());
}
#endif

void Instance::StartThread()
{
    m_running = true;
    if (m_output_kind == AudioOutputKind::SDL)
    {
        switch (m_format)
        {
        case AudioFormat::S16:
            m_thread = std::thread(RunInstanceSDL<int16_t>, std::ref(*this));
            break;
        case AudioFormat::S32:
            m_thread = std::thread(RunInstanceSDL<int32_t>, std::ref(*this));
            break;
        case AudioFormat::F32:
            m_thread = std::thread(RunInstanceSDL<float>, std::ref(*this));
            break;
        }
    }
    else if (m_output_kind == AudioOutputKind::ASIO)
    {
#if NUKED_ENABLE_ASIO
        m_thread = std::thread(RunInstanceASIO, std::ref(*this));
#else
        fprintf(stderr, "Attempted to start ASIO instance without ASIO support\n");
#endif
    }
}

void Instance::JoinThread()
{
    m_running = false;
    m_thread.join();
}

bool Instance::IsQuitRequested() const
{
    return m_sdl_lcd && m_sdl_lcd->IsQuitRequested();
}

void Instance::HandleEvent(const SDL_Event& ev)
{
    if (m_sdl_lcd)
    {
        m_sdl_lcd->HandleEvent(ev);
    }
}

void Instance::Render()
{
    LCD_Render(m_emu.GetLCD());
}
