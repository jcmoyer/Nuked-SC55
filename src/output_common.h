#pragma once

#include <cstdint>
#include <string>
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
