#pragma once

#include <type_traits>
#include <utility>

#include "decoder2/address_modes.h"
#include "decoder2/types.h"

#include "diagnostics.h"
#include "mcu.h"

namespace decoder2
{

// Implements pre/post decrement/increment for @-Rn and @Rn+ addressing modes
// and automatically adjusts the program counter on scope exit.
template <Size Sz, typename Mode>
class InstructionScope
{
public:
    InstructionScope(mcu_t& mcu, const InstructionParams& params, uint8_t instr_size)
        : m_mcu(mcu),
          m_params(params),
          m_instr_size(instr_size)
    {
        if constexpr (std::is_same_v<Mode, Mode_APreDecRn>)
        {
            m_mcu.r[m_params.ea_reg] -= GetAdjust();
        }
    }

    ~InstructionScope()
    {
        if constexpr (std::is_same_v<Mode, Mode_APostIncRn>)
        {
            m_mcu.r[m_params.ea_reg] += GetAdjust();
        }
        m_mcu.pc += static_cast<uint8_t>(ModeEASize(Mode{}) + m_instr_size);
    }

private:
    constexpr uint8_t GetAdjust() const
    {
        switch (Sz)
        {
        case Size::Byte:
            return (m_params.ea_reg == 7) ? 2 : 1;
        case Size::Word:
            return 2;
        }
    }

private:
    mcu_t&                   m_mcu;
    const InstructionParams& m_params;
    const uint8_t            m_instr_size;
};

constexpr uint8_t GetPageForRegister(const mcu_t& mcu, uint8_t Rn)
{
    switch (Rn)
    {
    case 0:
    case 1:
    case 2:
    case 3:
        return mcu.dp;
    case 4:
    case 5:
        return mcu.ep;
    case 6:
    case 7:
        return mcu.tp;
    default:
        std::unreachable();
    }
}

///////////////////////////////////////////////////////////////////////////////
// Implements sized stores/loads to/from mcu registers.
template <Size Sz>
void StoreToReg(mcu_t& mcu, uint8_t reg, SizeToIntType<Sz> value)
{
    if constexpr (Sz == Size::Byte)
        mcu.r[reg] = (mcu.r[reg] & 0xff00) | value;
    else if constexpr (Sz == Size::Word)
        mcu.r[reg] = value;
}

template <Size Sz>
SizeToIntType<Sz> LoadFromReg(mcu_t& mcu, uint8_t reg)
{
    if constexpr (Sz == Size::Byte)
        return (uint8_t)mcu.r[reg];
    else if constexpr (Sz == Size::Word)
        return mcu.r[reg];
}

///////////////////////////////////////////////////////////////////////////////
// Computes the EA pointer for a given addressing mode. This operation is only
// valid for modes that refer to an address in memory - the address of a
// register or immediate cannot be taken.
constexpr uint32_t ComputeEA(Mode_Ad8_Rn, mcu_t& mcu, uint8_t Rn, const InstructionParams& p)
{
    return (uint32_t)((GetPageForRegister(mcu, Rn) << 16) | (uint16_t)(mcu.r[Rn] + p.ea_disp));
}

constexpr uint32_t ComputeEA(Mode_Ad16_Rn, mcu_t& mcu, uint8_t Rn, const InstructionParams& p)
{
    return (uint32_t)((GetPageForRegister(mcu, Rn) << 16) | (uint16_t)(mcu.r[Rn] + p.ea_disp));
}

constexpr uint32_t ComputeEA(Mode_APreDecRn, mcu_t& mcu, uint8_t Rn, const InstructionParams& p)
{
    (void)p;
    return (uint32_t)((GetPageForRegister(mcu, Rn) << 16) | mcu.r[Rn]);
}

constexpr uint32_t ComputeEA(Mode_APostIncRn, mcu_t& mcu, uint8_t Rn, const InstructionParams& p)
{
    (void)p;
    return (uint32_t)((GetPageForRegister(mcu, Rn) << 16) | mcu.r[Rn]);
}

constexpr uint32_t ComputeEA(Mode_ARn, mcu_t& mcu, uint8_t Rn, const InstructionParams& p)
{
    (void)p;
    return (uint32_t)((GetPageForRegister(mcu, Rn) << 16) | mcu.r[Rn]);
}

constexpr uint32_t ComputeEA(Mode_Aaa8, const mcu_t& mcu, uint8_t Rn, const InstructionParams& p)
{
    (void)Rn;
    return (uint32_t)(mcu.br << 8) | p.ea_data;
}

constexpr uint32_t ComputeEA(Mode_Aaa16, const mcu_t& mcu, uint8_t Rn, const InstructionParams& p)
{
    (void)Rn;
    return (uint32_t)(mcu.dp << 16) | p.ea_data;
}

///////////////////////////////////////////////////////////////////////////////
// Performs a load from EA treating EA as if it were a pointer. This may load
// from a register or a memory location, but this function abstracts over the
// exact method. To obtain the pointer itself, use ComputeEA instead.
template <Size Sz>
auto LoadFromEA(Mode_Rn, mcu_t& mcu, const InstructionParams& instr)
{
    return LoadFromReg<Sz>(mcu, instr.ea_reg);
}

template <Size Sz>
    requires(Sz == Size::Byte)
uint8_t LoadFromEA(Mode_Imm8, mcu_t& mcu, const InstructionParams& instr)
{
    (void)mcu;
    return (uint8_t)instr.ea_data;
}

template <Size Sz>
    requires(Sz == Size::Word)
uint16_t LoadFromEA(Mode_Imm16, mcu_t& mcu, const InstructionParams& instr)
{
    (void)mcu;
    return instr.ea_data;
}

template <Size Sz, typename Mode>
SizeToIntType<Sz> LoadFromEA(Mode, mcu_t& mcu, const InstructionParams& st)
{
    (void)st;
    const uint32_t addr = ComputeEA(Mode{}, mcu, st.ea_reg, st);
    if constexpr (Sz == Size::Byte)
        return MCU_Read(mcu, addr);
    else if constexpr (Sz == Size::Word)
        return MCU_Read16(mcu, addr);
}

///////////////////////////////////////////////////////////////////////////////
// Performs a store to EA treating EA as if it were a pointer. This may store
// to a register or a memory location, but this function abstracts over the
// exact method. To obtain the pointer used for the store, use ComputeEA.
template <Size Sz>
void StoreToEA(Mode_Rn, mcu_t& mcu, const InstructionParams& st, SizeToIntType<Sz> value)
{
    StoreToReg<Sz>(mcu, st.ea_reg, value);
}

template <Size Sz, typename Mode>
    requires(!std::is_same_v<Mode, Mode_Rn>)
void StoreToEA(Mode, mcu_t& mcu, const InstructionParams& st, SizeToIntType<Sz> value)
{
    const uint32_t addr = ComputeEA(Mode{}, mcu, st.ea_reg, st);
    if constexpr (Sz == Size::Byte)
        MCU_Write(mcu, addr, value);
    else if constexpr (Sz == Size::Word)
        MCU_Write16(mcu, addr, value);
}

inline bool IsSR_B(uint8_t cr)
{
    return cr == 1;
}

inline bool IsSR_W(uint8_t cr)
{
    return cr == 0;
}

template <Size Sz>
inline bool IsSR(uint8_t cr)
{
    if constexpr (Sz == Size::Byte)
        return IsSR_B(cr);
    else if constexpr (Sz == Size::Word)
        return IsSR_W(cr);
}

inline uint8_t LoadFromCR_B(mcu_t& mcu, uint8_t cr)
{
    switch (cr)
    {
    case 0:
        break;
    case 1:
        return (uint8_t)mcu.sr;
    case 2:
        break;
    case 3:
        return mcu.br;
    case 4:
        return mcu.ep;
    case 5:
        return mcu.dp;
    case 6:
        break;
    case 7:
        return mcu.tp;
    }
    return 0;
}

inline uint16_t LoadFromCR_W(mcu_t& mcu, uint8_t cr)
{
    switch (cr)
    {
    case 0:
        return mcu.sr & sr_mask;
    case 1:
        break;
    case 2:
        break;
    case 3: // CR other than 0 "not allowed" but roms contain code that try to do this
        return (uint16_t)((LoadFromCR_B(mcu, cr) << 8) | LoadFromCR_B(mcu, cr));
    case 4:
        return (uint16_t)((LoadFromCR_B(mcu, cr) << 8) | LoadFromCR_B(mcu, cr));
    case 5:
        return (uint16_t)((LoadFromCR_B(mcu, cr) << 8) | LoadFromCR_B(mcu, cr));
    case 6:
        break;
    case 7:
        break;
    }
    return 0;
}

inline void StoreToCR_B(mcu_t& mcu, uint8_t cr, uint8_t value)
{
    switch (cr)
    {
    case 0:
        break;
    case 1:
        mcu.sr = (uint16_t)((mcu.sr & 0xff00) | value);
        break;
    case 2:
        break;
    case 3:
        mcu.br = value;
        break;
    case 4:
        mcu.ep = value;
        break;
    case 5:
        mcu.dp = value;
        break;
    case 6:
        break;
    case 7:
        mcu.tp = value;
        break;
    }
}

inline void StoreToCR_W(mcu_t& mcu, uint8_t cr, uint16_t value)
{
    switch (cr)
    {
    case 0:
        mcu.sr = value & sr_mask;
        return;
    case 1:
    case 2:
    case 3:
    case 4:
        break;
    case 5: // "not allowed" but roms contain code that try to do this
        StoreToCR_B(mcu, cr, (uint8_t)value);
        return;
    case 6:
    case 7:
        break;
    }
    Diag_Printf(Diag_Category::Debug, "I_WriteControlRegisterW: id (%d) not handled\n", cr);
}

template <Size Sz>
inline void StoreToCR(mcu_t& mcu, uint8_t cr, SizeToIntType<Sz> value)
{
    if constexpr (Sz == Size::Byte)
        StoreToCR_B(mcu, cr, value);
    else if constexpr (Sz == Size::Word)
        StoreToCR_W(mcu, cr, value);
}

template <Size Sz>
inline SizeToIntType<Sz> LoadFromCR(mcu_t& mcu, uint8_t cr)
{
    if constexpr (Sz == Size::Byte)
        return LoadFromCR_B(mcu, cr);
    else if constexpr (Sz == Size::Word)
        return LoadFromCR_W(mcu, cr);
}

template <Size Sz>
SizeToIntType<Sz> LoadFromOpData(mcu_t& mcu, const InstructionParams& st)
{
    (void)mcu;
    if constexpr (Sz == Size::Byte)
        return (uint8_t)st.op_data;
    else if constexpr (Sz == Size::Word)
        return st.op_data;
}

template <Size Sz>
void StoreToOpReg(mcu_t& mcu, const InstructionParams& st, SizeToIntType<Sz> value)
{
    StoreToReg<Sz>(mcu, st.op_reg, value);
}

template <Size Sz>
SizeToIntType<Sz> LoadFromOpReg(mcu_t& mcu, const InstructionParams& st)
{
    return LoadFromReg<Sz>(mcu, st.op_reg);
}

} // namespace decoder2
