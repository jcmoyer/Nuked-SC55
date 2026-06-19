#pragma once

#include <cstdint>
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

template <Size Sz, typename State>
SizeToIntType<Sz> LoadFromOpData(mcu_t& mcu, const State& st)
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

// CMP:G.[B|W] <EAs>, Rd
template <Size Sz, typename State>
inline void I_CMP_G_EAs_Rd(mcu_t& mcu, const InstructionParams& st)
{
    InstructionScope<Sz, State> scope(mcu, st, 1);

    const SizeToIntType<Sz> rd  = LoadFromOpReg<Sz>(mcu, st);
    const SizeToIntType<Sz> eas = LoadFromEA<Sz>(State{}, mcu, st);

    const BinopResult<Sz> result = GenericSubtract<Sz>(rd, eas);

    MCU_SetStatus(mcu, result.negative, STATUS_N);
    MCU_SetStatus(mcu, result.zero, STATUS_Z);
    MCU_SetStatus(mcu, result.overflow, STATUS_V);
    MCU_SetStatus(mcu, result.carry, STATUS_C);
}

// CMP:G.[B|W] #xx:8, <EAd>
template <Size Sz, typename State>
inline void I_CMP_G_imm8_EAd(mcu_t& mcu, const InstructionParams& st)
{
    InstructionScope<Sz, State> scope(mcu, st, 2);

    const SizeToIntType<Sz> ea  = LoadFromEA<Sz>(State{}, mcu, st);
    SizeToIntType<Sz>       imm = LoadFromOpData<Size::Byte>(mcu, st);

    if constexpr (Sz == Size::Word)
    {
        // It's fine to truncate imm here because it came from a byte load. We
        // do this because the immediate is 8-bit, but the operation is Word
        // sized.
        imm = SX((uint8_t)imm);
    }

    const BinopResult<Sz> result = GenericSubtract<Sz>(ea, imm);

    MCU_SetStatus(mcu, result.negative, STATUS_N);
    MCU_SetStatus(mcu, result.zero, STATUS_Z);
    MCU_SetStatus(mcu, result.overflow, STATUS_V);
    MCU_SetStatus(mcu, result.carry, STATUS_C);
}

// CMP:G.[B|W] #xx:16, <EAd>
template <Size Sz, typename State>
inline void I_CMP_G_imm16_EAd(mcu_t& mcu, const InstructionParams& st)
{
    InstructionScope<Sz, State> scope(mcu, st, 3);

    const SizeToIntType<Sz> ea     = LoadFromEA<Sz>(State{}, mcu, st);
    const SizeToIntType<Sz> imm    = LoadFromOpData<Sz>(mcu, st);
    const BinopResult<Sz>   result = GenericSubtract<Sz>(ea, imm);

    MCU_SetStatus(mcu, result.negative, STATUS_N);
    MCU_SetStatus(mcu, result.zero, STATUS_Z);
    MCU_SetStatus(mcu, result.overflow, STATUS_V);
    MCU_SetStatus(mcu, result.carry, STATUS_C);
}

// CLR.[B|W] <EAd>
template <Size Sz, typename State>
inline void I_CLR_EAd(mcu_t& mcu, const InstructionParams& st)
{
    InstructionScope<Sz, State> scope(mcu, st, 1);

    StoreToEA<Sz>(State{}, mcu, st, 0);
    MCU_SetStatus(mcu, 0, STATUS_N);
    MCU_SetStatus(mcu, 1, STATUS_Z);
    MCU_SetStatus(mcu, 0, STATUS_V);
    MCU_SetStatus(mcu, 0, STATUS_C);
}

// ADD:G.[B|W] <EAs>, Rd
template <Size Sz, typename Mode>
void I_ADD_G_EAs_Rd(mcu_t& mcu, const InstructionParams& st)
{
    InstructionScope<Sz, Mode> scope(mcu, st, 1);

    const SizeToIntType<Sz> data    = LoadFromEA<Sz>(Mode{}, mcu, st);
    const SizeToIntType<Sz> operand = LoadFromOpReg<Sz>(mcu, st);

    BinopResult<Sz> result = GenericAdd<Sz>(data, operand);

    StoreToOpReg<Sz>(mcu, st, result.result_bits);
    MCU_SetStatus(mcu, result.negative, STATUS_N);
    MCU_SetStatus(mcu, result.zero, STATUS_Z);
    MCU_SetStatus(mcu, result.overflow, STATUS_V);
    MCU_SetStatus(mcu, result.carry, STATUS_C);
}

template <Size Sz, typename Mode>
inline void I_BSET_imm4_EAd(mcu_t& mcu, const InstructionParams& st)
{
    InstructionScope<Sz, Mode> scope(mcu, st, 1);

    const SizeToIntType<Sz> mask   = 1 << st.op_data;
    const SizeToIntType<Sz> data   = LoadFromEA<Sz>(Mode{}, mcu, st);
    const SizeToIntType<Sz> result = data | mask;
    const bool              Z      = (data & mask) == 0;
    StoreToEA<Sz>(Mode{}, mcu, st, result);
    MCU_SetStatus(mcu, Z, STATUS_Z);
}

template <Size Sz, typename Mode>
inline void I_BSET_Rs_EAd(mcu_t& mcu, const InstructionParams& st)
{
    InstructionScope<Sz, Mode> scope(mcu, st, 1);

    // Both Byte and Word loads work here since we truncate to 4 bits either way.
    const uint8_t           bit    = LoadFromOpReg<Size::Word>(mcu, st) & 0b1111;
    const SizeToIntType<Sz> mask   = 1 << bit;
    const SizeToIntType<Sz> data   = LoadFromEA<Sz>(Mode{}, mcu, st);
    const SizeToIntType<Sz> result = data | mask;
    const bool              Z      = (data & mask) == 0;
    StoreToEA<Sz>(Mode{}, mcu, st, result);
    MCU_SetStatus(mcu, Z, STATUS_Z);
}

