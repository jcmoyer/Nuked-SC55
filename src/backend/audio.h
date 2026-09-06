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

#include "math_util.h"
#include <cstddef>
#include <cstdint>

enum class AudioFormat
{
    S16,
    S32,
    F32,
};

template <typename T>
struct AudioFrame
{
    T left;
    T right;

    static constexpr size_t channel_count = 2;
};

inline void Normalize(const AudioFrame<int32_t>& in, AudioFrame<int16_t>& out)
{
    out.left  = (int16_t)Clamp<int32_t>(in.left >> 15, INT16_MIN, INT16_MAX);
    out.right = (int16_t)Clamp<int32_t>(in.right >> 15, INT16_MIN, INT16_MAX);
}

inline void Normalize(const AudioFrame<int32_t>& in, AudioFrame<int32_t>& out)
{
    out.left  = (int32_t)Clamp<int64_t>((int64_t)in.left << 1, INT32_MIN, INT32_MAX);
    out.right = (int32_t)Clamp<int64_t>((int64_t)in.right << 1, INT32_MIN, INT32_MAX);
}

inline void Normalize(const AudioFrame<int32_t>& in, AudioFrame<float>& out)
{
    constexpr float DIV_REC = 1.0f / 536870912.0f;

    out.left  = (float)in.left * DIV_REC;
    out.right = (float)in.right * DIV_REC;
}

inline void MixFrame(AudioFrame<int16_t>& dest, const AudioFrame<int16_t>& src)
{
    dest.left  = SaturatingAdd(dest.left, src.left);
    dest.right = SaturatingAdd(dest.right, src.right);
}

inline void MixFrame(AudioFrame<int32_t>& dest, const AudioFrame<int32_t>& src)
{
    dest.left  = SaturatingAdd(dest.left, src.left);
    dest.right = SaturatingAdd(dest.right, src.right);
}

inline void MixFrame(AudioFrame<float>& dest, const AudioFrame<float>& src)
{
    dest.left  += src.left;
    dest.right += src.right;
}

template <typename SampleT>
void Scale(AudioFrame<SampleT>& frame, float scalar_gain)
{
    frame.left  = SaturatingMul(frame.left, scalar_gain);
    frame.right = SaturatingMul(frame.right, scalar_gain);
}

inline void Scale(AudioFrame<float>& frame, float scalar_gain)
{
    frame.left  = frame.left * scalar_gain;
    frame.right = frame.right * scalar_gain;
}
