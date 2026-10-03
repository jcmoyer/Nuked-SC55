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

#include "mcu.h"

namespace decoder2
{

enum class ReadError
{
    // a read crossed a page boundary
    OutOfPageBounds = 1,

    // a read is totally outside the bounds of the source buffer
    OutOfBounds,
};

inline const char* ToCString(ReadError err)
{
    switch (err)
    {
    case ReadError::OutOfPageBounds:
        return "attempt to read beyond page bounds";
    case ReadError::OutOfBounds:
        return "attempt to read out of bounds";
    }
    return "unknown error";
}

class CodeReader
{
public:
    virtual ReadError ReadU8(uint8_t& byte) = 0;

    virtual ReadError ReadS8(int8_t& byte)
    {
        ReadError err;
        uint8_t   ubyte = 0;

        err  = ReadU8(ubyte);
        byte = (int8_t)ubyte;
        return err;
    }

    virtual ReadError ReadU16(uint16_t& word)
    {
        ReadError err;
        uint8_t   byte = 0;

        err = ReadU8(byte);
        if (err != ReadError{})
        {
            return err;
        }
        word = byte;

        err = ReadU8(byte);
        if (err != ReadError{})
        {
            return err;
        }
        word = static_cast<uint16_t>((word << 8) | byte);

        return ReadError{};
    }

    virtual ReadError ReadS16(int16_t& word)
    {
        ReadError err;
        uint16_t  uword = 0;

        err  = ReadU16(uword);
        word = (int16_t)uword;
        return err;
    }

    virtual uint32_t GetReadAddress() const = 0;
    virtual uint16_t GetAddressInPage() const = 0;
};

class BufferReader : public CodeReader
{
public:
    BufferReader() = default;

    void SetView(std::span<uint8_t> view)
    {
        m_view = view;
    }

    ReadError SetOffset(size_t new_offset)
    {
        if (new_offset >= m_view.size())
        {
            return ReadError::OutOfBounds;
        }
        m_offset = new_offset;
        return ReadError{};
    }

    ReadError ReadU8(uint8_t& byte) override
    {
        if (m_offset >= m_view.size())
        {
            return ReadError::OutOfBounds;
        }
        byte = m_view[m_offset];
        ++m_offset;
        return ReadError{};
    }

    uint32_t GetReadAddress() const override
    {
        return (uint32_t)m_offset;
    }

    virtual uint16_t GetAddressInPage() const override
    {
        return (uint16_t)m_offset;
    }

private:
    std::span<uint8_t> m_view{};

    // offset into view, if this leaves the bounds of m_view future
    // read operations are errors
    size_t m_offset = 0;
};

// Reads data starting from the mcu's PC and presents the data as a stream.
class McuCodeReader : public CodeReader
{
public:
    McuCodeReader() = default;

    McuCodeReader(mcu_t& mcu)
        : m_mcu(&mcu)
    {
    }

    void SetReadAddress(uint8_t page, uint16_t addr)
    {
        m_page      = page;
        m_read_addr = static_cast<uint32_t>((m_page << 16) | addr);
    }

    uint32_t GetReadAddress() const override
    {
        return m_read_addr;
    }

    ReadError ReadU8(uint8_t& byte) override
    {
        if (GetReadPage() != m_page)
        {
            return ReadError::OutOfPageBounds;
        }
        byte = MCU_Read(*m_mcu, m_read_addr);
        ++m_read_addr;
        return ReadError{};
    }

    uint16_t GetAddressInPage() const override
    {
        return static_cast<uint16_t>(m_read_addr);
    }

private:
    uint8_t GetReadPage() const
    {
        return static_cast<uint8_t>(m_read_addr >> 16);
    }

private:
    mcu_t* m_mcu = nullptr;

    // page we expect to be reading from
    uint8_t m_page = 0;
    // wide address so that we can detect reading beyond the selected page
    uint32_t m_read_addr = 0;
};

} // namespace decoder2