// BNOT.[B|W] #xx, <EAd>
template <Size Sz, typename Mode>
inline void I_BNOT_imm4_EAd(mcu_t& mcu, const InstructionParams& st)
{
    InstructionScope<Sz, Mode> scope(mcu, st, 1);

    const SizeToIntType<Sz> mask   = 1 << st.op_data;
    const SizeToIntType<Sz> data   = LoadFromEA<Sz>(Mode{}, mcu, st);
    const SizeToIntType<Sz> result = data ^ mask;
    const bool              Z      = (data & mask) == 0;
    StoreToEA<Sz>(Mode{}, mcu, st, result);
    MCU_SetStatus(mcu, Z, STATUS_Z);
}

// BCLR.[B|W] #xx, <EAd>
template <Size Sz, typename State>
inline void I_BCLR_imm4_EAd(mcu_t& mcu, const InstructionParams& st)
{
    InstructionScope<Sz, State> scope(mcu, st, 1);

    const SizeToIntType<Sz> mask = 1 << st.op_data;
    const SizeToIntType<Sz> data = LoadFromEA<Sz>(State{}, mcu, st);
    MCU_SetStatus(mcu, (data & mask) == 0, STATUS_Z);
    StoreToEA<Sz>(State{}, mcu, st, data & (~mask));
}

// BCLR.[B|W] Rs,<EAd>
template <Size Sz, typename State>
inline void I_BCLR_Rs_EAd(mcu_t& mcu, const InstructionParams& st)
{
    InstructionScope<Sz, State> scope(mcu, st, 1);

    const uint8_t           bit  = LoadFromOpReg<Size::Word>(mcu, st) & 0b1111;
    const SizeToIntType<Sz> mask = 1 << bit;
    const SizeToIntType<Sz> data = LoadFromEA<Sz>(State{}, mcu, st);
    MCU_SetStatus(mcu, (data & mask) == 0, STATUS_Z);
    StoreToEA<Sz>(State{}, mcu, st, data & (~mask));
}

template <Size Sz, typename Mode>
inline void I_BTST_imm4_EAd(mcu_t& mcu, const InstructionParams& st)
{
    InstructionScope<Sz, Mode> scope(mcu, st, 1);

    const SizeToIntType<Sz> mask = 1 << st.op_data;
    const SizeToIntType<Sz> data = LoadFromEA<Sz>(Mode{}, mcu, st);
    const bool              Z    = (data & mask) == 0;
    MCU_SetStatus(mcu, Z, STATUS_Z);
}

template <Size Sz, typename Mode>
inline void I_BTST_Rs_EAd(mcu_t& mcu, const InstructionParams& st)
{
    InstructionScope<Sz, Mode> scope(mcu, st, 1);

    const SizeToIntType<Sz> data  = LoadFromEA<Sz>(Mode{}, mcu, st);
    const uint8_t           shift = LoadFromOpReg<Sz>(mcu, st) & 0b1111;
    const SizeToIntType<Sz> mask  = 1 << shift;
    const bool              Z     = (data & mask) == 0;
    MCU_SetStatus(mcu, Z, STATUS_Z);
}

// MULXU.[B|W] <EAs>, Rd
template <Size Sz, typename State>
inline void I_MULXU_EAs_Rd(mcu_t& mcu, const InstructionParams& st)
{
    using ResultType = WidenType<SizeToIntType<Sz>>;

    InstructionScope<Sz, State> scope(mcu, st, 1);

    const SizeToIntType<Sz> data = LoadFromEA<Sz>(State{}, mcu, st);
    const ResultType result      = static_cast<ResultType>(data) * static_cast<ResultType>(LoadFromOpReg<Sz>(mcu, st));

    // Store size is one size wider than the operation size.
    if constexpr (Sz == Size::Byte)
    {
        mcu.r[st.op_reg] = result;
        MCU_SetStatus(mcu, result & 0x8000, STATUS_N);
    }
    else if constexpr (Sz == Size::Word)
    {
        // TODO: consider a DoubleWord Size to model larger operations?
        mcu.r[st.op_reg + 0] = (uint16_t)(result >> 16);
        mcu.r[st.op_reg + 1] = (uint16_t)result;
        MCU_SetStatus(mcu, result & 0x80000000, STATUS_N);
    }

    MCU_SetStatus(mcu, result == 0, STATUS_Z);
    MCU_SetStatus(mcu, 0, STATUS_V);
    MCU_SetStatus(mcu, 0, STATUS_C);
}

// DIVXU.[B|W] <EAs>, Rd
template <Size Sz, typename State>
inline void I_DIVXU_EAs_Rd(mcu_t& mcu, const InstructionParams& st)
{
    using WideType = WidenType<SizeToIntType<Sz>>;

    InstructionScope<Sz, State> scope(mcu, st, 1);

    WideType dividend;
    if (Sz == Size::Byte)
    {
        dividend = LoadFromOpReg<Size::Word>(mcu, st);
    }
    else if (Sz == Size::Word)
    {
        dividend = static_cast<uint32_t>((mcu.r[st.op_reg] << 16) | mcu.r[st.op_reg + 1]);
    }

    const SizeToIntType<Sz> divisor = LoadFromEA<Sz>(State{}, mcu, st);

    if (divisor == 0)
    {
        // TODO exception
        MCU_SetStatus(mcu, 0, STATUS_N);
        MCU_SetStatus(mcu, 1, STATUS_Z);
        MCU_SetStatus(mcu, 0, STATUS_V);
        MCU_SetStatus(mcu, 0, STATUS_C);
        return;
    }

    const WideType q = dividend / divisor;
    const WideType r = dividend % divisor;

    if (q > SizeTraits<Sz>::UnsignedMax)
    {
        MCU_SetStatus(mcu, 0, STATUS_N);
        MCU_SetStatus(mcu, 0, STATUS_Z);
        MCU_SetStatus(mcu, 1, STATUS_V);
        MCU_SetStatus(mcu, 0, STATUS_C);
        return;
    }

    if (Sz == Size::Byte)
    {
        mcu.r[st.op_reg] = (uint16_t)((r << 8) | q);
    }
    else if (Sz == Size::Word)
    {
        mcu.r[st.op_reg + 0] = (uint16_t)r;
        mcu.r[st.op_reg + 1] = (uint16_t)q;
    }

    MCU_SetStatus(mcu, q & MSB<Sz>, STATUS_N);
    MCU_SetStatus(mcu, q == 0, STATUS_Z);
    MCU_SetStatus(mcu, 0, STATUS_V);
    MCU_SetStatus(mcu, 0, STATUS_C);
}

