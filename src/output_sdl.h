#pragma once

#include "output_common.h"
#include "ringbuffer.h"

#include <SDL.h>

bool Out_SDL_QueryOutputs(AudioOutputList& list);

bool Out_SDL_Start(const char* device_name);
void Out_SDL_Stop();

// TODO: this is gross and temporary
void Out_SDL_AddStream(RingbufferView<AudioFrame<int16_t>>* view);
void Out_SDL_AddStream(RingbufferView<AudioFrame<int32_t>>* view);
void Out_SDL_AddStream(RingbufferView<AudioFrame<float>>* view);

void Out_SDL_SetFrequency(int frequency);
void Out_SDL_SetBufferSize(int size_frames);
void Out_SDL_SetFormat(AudioFormat format);
