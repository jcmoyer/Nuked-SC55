#include "output_sdl.h"

#include <SDL.h>

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
