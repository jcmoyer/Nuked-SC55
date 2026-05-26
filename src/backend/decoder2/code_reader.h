#pragma once

#include "mcu.h"

namespace decoder2 {

enum class ReadError
{
    // a read crossed a page boundary
    OutOfPageBounds = 1,
};

inline const char* ToCString(ReadError err)
{
    switch (err)
    {
    case ReadError::OutOfPageBounds:
        return "attempt to read beyond page bounds";
    }
    return "unknown error";
}

// Reads data starting from the mcu's PC and presents the data as a stream.
class CodeReader
{
public:
    CodeReader() = default;

    CodeReader(mcu_t& mcu)
        : m_mcu(&mcu)
    {
    }

    void SetReadAddress(uint8_t page, uint16_t addr)
    {
        m_page      = page;
        m_read_addr = static_cast<uint32_t>((m_page << 16) | addr);
    }

    uint32_t GetReadAddress()
    {
        return m_read_addr;
    }

    ReadError ReadU8(uint8_t& byte)
    {
        if (GetReadPage() != m_page)
        {
            return ReadError::OutOfPageBounds;
        }
        byte = MCU_Read(*m_mcu, m_read_addr);
        ++m_read_addr;
        return ReadError{};
    }

    ReadError ReadS8(int8_t& byte)
    {
        ReadError err;
        uint8_t   ubyte;

        err  = ReadU8(ubyte);
        byte = (int8_t)ubyte;
        return err;
    }

    ReadError ReadU16(uint16_t& word)
    {
        ReadError err;
        uint8_t   byte;

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

    ReadError ReadS16(int16_t& word)
    {
        ReadError err;
        uint16_t  uword;

        err  = ReadU16(uword);
        word = (int16_t)uword;
        return err;
    }

    uint16_t GetAddressInPage() const
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
