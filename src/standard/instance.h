#pragma once

#include <cstddef>
#include <filesystem>
#include <thread>

#include "emu.h"
#include "lcd_sdl.h"
#include "output_common.h"
#include "ringbuffer.h"

#include "config.h"

struct InstanceParameters
{
    size_t          instance_id;
    AudioFormat     output_format;
    uint32_t        buffer_size;
    uint32_t        buffer_count;
    float           gain;
    bool            enable_lcd;
    bool            enable_oversampling;
    AudioOutputKind output_kind;

    std::filesystem::path nvram_filename;

    const AllRomsetInfo* romset_info;
    Romset               romset;
};

class Instance
{
public:
    Instance() = default;
    ~Instance();

    Instance(const Instance&)            = delete;
    Instance& operator=(const Instance&) = delete;
    Instance(Instance&&)                 = delete;
    Instance& operator=(Instance&&)      = delete;

    bool Initialize(const InstanceParameters& params);

    Emulator& GetEmulator()
    {
        return m_emu;
    }

    void OpenSDLAudio();

#if NUKED_ENABLE_ASIO
    void OpenASIOAudio();
#endif

    void StartThread();
    void JoinThread();

    bool IsQuitRequested() const;
    void HandleEvent(const SDL_Event& ev);
    void Render();

private:
    template <typename SampleT>
    void Prepare();

    template <typename SampleT>
    void Finish();

    template <typename SampleT>
    void CreateAndPrepareBuffer();

    mcu_sample_callback PickSampleCallback(AudioOutputKind kind) const;

    template <typename SampleT>
    static void RunInstanceSDL(Instance& self);

    template <typename SampleT, bool ApplyGain>
    static void ReceiveSampleSDL(void* userdata, const AudioFrame<int32_t>& in);

#if NUKED_ENABLE_ASIO
    static void RunInstanceASIO(Instance& self);

    template <typename SampleT, bool ApplyGain>
    static void ReceiveSampleASIO(void* userdata, const AudioFrame<int32_t>& in);
#endif

private:
    Emulator        m_emu;
    size_t          m_instance_id;
    AudioOutputKind m_output_kind;

    std::unique_ptr<LCD_SDL_Backend> m_sdl_lcd;

    GenericBuffer  m_sample_buffer;
    RingbufferView m_view;
    void*          m_chunk_first = nullptr;
    void*          m_chunk_last  = nullptr;

    std::thread m_thread;
    AudioFormat m_format;

    // read by instance thread, written by main thread
    std::atomic<bool> m_running = false;

    uint32_t m_buffer_size;
    uint32_t m_buffer_count;

    float m_gain = 1.0f;

#if NUKED_ENABLE_ASIO
    // ASIO uses an SDL_AudioStream because it needs resampling to a more conventional frequency, but putting data into
    // the stream one frame at a time is *slow* so we buffer audio in `sample_buffer` and add it all at once.
    SDL_AudioStream* stream = nullptr;
#endif
};
