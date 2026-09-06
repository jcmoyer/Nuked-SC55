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

#include <SDL.h>
#include <filesystem>
#include <optional>

#include "audio.h"
#include "bounded_vector.h"
#include "config.h"
#include "instance.h"
#include "midi.h"
#include "output_asio.h"
#include "output_sdl.h"

#include "common/rom_loader.h"

struct AdvancedCliParameters
{
    common::RomOverrides rom_overrides;
};

struct CliParameters
{
    // General options
    bool help    = false;
    bool version = false;

    // Audio options
    std::string midi_device;
    std::string audio_device;
    uint32_t    buffer_size          = 512;
    uint32_t    buffer_count         = 16;
    AudioFormat output_format        = AudioFormat::S16;
    bool        disable_oversampling = false;
    float       gain                 = 1.0f;

    // Emulator options
    std::optional<EMU_SystemReset> reset;
    size_t                         instances = 1;
    bool                           no_lcd    = false;
    std::filesystem::path          nvram_filename;

    // Rom management options
    std::optional<std::filesystem::path> rom_directory;
    std::string_view                     romset_name;
    common::RomLoader                    rom_loader = common::RomLoader::Hashing;

    // ASIO options
    std::optional<uint32_t> asio_sample_rate;
    std::string             asio_left_channel;
    std::string             asio_right_channel;

    // Advanced options
    AdvancedCliParameters adv;
};

enum class CliParseError
{
    Success,
    InstancesInvalid,
    InstancesOutOfRange,
    UnexpectedEnd,
    BufferSizeInvalid,
    BufferCountInvalid,
    UnknownArgument,
    RomDirectoryNotFound,
    FormatInvalid,
    ASIOSampleRateOutOfRange,
    ASIOChannelInvalid,
    ResetInvalid,
    GainInvalid,
};

CliParseError ParseCommandLine(int argc, char* argv[], CliParameters& result);
void          FixupParameters(CliParameters& params);
const char*   ParseErrorStr(CliParseError err);

class Application : private MIDI_Output
{
public:
    Application() = default;
    ~Application();

    Application(const Application&)            = delete;
    Application& operator=(const Application&) = delete;

    Application(Application&&)            = delete;
    Application& operator=(Application&&) = delete;

    bool Initialize(const CliParameters& params);

    void Run();

    void SendMIDI(size_t n, std::span<const uint8_t> bytes);
    void BroadcastMIDI(std::span<const uint8_t> bytes);
    void RouteMIDI(std::span<const uint8_t> bytes);

private:
    bool AllocateInstance(Instance** result);
    bool CreateInstance(const CliParameters& params);

    void RunEventLoop();
    bool HandleGlobalEvent(const SDL_Event& ev);

    bool OpenSDLAudio(const AudioOutputParameters& params, const char* device_name);
#if NUKED_ENABLE_ASIO
    bool OpenASIOAudio(const ASIO_OutputParameters& params, const char* name);
#endif
    bool OpenAudio(const CliParameters& params);

    // MIDI_Output interface
    void Write(std::span<const uint8_t> bytes) override
    {
        RouteMIDI(bytes);
    }

private:
    static const size_t MAX_INSTANCES = 16;

    BoundedVector<Instance, MAX_INSTANCES> m_instances;

    RomsetInfo m_romset_info;
    Romset     m_romset;

    AudioOutput m_audio_output{};

    bool m_running = false;
};
