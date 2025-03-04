#pragma once

#include "audio.h"
#include "output_common.h"
#include "ringbuffer.h"

bool Out_SDL_QueryOutputs(AudioOutputList& list);

bool Out_SDL_Create(const char* device_name);
void Out_SDL_Destroy();

bool Out_SDL_Start();
void Out_SDL_Stop();

void Out_SDL_AddStream(RingbufferView& view);

void Out_SDL_SetFrequency(int frequency);
void Out_SDL_SetBufferSize(int size_frames);
void Out_SDL_SetFormat(AudioFormat format);
