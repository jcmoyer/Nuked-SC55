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

// This is a very minimal WAVE writer. It only exists to output something other
// than raw sample data.

#pragma once

#include "audio.h"
#include <cstdint>
#include <cstdio>
#include <filesystem>

class WAV_Handle
{
public:
    WAV_Handle() = default;
    ~WAV_Handle();
    // moveable
    WAV_Handle(WAV_Handle&&) noexcept;
    WAV_Handle& operator=(WAV_Handle&&) noexcept;
    // noncopyable
    WAV_Handle(const WAV_Handle&) = delete;
    WAV_Handle& operator=(const WAV_Handle&) = delete;

    void SetSampleRate(uint32_t sample_rate);

    void OpenStdout(AudioFormat format);
    void Open(const char* filename, AudioFormat format);
    void Open(const std::filesystem::path& filename, AudioFormat format);
    void Close();
    void Write(const AudioFrame<int16_t>& frame);
    void Write(const AudioFrame<int32_t>& frame);
    void Write(const AudioFrame<float>& frame);
    void Finish();

private:
    FILE*       m_output = nullptr;
    uint64_t    m_frames_written = 0;
    AudioFormat m_format;
    uint32_t    m_sample_rate;
};
