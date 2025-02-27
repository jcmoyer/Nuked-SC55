#include "output_sdl.h"

#include "audio.h"
#include "cast.h"
#include <SDL.h>
#include <cinttypes>

// one per instance
const size_t MAX_STREAMS = 16;

// TODO all this is temporary
static Uint16            g_buffer_size = 0;
static int               g_frequency   = 0;
static SDL_AudioDeviceID g_device      = 0;
static SDL_AudioFormat   g_format      = 0;
static SDL_AudioCallback g_callback    = 0;

static RingbufferView* g_views[MAX_STREAMS];
static size_t          g_stream_count = 0;

template <typename SampleT>
void AudioCallback(void* userdata, Uint8* stream, int len)
{
    (void)userdata;

    using Frame = AudioFrame<SampleT>;

    memset(stream, 0, (size_t)len);

    for (size_t i = 0; i < g_stream_count; ++i)
    {
        if (g_views[i]->GetReadableElements<Frame>() >= g_buffer_size)
        {
            auto span = g_views[i]->UncheckedPrepareRead<Frame>(g_buffer_size);
            for (size_t samp = 0; samp < span.size(); ++samp)
            {
                MixFrame(*((Frame*)stream + samp), span[samp]);
            }
            g_views[i]->UncheckedFinishRead<Frame>(g_buffer_size);
        }
    }
}

static const char* AudioFormatToString(SDL_AudioFormat format)
{
    switch (format)
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
    return "Unknown";
}

bool Out_SDL_QueryOutputs(AudioOutputList& list)
{
    if (SDL_InitSubSystem(SDL_INIT_AUDIO) != 0)
    {
        return false;
    }

    const int num_audio_devs = SDL_GetNumAudioDevices(0);

    for (int i = 0; i < num_audio_devs; ++i)
    {
        list.push_back({.name = SDL_GetAudioDeviceName(i, 0), .kind = AudioOutputKind::SDL});
    }

    SDL_QuitSubSystem(SDL_INIT_AUDIO);

    return true;
}

bool Out_SDL_Start(const char* device_name)
{
    if (SDL_InitSubSystem(SDL_INIT_AUDIO) != 0)
    {
        fprintf(stderr, "Failed to initialize audio: %s\n", SDL_GetError());
        return false;
    }

    SDL_AudioSpec spec        = {};
    SDL_AudioSpec spec_actual = {};

    spec.format   = g_format;
    spec.callback = g_callback;
    spec.freq     = g_frequency;
    spec.channels = 2;
    spec.userdata = nullptr;
    spec.samples  = g_buffer_size;

    g_device = SDL_OpenAudioDevice(device_name, 0, &spec, &spec_actual, 0);

    if (!g_device)
    {
        fprintf(stderr, "Failed to open audio device: %s\n", SDL_GetError());
        return false;
    }

    fprintf(stderr, "Audio device: %s\n", device_name ? device_name : "Default device (SDL)");

    fprintf(stderr,
            "Audio requested: Format=%s, Channels=%d, Frequency=%d, Buffer=%d\n",
            AudioFormatToString(spec.format),
            spec.channels,
            spec.freq,
            spec.samples);

    fprintf(stderr,
            "Audio actual: Format=%s, Channels=%d, Frequency=%d, Buffer=%d\n",
            AudioFormatToString(spec_actual.format),
            spec_actual.channels,
            spec_actual.freq,
            spec_actual.samples);

    SDL_PauseAudioDevice(g_device, 0);

    return true;
}

void Out_SDL_Stop()
{
    SDL_CloseAudioDevice(g_device);
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
}

void Out_SDL_AddStream(RingbufferView* view)
{
    if (g_stream_count == MAX_STREAMS)
    {
        fprintf(stderr, "PANIC: attempted to add more than %" PRIu64 " SDL streams\n", MAX_STREAMS);
        exit(1);
    }

    g_views[g_stream_count] = view;

    ++g_stream_count;
}

void Out_SDL_SetFrequency(int frequency)
{
    g_frequency = frequency;
}

void Out_SDL_SetBufferSize(int size_frames)
{
    g_buffer_size = RangeCast<Uint16>(size_frames);
}

void Out_SDL_SetFormat(AudioFormat format)
{
    switch (format)
    {
    case AudioFormat::S16:
        g_format   = AUDIO_S16SYS;
        g_callback = AudioCallback<int16_t>;
        break;
    case AudioFormat::S32:
        g_format   = AUDIO_S32SYS;
        g_callback = AudioCallback<int32_t>;
        break;
    case AudioFormat::F32:
        g_format   = AUDIO_F32SYS;
        g_callback = AudioCallback<float>;
        break;
    }
}
