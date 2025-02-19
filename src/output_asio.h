#pragma once

#include "output_common.h"

bool Out_ASIO_QueryOutputs(AudioOutputList& list);
void Out_ASIO_Start(const char* driver_name);