//=============================================================================
// General instruction handlers
//=============================================================================

// MOV:G.[B|W] <EAs>, Rd
template <Size Sz, typename State>
inline void I_MOV_G_EAs_Rd(mcu_t& mcu, const InstructionParams& st)
{
    InstructionScope<Sz, State> scope(mcu, st, 1);

    const SizeToIntType<Sz> data = LoadFromEA<Sz>(State{}, mcu, st);
    StoreToOpReg<Sz>(mcu, st, data);
    MCU_SetStatus(mcu, data & MSB<Sz>, STATUS_N);
    MCU_SetStatus(mcu, data == 0, STATUS_Z);
    MCU_SetStatus(mcu, 0, STATUS_V);
}

// MOV:G.[B|W] Rs, <EAd>
template <Size Sz, typename Mode>
inline void I_MOV_G_Rs_EAd(mcu_t& mcu, const InstructionParams& st)
{
    InstructionScope<Sz, Mode> scope(mcu, st, 1);

    const SizeToIntType<Sz> data = LoadFromOpReg<Sz>(mcu, st);
    StoreToEA<Sz>(Mode{}, mcu, st, data);
    MCU_SetStatus(mcu, data & MSB<Sz>, STATUS_N);
    MCU_SetStatus(mcu, data == 0, STATUS_Z);
    MCU_SetStatus(mcu, 0, STATUS_V);
}

// MOV:G.[B|W] #xx:8, <EAd>
template <Size Sz, typename State>
inline void I_MOV_G_imm8_EAd(mcu_t& mcu, const InstructionParams& st)
{
    InstructionScope<Sz, State> scope(mcu, st, 2);

    SizeToIntType<Sz> data;
    if (Sz == Size::Byte)
    {
        data = LoadFromOpData<Size::Byte>(mcu, st);
    }
    else if (Sz == Size::Word)
    {
        data = SX(LoadFromOpData<Size::Byte>(mcu, st));
    }

    StoreToEA<Sz>(State{}, mcu, st, data);
    MCU_SetStatus(mcu, data & MSB<Sz>, STATUS_N);
    MCU_SetStatus(mcu, data == 0, STATUS_Z);
    MCU_SetStatus(mcu, 0, STATUS_V);
}

// MOV:G.[B|W] #xx:16, <EAd>
template <Size Sz, typename State>
inline void I_MOV_G_imm16_EAd(mcu_t& mcu, const InstructionParams& st)
{
    InstructionScope<Sz, State> scope(mcu, st, 3);

    const SizeToIntType<Sz> data = LoadFromOpData<Sz>(mcu, st);
    StoreToEA<Sz>(State{}, mcu, st, data);
    MCU_SetStatus(mcu, data & MSB<Sz>, STATUS_N);
    MCU_SetStatus(mcu, data == 0, STATUS_Z);
    MCU_SetStatus(mcu, 0, STATUS_V);
}

// SWAP Rd
// This instruction defined for BYTE size despite operating on the entire register WORD.
template <typename Mode>
inline void I_SWAP_B_Rd(mcu_t& mcu, const InstructionParams& st)
{
    InstructionScope<Size::Byte, Mode> scope(mcu, st, 1);

    const uint16_t value = std::byteswap(mcu.r[st.ea_reg]);
    mcu.r[st.ea_reg]     = value;
    MCU_SetStatus(mcu, value & 0x8000, STATUS_N);
    MCU_SetStatus(mcu, value == 0, STATUS_Z);
    MCU_SetStatus(mcu, 0, STATUS_V);
}

// XCH Rs, Rd
// This instruction defined for WORD size only.
template <typename Mode>
inline void I_XCH_W_Rs_Rd(mcu_t& mcu, const InstructionParams& instr)
{
    InstructionScope<Size::Word, Mode> scope(mcu, instr, 1);

    const uint16_t tmp  = mcu.r[instr.ea_reg];
    mcu.r[instr.ea_reg] = mcu.r[instr.op_reg];
    mcu.r[instr.op_reg] = tmp;
}

// ADD:Q.[B|W] #1, <EAd>
// ADD:Q.[B|W] #2, <EAd>
// ADD:Q.[B|W] #-1, <EAd>
// ADD:Q.[B|W] #-2, <EAd>
template <Size Sz, typename State, int8_t N>
inline void I_ADD_Q_n(mcu_t& mcu, const InstructionParams& st)
{
    InstructionScope<Sz, State> scope(mcu, st, 1);

    const SizeToIntType<Sz> ea_byte = LoadFromEA<Sz>(State{}, mcu, st);

    const BinopResult<Sz> result = GenericAdd<Sz>(ea_byte, N);

    StoreToEA<Sz>(State{}, mcu, st, result.result_bits);
    MCU_SetStatus(mcu, result.negative, STATUS_N);
    MCU_SetStatus(mcu, result.zero, STATUS_Z);
    MCU_SetStatus(mcu, result.overflow, STATUS_V);
    MCU_SetStatus(mcu, result.carry, STATUS_C);
}

