#pragma once

#include <array>
#include <cstdint>
#include <memory>

#include "decoder2/instruction.h"

namespace decoder2
{

// Maps wide addresses (page:addr) to decoded instructions.
class InstructionCache
{
private:
    // Contains space for 16 pages of 64K, i.e. the entire memory space of the
    // mcu. Only 9 of the 16 pages contain code, but we allocate space for the
    // full 16 pages so that looking up a cached instruction is a simple array
    // index.
    //
    // Code is mapped to the following address spaces; ranges are inclusive:
    //
    // page   : addr
    // 0      : 0..0x7fff  => mcu.rom1
    // 1..4   : 0..0x7ffff => mcu.rom2
    // 8..9   : 0..0x7ffff => mcu.rom2 (only when !is_jv880)
    // 14..15 : 0..0x7ffff => mcu.rom2 (only when !is_jv880)
    using ArrayType = std::array<DecodedInstruction, static_cast<size_t>(16 * 0x10000)>;

public:
    InstructionCache();

    const DecodedInstruction& Lookup(uint32_t addr) const
    {
        return (*m_cache)[addr];
    }

    bool Contains(uint32_t addr) const
    {
        return Lookup(addr).handler;
    }

    void Write(uint32_t addr, DecodedInstruction handler)
    {
        (*m_cache)[addr] = handler;
    }

    size_t CountCached() const;

private:
    std::unique_ptr<ArrayType> m_cache;
};

} // namespace decoder2
