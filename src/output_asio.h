#pragma once

#include "output_common.h"

#include <SDL.h>

bool Out_ASIO_QueryOutputs(AudioOutputList& list);

bool Out_ASIO_Start(const char* driver_name);
void Out_ASIO_Stop();

bool Out_ASIO_IsResetRequested();
void Out_ASIO_Reset();

// Adds a stream to be mixed into the ASIO output. It should not be freed until ASIO shuts down.
void Out_ASIO_AddStream(SDL_AudioStream* stream);

double          Out_ASIO_GetFrequency();
SDL_AudioFormat Out_ASIO_GetFormat();

// Returns the size of a single sample in bytes.
size_t Out_ASIO_GetFormatSampleSizeBytes();

void Out_ASIO_SetBufferSize(int size);
int  Out_ASIO_GetBufferSize();