template <Size Sz, typename State>
inline void I_ADDX_EAs_Rd(mcu_t& mcu, const InstructionParams& st)
{
    InstructionScope<Sz, State> scope(mcu, st, 1);

    const bool old_C = mcu.sr & STATUS_C;
    const bool old_Z = mcu.sr & STATUS_Z;

    const SizeToIntType<Sz> data    = LoadFromEA<Sz>(State{}, mcu, st);
    const SizeToIntType<Sz> operand = LoadFromOpReg<Sz>(mcu, st);

    const BinopResult<Sz> result = GenericAdd<Sz>(data, operand, old_C);

    StoreToOpReg<Sz>(mcu, st, result.result_bits);
    MCU_SetStatus(mcu, result.negative, STATUS_N);
    MCU_SetStatus(mcu, old_Z && result.zero, STATUS_Z);
    MCU_SetStatus(mcu, result.overflow, STATUS_V);
    MCU_SetStatus(mcu, result.carry, STATUS_C);
}

template <Size Sz, typename State>
inline void I_SUB_EAs_Rd(mcu_t& mcu, const InstructionParams& st)
{
    InstructionScope<Sz, State> scope(mcu, st, 1);

    const SizeToIntType<Sz> EAs = LoadFromEA<Sz>(State{}, mcu, st);
    const SizeToIntType<Sz> Rd  = LoadFromOpReg<Sz>(mcu, st);

    const BinopResult<Sz> result = GenericSubtract<Sz>(Rd, EAs);

    StoreToOpReg<Sz>(mcu, st, result.result_bits);

    MCU_SetStatus(mcu, result.negative, STATUS_N);
    MCU_SetStatus(mcu, result.zero, STATUS_Z);
    MCU_SetStatus(mcu, result.overflow, STATUS_V);
    MCU_SetStatus(mcu, result.carry, STATUS_C);
}

template <Size Sz, typename State>
inline void I_SUBX_EAs_Rd(mcu_t& mcu, const InstructionParams& st)
{
    InstructionScope<Sz, State> scope(mcu, st, 1);

    const bool              sub_C = (mcu.sr & STATUS_C) != 0;
    const SizeToIntType<Sz> EAs   = LoadFromEA<Sz>(State{}, mcu, st);
    const SizeToIntType<Sz> Rd    = LoadFromOpReg<Sz>(mcu, st);

    const BinopResult<Sz> result = GenericSubtract<Sz>(Rd, EAs, sub_C);

    StoreToOpReg<Sz>(mcu, st, result.result_bits);

    MCU_SetStatus(mcu, result.negative, STATUS_N);
    MCU_SetStatus(mcu, result.zero, STATUS_Z);
    MCU_SetStatus(mcu, result.overflow, STATUS_V);
    MCU_SetStatus(mcu, result.carry, STATUS_C);
}

template <Size Sz, typename State>
inline void I_SUBS_EAs_Rd(mcu_t& mcu, const InstructionParams& st)
{
    InstructionScope<Sz, State> scope(mcu, st, 1);

    uint16_t EAs = LoadFromEA<Sz>(State{}, mcu, st);

    if constexpr (Sz == Size::Byte)
    {
        // Safe to truncate because this was an 8-bit load.
        EAs = SX(static_cast<uint8_t>(EAs));
    }

    // Rd always accessed as word in this form
    const uint16_t Rd = LoadFromOpReg<Size::Word>(mcu, st);

    const uint16_t sub_u = Rd - EAs;
    StoreToOpReg<Size::Word>(mcu, st, sub_u);
}

template <Size Sz, typename State>
inline void I_TST_EAd(mcu_t& mcu, const InstructionParams& st)
{
    InstructionScope<Sz, State> scope(mcu, st, 1);

    const SizeToIntType<Sz> value = LoadFromEA<Sz>(State{}, mcu, st);
    MCU_SetStatus(mcu, value & MSB<Sz>, STATUS_N);
    MCU_SetStatus(mcu, value == 0, STATUS_Z);
    MCU_SetStatus(mcu, 0, STATUS_V);
    MCU_SetStatus(mcu, 0, STATUS_C);
}

template <Size Sz, typename State>
inline void I_NEG_EAd(mcu_t& mcu, const InstructionParams& st)
{
    InstructionScope<Sz, State> scope(mcu, st, 1);

    const SizeToIntType<Sz> value = LoadFromEA<Sz>(State{}, mcu, st);

    const BinopResult<Sz> result = GenericSubtract<Sz>(0, value);

    StoreToEA<Sz>(State{}, mcu, st, result.result_bits);
    MCU_SetStatus(mcu, result.negative, STATUS_N);
    MCU_SetStatus(mcu, result.zero, STATUS_Z);
    MCU_SetStatus(mcu, result.overflow, STATUS_V);
    MCU_SetStatus(mcu, result.carry, STATUS_C);
}

// SHLL.[B|W] <EAd>
template <Size Sz, typename State>
inline void I_SHLL_EAd(mcu_t& mcu, const InstructionParams& st)
{
    InstructionScope<Sz, State> scope(mcu, st, 1);

    const SizeToIntType<Sz> val_old = LoadFromEA<Sz>(State{}, mcu, st);
    const SizeToIntType<Sz> val_new = val_old << 1;
    StoreToEA<Sz>(State{}, mcu, st, val_new);

    const bool N = val_new & MSB<Sz>;
    const bool Z = val_new == 0;
    const bool V = 0;
    const bool C = val_old & MSB<Sz>;

    MCU_SetStatus(mcu, N, STATUS_N);
    MCU_SetStatus(mcu, Z, STATUS_Z);
    MCU_SetStatus(mcu, V, STATUS_V);
    MCU_SetStatus(mcu, C, STATUS_C);
}

