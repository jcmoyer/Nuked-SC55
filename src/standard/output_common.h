/*
 * Copyright (C) 2024-2026 J.C. Moyer
 *
 * This file is part of Nuked-SC55.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

#include "audio.h"

enum class AudioOutputKind
{
    SDL,
    ASIO,
};

struct AudioOutput
{
    std::string     name;
    AudioOutputKind kind;
};

using AudioOutputList = std::vector<AudioOutput>;

// Outputs should try to respect these if possible
struct AudioOutputParameters
{
    uint32_t    frequency;
    uint32_t    buffer_size;
    AudioFormat format;
};

enum class PickOutputResult
{
    WantMatchedName,
    WantDefaultDevice,
    NoOutputDevices,
    NoMatchingName,
};

void QueryAllOutputs(AudioOutputList& outputs);

PickOutputResult PickOutputDevice(std::string_view preferred_name, AudioOutput& out_device);

void PrintAudioDevices(FILE* output);
