#pragma once

#include <array>
#include <cstdint>
#include <memory>

struct mcu_t;

namespace decoder2
{

// Contains all of the parameters for a fully decoded instruction.
struct DecodedInstructionParams
{
    union {
        uint16_t ea_data; // used by addressing modes that include generic data in the EA field
        int16_t  ea_disp; // used by addressing modes that include a displacement in the EA field
        uint16_t br_true; // only used for short form branch instructions
    };

    union {
        uint8_t op_reg;  // used by instructions that encode a general purpose register as part of the opcode
        uint8_t op_c;    // used by instructions that encode a control register as part of the opcode
        uint8_t op_page; // used by instructions that encode an immediate page in addition to immediate address
    };

    uint8_t ea_reg; // used by addressing modes that refer to a register

    union {
        uint16_t op_data;  // used by instructions that have immediate data
        uint16_t br_false; // only used for short form branch instructions
    };
};

using CachedInstructionHandler = void (*)(mcu_t&, const DecodedInstructionParams&);

struct CachedInstruction
{
    CachedInstructionHandler handler;
    DecodedInstructionParams params;
};

class InstructionCache
{
private:
    // 16 pages of 64K, TODO determine upper bound (not all pages contain code)
    using ArrayType = std::array<CachedInstruction, static_cast<size_t>(16 * 0x10000)>;

public:
    InstructionCache();

    const CachedInstruction& Lookup(uint32_t addr) const
    {
        return (*m_cache)[addr];
    }

    bool Contains(uint32_t addr) const
    {
        return Lookup(addr).handler;
    }

    void Write(uint32_t addr, CachedInstruction handler)
    {
        (*m_cache)[addr] = handler;
    }

    size_t CountCached() const;

private:
    std::unique_ptr<ArrayType> m_cache;
};

} // namespace decoder2