// SHLR.[B|W] <EAd>
template <Size Sz, typename State>
inline void I_SHLR_EAd(mcu_t& mcu, const InstructionParams& st)
{
    InstructionScope<Sz, State> scope(mcu, st, 1);

    const SizeToIntType<Sz> val_old = LoadFromEA<Sz>(State{}, mcu, st);
    const SizeToIntType<Sz> val_new = val_old >> 1;
    StoreToEA<Sz>(State{}, mcu, st, val_new);

    const bool N = 0;
    const bool Z = val_new == 0;
    const bool V = 0;
    const bool C = val_old & 1;

    MCU_SetStatus(mcu, N, STATUS_N);
    MCU_SetStatus(mcu, Z, STATUS_Z);
    MCU_SetStatus(mcu, V, STATUS_V);
    MCU_SetStatus(mcu, C, STATUS_C);
}

// SHAL.[B|W] <EAd>
template <Size Sz, typename State>
inline void I_SHAL_EAd(mcu_t& mcu, const InstructionParams& st)
{
    InstructionScope<Sz, State> scope(mcu, st, 1);

    const SizeToIntType<Sz> val_old = LoadFromEA<Sz>(State{}, mcu, st);
    const SizeToIntType<Sz> val_new = val_old << 1;
    StoreToEA<Sz>(State{}, mcu, st, val_new);

    const bool N = val_new & MSB<Sz>;
    const bool Z = val_new == 0;
    const bool V = (val_new & MSB<Sz>) != (val_old & MSB<Sz>);
    const bool C = val_old & MSB<Sz>;

    MCU_SetStatus(mcu, N, STATUS_N);
    MCU_SetStatus(mcu, Z, STATUS_Z);
    MCU_SetStatus(mcu, V, STATUS_V);
    MCU_SetStatus(mcu, C, STATUS_C);
}

// SHAR.[B|W] <EAd>
template <Size Sz, typename State>
inline void I_SHAR_EAd(mcu_t& mcu, const InstructionParams& st)
{
    InstructionScope<Sz, State> scope(mcu, st, 1);

    const SizeToIntType<Sz> val_old = LoadFromEA<Sz>(State{}, mcu, st);
    const SizeToIntType<Sz> val_new = (val_old >> 1) | (val_old & MSB<Sz>);
    StoreToEA<Sz>(State{}, mcu, st, val_new);

    const bool N = val_new & MSB<Sz>;
    const bool Z = val_new == 0;
    const bool V = 0;
    const bool C = val_old & 1;

    MCU_SetStatus(mcu, N, STATUS_N);
    MCU_SetStatus(mcu, Z, STATUS_Z);
    MCU_SetStatus(mcu, V, STATUS_V);
    MCU_SetStatus(mcu, C, STATUS_C);
}

// LDC.[B|W] <EAs>, CR
template <Size Sz, typename State>
inline void I_LDC_EAs_CR(mcu_t& mcu, const InstructionParams& st)
{
    InstructionScope<Sz, State> scope(mcu, st, 1);

    const SizeToIntType<Sz> byte = LoadFromEA<Sz>(State{}, mcu, st);

    StoreToCR<Sz>(mcu, st.op_c, byte);
    mcu.ex_ignore = 1;
}

// STC.[B|W] CR, <EAd>
template <Size Sz, typename State>
void I_STC_CR_EAd(mcu_t& mcu, const InstructionParams& st)
{
    InstructionScope<Sz, State> scope(mcu, st, 1);

    const SizeToIntType<Sz> value = LoadFromCR<Sz>(mcu, st.op_c);
    StoreToEA<Sz>(State{}, mcu, st, value);
}

// ANDC.[B|W] #xx:[8|16], CR
template <Size Sz, typename Mode, uint8_t CR>
inline void I_ANDC_immXX_CR(mcu_t& mcu, const InstructionParams& st)
{
    InstructionScope<Sz, Mode> scope(mcu, st, 1);

    const SizeToIntType<Sz> result = LoadFromCR<Sz>(mcu, CR) & static_cast<SizeToIntType<Sz>>(st.ea_data);
    StoreToCR<Sz>(mcu, CR, result);
    mcu.ex_ignore = 1;

    if (!IsSR<Sz>(CR))
    {
        const bool N = result & MSB<Sz>;
        const bool Z = result == 0;
        const bool V = 0;
        MCU_SetStatus(mcu, N, STATUS_N);
        MCU_SetStatus(mcu, Z, STATUS_Z);
        MCU_SetStatus(mcu, V, STATUS_V);
    }
}

// ORC.[B|W] #xx:[8|16], CR
template <Size Sz, typename Mode, uint8_t CR>
inline void I_ORC_immXX_CR(mcu_t& mcu, const InstructionParams& st)
{
    InstructionScope<Sz, Mode> scope(mcu, st, 1);

    const SizeToIntType<Sz> result = LoadFromCR<Sz>(mcu, CR) | static_cast<SizeToIntType<Sz>>(st.ea_data);
    StoreToCR<Sz>(mcu, CR, result);
    mcu.ex_ignore = 1;

    if (!IsSR<Sz>(CR))
    {
        const bool N = result & MSB<Sz>;
        const bool Z = result == 0;
        const bool V = 0;
        MCU_SetStatus(mcu, N, STATUS_N);
        MCU_SetStatus(mcu, Z, STATUS_Z);
        MCU_SetStatus(mcu, V, STATUS_V);
    }
}

//=============================================================================
// End general instruction handlers
//=============================================================================

void I_BRA(mcu_t& mcu, const InstructionParams& st);
void I_BRN(mcu_t& mcu, const InstructionParams& st);
void I_BHI(mcu_t& mcu, const InstructionParams& st);
void I_BLS(mcu_t& mcu, const InstructionParams& st);
void I_BCC(mcu_t& mcu, const InstructionParams& st);
void I_BCS(mcu_t& mcu, const InstructionParams& st);
void I_BNE(mcu_t& mcu, const InstructionParams& st);
void I_BEQ(mcu_t& mcu, const InstructionParams& st);
void I_BVC(mcu_t& mcu, const InstructionParams& st);
void I_BVS(mcu_t& mcu, const InstructionParams& st);
void I_BPL(mcu_t& mcu, const InstructionParams& st);
void I_BMI(mcu_t& mcu, const InstructionParams& st);
void I_BGE(mcu_t& mcu, const InstructionParams& st);
void I_BLT(mcu_t& mcu, const InstructionParams& st);
void I_BGT(mcu_t& mcu, const InstructionParams& st);
void I_BLE(mcu_t& mcu, const InstructionParams& st);

