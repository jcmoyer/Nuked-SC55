#pragma once

#include "output_common.h"

bool Out_SDL_QueryOutputs(AudioOutputList& list);
bool Out_SDL_Start(const char* driver_name);

