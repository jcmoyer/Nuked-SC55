#pragma once

#include <array>
#include <cstdint>
#include <memory>

struct mcu_t;

// Contains all of the parameters for a fully decoded instruction.
struct I_CachedInstruction
{
    union {
        uint16_t ea_data; // used by addressing modes that include generic data in the EA field
        int16_t  ea_disp; // used by addressing modes that include a displacement in the EA field
        uint16_t br_true; // only used for short form branch instructions
    };

    union {
        uint8_t op_reg; // used by instructions that encode a general purpose register as part of the opcode
        uint8_t op_c;   // used by instructions that encode a control register as part of the opcode
    };

    uint8_t ea_reg; // used by addressing modes that refer to a register

    union {
        uint16_t op_data;  // used by instructions that have immediate data
        uint16_t br_false; // only used for short form branch instructions
    };
};

using I_Handler_Erased_Func = void (*)(mcu_t&, const I_CachedInstruction&);

struct I_Handler
{
    I_Handler_Erased_Func F;
    I_CachedInstruction   instr;
};

class I_InstructionCache
{
private:
    // 16 pages of 64K, TODO determine upper bound (not all pages contain code)
    using ArrayType = std::array<I_Handler, static_cast<size_t>(16 * 0x10000)>;

public:
    I_InstructionCache();

    const I_Handler& Lookup(uint32_t addr) const
    {
        return (*m_cache)[addr];
    }

    void DoCache(mcu_t& mcu, uint32_t instr_start, I_Handler_Erased_Func func, const I_CachedInstruction& st);

    void DoCacheJump(mcu_t& mcu, uint32_t instr_start, I_Handler_Erased_Func func, int16_t disp);

    void DoCacheBranch(mcu_t& mcu, uint32_t instr_start, I_Handler_Erased_Func func, int16_t disp);

    size_t CountCached() const;

private:
    std::unique_ptr<ArrayType> m_cache;
};