void I_RTE(mcu_t& mcu, const InstructionParams& st);

void I_NOP(mcu_t&, const InstructionParams&);

// EXTS Rd
// This instruction defined for BYTE size despite operating on the entire register WORD.
template <typename Mode>
inline void I_EXTS_B_Rd(mcu_t& mcu, const InstructionParams& st)
{
    InstructionScope<Size::Byte, Mode> scope(mcu, st, 1);

    mcu.r[st.ea_reg] = SX(static_cast<uint8_t>(mcu.r[st.ea_reg]));
    MCU_SetStatus(mcu, mcu.r[st.ea_reg] & 0x8000, STATUS_N);
    MCU_SetStatus(mcu, mcu.r[st.ea_reg] == 0, STATUS_Z);
    MCU_SetStatus(mcu, 0, STATUS_V);
    MCU_SetStatus(mcu, 0, STATUS_C);
}

// EXTU Rd
// This instruction defined for BYTE size despite operating on the entire register WORD.
template <typename Mode>
inline void I_EXTU_B_Rd(mcu_t& mcu, const InstructionParams& st)
{
    InstructionScope<Size::Byte, Mode> scope(mcu, st, 1);

    mcu.r[st.ea_reg] = (uint8_t)mcu.r[st.ea_reg];
    MCU_SetStatus(mcu, 0, STATUS_N);
    MCU_SetStatus(mcu, mcu.r[st.ea_reg] == 0, STATUS_Z);
    MCU_SetStatus(mcu, 0, STATUS_V);
    MCU_SetStatus(mcu, 0, STATUS_C);
}

template <Size Sz, typename Mode>
inline void I_NOT_EAd(mcu_t& mcu, const InstructionParams& st)
{
    InstructionScope<Sz, Mode> scope(mcu, st, 1);

    const SizeToIntType<Sz> value = ~LoadFromEA<Sz>(Mode{}, mcu, st);
    StoreToEA<Sz>(Mode{}, mcu, st, value);
    MCU_SetStatus(mcu, value & MSB<Sz>, STATUS_N);
    MCU_SetStatus(mcu, value == 0, STATUS_Z);
    MCU_SetStatus(mcu, 0, STATUS_V);
}

template <Size Sz, typename Mode>
inline void I_ADDS_EAs_Rd(mcu_t& mcu, const InstructionParams& st)
{
    InstructionScope<Sz, Mode> scope(mcu, st, 1);

    // This instruction is unusual in that it always operates on words even if the form is ADDS.B.
    const uint16_t old_reg = LoadFromOpReg<Size::Word>(mcu, st);

    uint16_t value;
    if constexpr (Sz == Size::Byte)
    {
        // To get a word addend we sign extend the low byte of the register.
        value = SX(LoadFromEA<Size::Byte>(Mode{}, mcu, st));
    }
    else
    {
        value = LoadFromEA<Size::Word>(Mode{}, mcu, st);
    }

    StoreToOpReg<Size::Word>(mcu, st, old_reg + value);
}

template <Size Sz, typename Mode>
inline void I_ROTL_EAd(mcu_t& mcu, const InstructionParams& st)
{
    InstructionScope<Sz, Mode> scope(mcu, st, 1);

    const SizeToIntType<Sz> input  = LoadFromEA<Sz>(Mode{}, mcu, st);
    const bool              msb    = input & MSB<Sz>;
    const SizeToIntType<Sz> result = RotateLeft(input, msb);
    StoreToEA<Sz>(Mode{}, mcu, st, result);
    MCU_SetStatus(mcu, result & MSB<Sz>, STATUS_N);
    MCU_SetStatus(mcu, result == 0, STATUS_Z);
    MCU_SetStatus(mcu, 0, STATUS_V);
    MCU_SetStatus(mcu, msb, STATUS_C);
}

template <Size Sz, typename Mode>
inline void I_ROTR_EAd(mcu_t& mcu, const InstructionParams& st)
{
    InstructionScope<Sz, Mode> scope(mcu, st, 1);

    const SizeToIntType<Sz> input  = LoadFromEA<Sz>(Mode{}, mcu, st);
    const bool              lsb    = input & 1;
    const SizeToIntType<Sz> result = RotateRight(input, lsb);
    StoreToEA<Sz>(Mode{}, mcu, st, result);
    MCU_SetStatus(mcu, result & MSB<Sz>, STATUS_N);
    MCU_SetStatus(mcu, result == 0, STATUS_Z);
    MCU_SetStatus(mcu, 0, STATUS_V);
    MCU_SetStatus(mcu, lsb, STATUS_C);
}

template <Size Sz, typename Mode>
inline void I_ROTXL_EAd(mcu_t& mcu, const InstructionParams& st)
{
    InstructionScope<Sz, Mode> scope(mcu, st, 1);

    const bool              old_C  = mcu.sr & STATUS_C;
    const SizeToIntType<Sz> input  = LoadFromEA<Sz>(Mode{}, mcu, st);
    const bool              msb    = input & MSB<Sz>;
    const SizeToIntType<Sz> result = RotateLeft(input, old_C);
    StoreToEA<Sz>(Mode{}, mcu, st, result);
    MCU_SetStatus(mcu, result & MSB<Sz>, STATUS_N);
    MCU_SetStatus(mcu, result == 0, STATUS_Z);
    MCU_SetStatus(mcu, 0, STATUS_V);
    MCU_SetStatus(mcu, msb, STATUS_C);
}

