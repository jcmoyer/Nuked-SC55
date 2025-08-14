#include "output_sdl.h"

#include "audio_sdl.h"
#include "bounded_vector.h"
#include "cast.h"
#include <SDL.h>

// one per instance
const size_t MAX_STREAMS = 16;

struct SDLOutput
{
    SDL_AudioSpec requested_spec{};
    SDL_AudioSpec actual_spec{};

    SDL_AudioDeviceID device = 0;

    BoundedVector<RingbufferView*, MAX_STREAMS> views;

    // Parameters requested by the user
    AudioOutputParameters create_params;
};

static SDLOutput g_output;

template <typename SampleT>
void AudioCallback(void* userdata, Uint8* stream, int len)
{
    (void)userdata;

    using Frame = AudioFrame<SampleT>;

    memset(stream, 0, (size_t)len);

    for (RingbufferView* view : g_output.views)
    {
        if (view->GetReadableElements<Frame>() >= g_output.create_params.buffer_size)
        {
            auto span = view->UncheckedPrepareRead<Frame>(g_output.create_params.buffer_size);
            for (size_t samp = 0; samp < span.size(); ++samp)
            {
                MixFrame(*((Frame*)stream + samp), span[samp]);
            }
            view->UncheckedFinishRead<Frame>(g_output.create_params.buffer_size);
        }
    }
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

bool Out_SDL_Create(const char* device_name, const AudioOutputParameters& params)
{
    if (SDL_InitSubSystem(SDL_INIT_AUDIO) != 0)
    {
        fprintf(stderr, "Failed to initialize audio: %s\n", SDL_GetError());
        return false;
    }

    SDL_AudioSpec spec{};
    SDL_AudioSpec spec_actual{};

    spec.freq     = RangeCast<int>(params.frequency);
    spec.channels = 2;
    spec.userdata = nullptr;
    spec.samples  = RangeCast<Uint16>(params.buffer_size);

    switch (params.format)
    {
    case AudioFormat::S16:
        spec.format   = AUDIO_S16SYS;
        spec.callback = AudioCallback<int16_t>;
        break;
    case AudioFormat::S32:
        spec.format   = AUDIO_S32SYS;
        spec.callback = AudioCallback<int32_t>;
        break;
    case AudioFormat::F32:
        spec.format   = AUDIO_F32SYS;
        spec.callback = AudioCallback<float>;
        break;
    }

    g_output.device = SDL_OpenAudioDevice(device_name, 0, &spec, &spec_actual, 0);

    if (!g_output.device)
    {
        fprintf(stderr, "Failed to open audio device: %s\n", SDL_GetError());
        return false;
    }

    fprintf(stderr, "Audio device: %s\n", device_name ? device_name : "Default device (SDL)");

    fprintf(stderr,
            "Audio requested: format=%s, channels=%d, frequency=%d, frames=%d\n",
            SDLAudioFormatToString(spec.format),
            spec.channels,
            spec.freq,
            spec.samples);

    fprintf(stderr,
            "Audio actual: format=%s, channels=%d, frequency=%d, frames=%d\n",
            SDLAudioFormatToString(spec_actual.format),
            spec_actual.channels,
            spec_actual.freq,
            spec_actual.samples);

    g_output.create_params  = params;
    g_output.requested_spec = spec;
    g_output.actual_spec    = spec_actual;

    return true;
}

void Out_SDL_Destroy()
{
    Out_SDL_Stop();
    SDL_CloseAudioDevice(g_output.device);
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
}

bool Out_SDL_Start()
{
    SDL_PauseAudioDevice(g_output.device, 0);

    return true;
}

void Out_SDL_Stop()
{
    SDL_PauseAudioDevice(g_output.device, 1);
}

void Out_SDL_AddSource(RingbufferView& view)
{
    g_output.views.EmplaceBack(&view);
}
