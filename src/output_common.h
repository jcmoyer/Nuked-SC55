#pragma once

#include <string>
#include <vector>

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