template <Size Sz, typename Mode>
inline void I_ROTXR_EAd(mcu_t& mcu, const InstructionParams& st)
{
    InstructionScope<Sz, Mode> scope(mcu, st, 1);

    const bool              old_C  = mcu.sr & STATUS_C;
    const SizeToIntType<Sz> input  = LoadFromEA<Sz>(Mode{}, mcu, st);
    const bool              lsb    = input & 1;
    const SizeToIntType<Sz> result = RotateRight(input, old_C);
    StoreToEA<Sz>(Mode{}, mcu, st, result);
    MCU_SetStatus(mcu, result & MSB<Sz>, STATUS_N);
    MCU_SetStatus(mcu, result == 0, STATUS_Z);
    MCU_SetStatus(mcu, 0, STATUS_V);
    MCU_SetStatus(mcu, lsb, STATUS_C);
}

template <Size Sz, typename Mode>
inline void I_XOR_EAs_Rd(mcu_t& mcu, const InstructionParams& st)
{
    InstructionScope<Sz, Mode> scope(mcu, st, 1);

    const SizeToIntType<Sz> EAs    = LoadFromEA<Sz>(Mode{}, mcu, st);
    const SizeToIntType<Sz> Rd     = LoadFromOpReg<Sz>(mcu, st);
    const SizeToIntType<Sz> result = EAs ^ Rd;

    StoreToOpReg<Sz>(mcu, st, result);
    MCU_SetStatus(mcu, result & MSB<Sz>, STATUS_N);
    MCU_SetStatus(mcu, result == 0, STATUS_Z);
    MCU_SetStatus(mcu, 0, STATUS_V);
}

template <Size Sz, typename Mode>
inline void I_OR_EAs_Rd(mcu_t& mcu, const InstructionParams& st)
{
    InstructionScope<Sz, Mode> scope(mcu, st, 1);

    const SizeToIntType<Sz> EAs    = LoadFromEA<Sz>(Mode{}, mcu, st);
    const SizeToIntType<Sz> Rd     = LoadFromOpReg<Sz>(mcu, st);
    const SizeToIntType<Sz> result = EAs | Rd;

    StoreToOpReg<Sz>(mcu, st, result);
    MCU_SetStatus(mcu, result & MSB<Sz>, STATUS_N);
    MCU_SetStatus(mcu, result == 0, STATUS_Z);
    MCU_SetStatus(mcu, 0, STATUS_V);
}

template <Size Sz, typename Mode>
inline void I_AND_EAs_Rd(mcu_t& mcu, const InstructionParams& st)
{
    InstructionScope<Sz, Mode> scope(mcu, st, 1);

    const SizeToIntType<Sz> EAs    = LoadFromEA<Sz>(Mode{}, mcu, st);
    const SizeToIntType<Sz> Rd     = LoadFromOpReg<Sz>(mcu, st);
    const SizeToIntType<Sz> result = EAs & Rd;

    StoreToOpReg<Sz>(mcu, st, result);
    MCU_SetStatus(mcu, result & MSB<Sz>, STATUS_N);
    MCU_SetStatus(mcu, result == 0, STATUS_Z);
    MCU_SetStatus(mcu, 0, STATUS_V);
}

// CMP:E #xx:8,Rd
template <uint8_t Rn>
inline void I_CMP_E_imm8_Rd(mcu_t& mcu, const InstructionParams& st)
{
    // behave as register-direct CMP:G.B #xx:8,Rd
    I_CMP_G_imm8_EAd<Size::Byte, Mode_Rn>(mcu, st);
    // TODO/FIXME
    --mcu.pc;
}

template <uint8_t Rn>
inline void I_CMP_I_W_imm16_Rd(mcu_t& mcu, const InstructionParams& st)
{
    // behave as register-direct CMP:G.W #xx:16,EAd
    I_CMP_G_imm16_EAd<Size::Word, Mode_Rn>(mcu, st);
    // TODO/FIXME
    --mcu.pc;
}

template <uint8_t Rn>
inline void I_MOV_E_imm8_Rd(mcu_t& mcu, const InstructionParams& st)
{
    // behave as register-direct MOV:G.B #xx:8,EAd
    I_MOV_G_imm8_EAd<Size::Byte, Mode_Rn>(mcu, st);
    // TODO/FIXME
    --mcu.pc;
}

template <Size Sz, uint8_t Rn>
inline void I_MOV_L_aa8_Rd(mcu_t& mcu, const InstructionParams& st)
{
    // behave as @aa:8 MOV:G.[B|W] EAs,Rd
    I_MOV_G_EAs_Rd<Sz, Mode_Aaa8>(mcu, st);
    // TODO/FIXME
    --mcu.pc;
}

template <uint8_t Rn>
inline void I_MOV_I_W_imm16_Rd(mcu_t& mcu, const InstructionParams& st)
{
    // behave as register-direct MOV:G.W #xx:16,EAd
    I_MOV_G_imm16_EAd<Size::Word, Mode_Rn>(mcu, st);
    // TODO/FIXME
    --mcu.pc;
}

template <Size Sz, uint8_t Rn>
inline void I_MOV_S_Rs_aa8(mcu_t& mcu, const InstructionParams& st)
{
    // behave as @aa:8 MOV:G.B Rs,EAd
    I_MOV_G_Rs_EAd<Sz, Mode_Aaa8>(mcu, st);
    // TODO/FIXME
    --mcu.pc;
}

template <uint8_t Rn>
inline void I_SCB_F(mcu_t& mcu, const InstructionParams& st)
{
    --mcu.r[Rn];
    if (mcu.r[Rn] == (uint16_t)-1)
    {
        mcu.pc = st.br_false;
    }
    else
    {
        mcu.pc = st.br_true;
    }
}

