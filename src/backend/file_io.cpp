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

#include "file_io.h"

#include <fstream>

#include "cast.h"

bool FIO_ReadAllBytes(const std::filesystem::path& filename, std::vector<uint8_t>& buffer)
{
    std::ifstream input(filename, std::ios::binary);

    if (!input)
    {
        return false;
    }

    input.seekg(0, std::ios::end);
    std::streamoff byte_count = input.tellg();
    input.seekg(0, std::ios::beg);

    buffer.resize(RangeCast<size_t>(byte_count));

    input.read((char*)buffer.data(), RangeCast<std::streamsize>(byte_count));

    return input.good();
}

bool FIO_ReadStreamExact(std::ifstream& s, void* into, std::streamsize byte_count)
{
    if (s.read((char*)into, byte_count))
    {
        return s.gcount() == byte_count;
    }
    return false;
}

bool FIO_ReadStreamExact(std::ifstream& s, std::span<uint8_t> into, std::streamsize byte_count)
{
    return FIO_ReadStreamExact(s, into.data(), byte_count);
}

std::streamsize FIO_ReadStreamUpTo(std::ifstream& s, void* into, std::streamsize byte_count)
{
    s.read((char*)into, byte_count);
    return s.gcount();
}
