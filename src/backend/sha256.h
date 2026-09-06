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

#include <array>
#include <cstdint>
#include <cstring>
#include <functional>
#include <span>

extern "C"
{
#include "sha/sha.h"
}

using SHA256_Digest = std::array<uint8_t, 32>;

template <>
struct std::hash<SHA256_Digest>
{
    size_t operator()(const SHA256_Digest& digest) const
    {
        size_t result = 0;
        for (size_t i = 0; i < sizeof(SHA256_Digest) / sizeof(size_t); ++i)
        {
            size_t block;
            memcpy(&block, &digest[i * sizeof(size_t)], sizeof(size_t));
            result ^= block;
        }
        return result;
    }
};

namespace detail
{
consteval uint8_t HexValue(char x)
{
    if (x >= '0' && x <= '9')
    {
        return (uint8_t)(x - '0');
    }
    else if (x >= 'a' && x <= 'f')
    {
        return 10 + (uint8_t)(x - 'a');
    }
    else
    {
        throw "character out of range";
    }
}
} // namespace detail

// Compile time string-to-SHA256_Digest
template <size_t N>
consteval SHA256_Digest SHA256_ToDigest(const char (&s)[N])
{
    static_assert(N == 65); // 64 + null terminator

    SHA256_Digest hash;
    for (size_t i = 0; i < N / 2; ++i)
    {
        hash[i] = (uint8_t)((detail::HexValue(s[2 * i + 0]) << 4) | detail::HexValue(s[2 * i + 1]));
    }

    return hash;
}

inline bool SHA256_HashBytes(std::span<uint8_t> bytes, SHA256_Digest& out_digest)
{
    SHA256Context ctx;

    int err;

    err = SHA256Reset(&ctx);
    if (err != shaSuccess)
    {
        return false;
    }

    err = SHA256Input(&ctx, bytes.data(), (unsigned int)bytes.size());
    if (err != shaSuccess)
    {
        return false;
    }

    err = SHA256Result(&ctx, out_digest.data());
    if (err != shaSuccess)
    {
        return false;
    }

    return true;
}