template <uint8_t Rn>
inline void I_SCB_NE(mcu_t& mcu, const InstructionParams& st)
{
    const bool Z = mcu.sr & STATUS_Z;
    if (!Z)
    {
        mcu.pc = st.br_false;
    }
    else
    {
        I_SCB_F<Rn>(mcu, st);
    }
}

template <uint8_t Rn>
inline void I_SCB_EQ(mcu_t& mcu, const InstructionParams& st)
{
    const bool Z = mcu.sr & STATUS_Z;
    if (Z)
    {
        mcu.pc = st.br_false;
    }
    else
    {
        I_SCB_F<Rn>(mcu, st);
    }
}

void I_RTS(mcu_t& mcu, const InstructionParams& st);
void I_PRTS(mcu_t& mcu, const InstructionParams& st);
void I_SLEEP(mcu_t& mcu, const InstructionParams& st);

// STM <register list>,@-SP
inline void I_STM(mcu_t& mcu, const InstructionParams& st)
{
    const uint8_t reglist = (uint8_t)st.op_data;
    // clang-format off
    if (reglist & 0b10000000) MCU_PushStack(mcu, mcu.r[7]);
    if (reglist & 0b01000000) MCU_PushStack(mcu, mcu.r[6]);
    if (reglist & 0b00100000) MCU_PushStack(mcu, mcu.r[5]);
    if (reglist & 0b00010000) MCU_PushStack(mcu, mcu.r[4]);
    if (reglist & 0b00001000) MCU_PushStack(mcu, mcu.r[3]);
    if (reglist & 0b00000100) MCU_PushStack(mcu, mcu.r[2]);
    if (reglist & 0b00000010) MCU_PushStack(mcu, mcu.r[1]);
    if (reglist & 0b00000001) MCU_PushStack(mcu, mcu.r[0]);
    // clang-format on
    mcu.pc += 2;
}

// STM <register list>,@-SP
// Specialized form of STM. Optimizer can eliminate runtime bit tests.
template <uint8_t Reglist>
inline void I_STM_Fast(mcu_t& mcu, const InstructionParams& st)
{
    (void)st;
    // clang-format off
    if (Reglist & 0b10000000) MCU_PushStack(mcu, mcu.r[7]);
    if (Reglist & 0b01000000) MCU_PushStack(mcu, mcu.r[6]);
    if (Reglist & 0b00100000) MCU_PushStack(mcu, mcu.r[5]);
    if (Reglist & 0b00010000) MCU_PushStack(mcu, mcu.r[4]);
    if (Reglist & 0b00001000) MCU_PushStack(mcu, mcu.r[3]);
    if (Reglist & 0b00000100) MCU_PushStack(mcu, mcu.r[2]);
    if (Reglist & 0b00000010) MCU_PushStack(mcu, mcu.r[1]);
    if (Reglist & 0b00000001) MCU_PushStack(mcu, mcu.r[0]);
    // clang-format on
    mcu.pc += 2;
}

// LDM @SP+,<register list>
inline void I_LDM(mcu_t& mcu, const InstructionParams& st)
{
    const uint8_t reglist = (uint8_t)st.op_data;
    // clang-format off
    if (reglist & 0b00000001) mcu.r[0] = MCU_PopStack(mcu);
    if (reglist & 0b00000010) mcu.r[1] = MCU_PopStack(mcu);
    if (reglist & 0b00000100) mcu.r[2] = MCU_PopStack(mcu);
    if (reglist & 0b00001000) mcu.r[3] = MCU_PopStack(mcu);
    if (reglist & 0b00010000) mcu.r[4] = MCU_PopStack(mcu);
    if (reglist & 0b00100000) mcu.r[5] = MCU_PopStack(mcu);
    if (reglist & 0b01000000) mcu.r[6] = MCU_PopStack(mcu);
    if (reglist & 0b10000000) mcu.r[7] = MCU_PopStack(mcu);
    // clang-format on
    mcu.pc += 2;
}

// LDM @SP+,<register list>
// Specialized form of LDM. Optimizer can eliminate runtime bit tests.
template <uint8_t Reglist>
inline void I_LDM_Fast(mcu_t& mcu, const InstructionParams& st)
{
    (void)st;
    // clang-format off
    if (Reglist & 0b00000001) mcu.r[0] = MCU_PopStack(mcu);
    if (Reglist & 0b00000010) mcu.r[1] = MCU_PopStack(mcu);
    if (Reglist & 0b00000100) mcu.r[2] = MCU_PopStack(mcu);
    if (Reglist & 0b00001000) mcu.r[3] = MCU_PopStack(mcu);
    if (Reglist & 0b00010000) mcu.r[4] = MCU_PopStack(mcu);
    if (Reglist & 0b00100000) mcu.r[5] = MCU_PopStack(mcu);
    if (Reglist & 0b01000000) mcu.r[6] = MCU_PopStack(mcu);
    if (Reglist & 0b10000000) mcu.r[7] = MCU_PopStack(mcu);
    // clang-format on
    mcu.pc += 2;
}

void I_JMP_ARn(mcu_t& mcu, const InstructionParams& st);
void I_JMP_aa16(mcu_t& mcu, const InstructionParams& st);
void I_JSR_aa16(mcu_t& mcu, const InstructionParams& st);
void I_JSR_ARn(mcu_t& mcu, const InstructionParams& st);
void I_PJMP_aa24(mcu_t& mcu, const InstructionParams& st);
void I_PJSR_aa24(mcu_t& mcu, const InstructionParams& st);
void I_PJSR_ARn(mcu_t& mcu, const InstructionParams& st);
void I_PJMP_ARn(mcu_t& mcu, const InstructionParams& st);
void I_RTD_immXX(mcu_t& mcu, const InstructionParams& st);
void I_TRAPA_imm4(mcu_t& mcu, const InstructionParams& st);
void I_BSR(mcu_t& mcu, const InstructionParams& instr);

} // namespace decoder2
