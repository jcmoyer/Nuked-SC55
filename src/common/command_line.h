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

#include <charconv>
#include <string_view>

namespace common
{

template <typename T>
bool TryParse(std::string_view s, T& n)
{
    std::from_chars_result r = std::from_chars(s.data(), s.data() + s.size(), n);
    if (r.ec != std::errc{})
    {
        return false;
    }
    return true;
}

class CommandLineReader
{
public:
    CommandLineReader(int argc, char* argv[])
        : m_argc(argc), m_argv(argv), m_index(0)
    {
    }

    bool Next()
    {
        ++m_index;
        if (m_index < m_argc)
        {
            m_arg = m_argv[m_index];
            return true;
        }
        else
        {
            return false;
        }
    }

    template <typename... Strings>
    bool Any(Strings... strings)
    {
        return ((strings == m_arg) || ...);
    }

    std::string_view Arg() const
    {
        return m_arg;
    }

    template <typename T>
    bool TryParse(T& output)
    {
        return common::TryParse(Arg(), output);
    }

private:
    int    m_argc;
    char** m_argv;
    int    m_index;
    std::string_view m_arg;
};

} // namespace common
