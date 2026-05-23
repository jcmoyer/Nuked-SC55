#pragma once

#include "mcu.h"

// Reads data starting from the mcu's PC and presents the data as a stream.
class CodeReader
{
public:
    CodeReader() = default;

    CodeReader(mcu_t& mcu)
        : m_mcu(&mcu)
    {
    }

    uint8_t ReadU8()
    {
        uint8_t result = MCU_ReadCodeOffset(*m_mcu, m_offset);
        ++m_offset;
        return result;
    }

    uint16_t ReadU16()
    {
        uint16_t result = ReadU8();
        result          = static_cast<uint16_t>((result << 8) | ReadU8());
        return result;
    }

    uint16_t GetAddressInPage() const
    {
        return static_cast<uint16_t>(m_mcu->pc + m_offset);
    }

    mcu_t& GetMCU()
    {
        return *m_mcu;
    }

private:
    mcu_t*  m_mcu    = nullptr;
    uint8_t m_offset = 0;
};
