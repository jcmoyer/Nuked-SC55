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

#include <cmath>
#include <cstdint>
#include <type_traits>

template <typename T>
inline T Min(T a, T b)
{
    return a < b ? a : b;
}

template <typename T>
inline T Clamp(T value, T min, T max)
{
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

template <typename T>
    requires std::is_signed_v<T>
inline T PickCloser(T to, T a, T b)
{
    if (std::abs(to - a) < std::abs(to - b))
    {
        return a;
    }
    else
    {
        return b;
    }
}

inline size_t NDigits(int32_t n)
{
    if (n < 0)
    {
        if (n == INT32_MIN)
        {
            return 11;
        }
        else
        {
            return 1 + NDigits(-n);
        }
    }
    else if (n < 10)
    {
        return 1;
    }
    else if (n < 100)
    {
        return 2;
    }
    else if (n < 1000)
    {
        return 3;
    }
    else if (n < 10000)
    {
        return 4;
    }
    else if (n < 100000)
    {
        return 5;
    }
    else if (n < 1000000)
    {
        return 6;
    }
    else if (n < 10000000)
    {
        return 7;
    }
    else if (n < 100000000)
    {
        return 8;
    }
    else if (n < 1000000000)
    {
        return 9;
    }
    else
    {
        return 10;
    }
}

inline int16_t SaturatingAdd(int16_t a, int16_t b)
{
    int32_t result = (int32_t)a + (int32_t)b;
    return (int16_t)Clamp<int32_t>(result, INT16_MIN, INT16_MAX);
}

inline int32_t SaturatingAdd(int32_t a, int32_t b)
{
    int64_t result = (int64_t)a + (int64_t)b;
    return (int32_t)Clamp<int64_t>(result, INT32_MIN, INT32_MAX);
}

inline int16_t SaturatingMul(int16_t a, float b)
{
    int32_t result = (int32_t)((float)a * b);
    return (int16_t)Clamp<int32_t>(result, INT16_MIN, INT16_MAX);
}

inline int32_t SaturatingMul(int32_t a, float b)
{
    int64_t result = (int64_t)((float)a * b);
    return (int32_t)Clamp<int64_t>(result, INT32_MIN, INT32_MAX);
}

// Auto vectorizes in clang at -O2, gcc at -O3
inline void HorizontalSatAddI16(int16_t* dest, int16_t* src_first, int16_t* src_last)
{
    while (src_first != src_last)
    {
        *dest = SaturatingAdd(*dest, *src_first);
        ++src_first;
        ++dest;
    }
}

inline void HorizontalSatAddI32(int32_t* dest, int32_t* src_first, int32_t* src_last)
{
    while (src_first != src_last)
    {
        *dest = SaturatingAdd(*dest, *src_first);
        ++src_first;
        ++dest;
    }
}

inline void HorizontalAddF32(float* dest, float* src_first, float* src_last)
{
    while (src_first != src_last)
    {
        *dest = *dest + *src_first;
        ++src_first;
        ++dest;
    }
}
