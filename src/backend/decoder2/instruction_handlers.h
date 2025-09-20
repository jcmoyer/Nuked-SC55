#pragma once

#include <cstdint>
#include <type_traits>
#include <utility>

#include "address_modes.h"
#include "mcu.h"
#include "mcu_opcodes.h"

constexpr uint16_t SX(uint8_t byte)
{
    return (uint16_t)(int8_t)byte;
}

template <MCU_Operand_Size Sz>
struct MCU_Operand_Size_Int
{
};

template <>
struct MCU_Operand_Size_Int<MCU_Operand_Size::BYTE>
{
    using Type = uint8_t;
};

template <>
struct MCU_Operand_Size_Int<MCU_Operand_Size::WORD>
{
    using Type = uint16_t;
};

constexpr uint8_t ModeEASize(Mode_Rn)
{
    return 1;
}

constexpr uint8_t ModeEASize(Mode_ARn)
{
    return 1;
}

constexpr uint8_t ModeEASize(Mode_Ad8_Rn)
{
    return 2;
}

constexpr uint8_t ModeEASize(Mode_Ad16_Rn)
{
    return 3;
}

constexpr uint8_t ModeEASize(Mode_APreDecRn)
{
    return 1;
}

constexpr uint8_t ModeEASize(Mode_APostIncRn)
{
    return 1;
}

constexpr uint8_t ModeEASize(Mode_Aaa8)
{
    return 2;
}

constexpr uint8_t ModeEASize(Mode_Aaa16)
{
    return 3;
}

constexpr uint8_t ModeEASize(Mode_Imm8)
{
    return 2;
}

constexpr uint8_t ModeEASize(Mode_Imm16)
{
    return 3;
}

// Implements pre/post decrement/increment for @-Rn and @Rn+ addressing modes.
template <MCU_Operand_Size Sz, typename Mode>
class InstructionScope
{
public:
    InstructionScope(mcu_t& mcu, const I_CachedInstruction& instr, uint8_t instr_size)
        : m_mcu(mcu),
          m_instr(instr),
          m_instr_size(instr_size)
    {
        if constexpr (std::is_same_v<Mode, Mode_APreDecRn>)
        {
            m_mcu.r[m_instr.ea_reg] -= GetAdjust();
        }
    }

    ~InstructionScope()
    {
        if constexpr (std::is_same_v<Mode, Mode_APostIncRn>)
        {
            m_mcu.r[m_instr.ea_reg] += GetAdjust();
        }
        m_mcu.pc += static_cast<uint8_t>(ModeEASize(Mode{}) + m_instr_size);
    }

private:
    constexpr uint8_t GetAdjust() const
    {
        switch (Sz)
        {
        case MCU_Operand_Size::BYTE:
            return (m_instr.ea_reg == 7) ? 2 : 1;
        case MCU_Operand_Size::WORD:
            return 2;
        }
    }

private:
    mcu_t&                     m_mcu;
    const I_CachedInstruction& m_instr;
    const uint8_t              m_instr_size;
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
template <MCU_Operand_Size Sz>
void StoreToReg(mcu_t& mcu, uint8_t reg, typename MCU_Operand_Size_Int<Sz>::Type value)
{
    if constexpr (Sz == MCU_Operand_Size::BYTE)
        mcu.r[reg] = (mcu.r[reg] & 0xff00) | value;
    else if constexpr (Sz == MCU_Operand_Size::WORD)
        mcu.r[reg] = value;
}

template <MCU_Operand_Size Sz>
typename MCU_Operand_Size_Int<Sz>::Type LoadFromReg(mcu_t& mcu, uint8_t reg)
{
    if constexpr (Sz == MCU_Operand_Size::BYTE)
        return (uint8_t)mcu.r[reg];
    else if constexpr (Sz == MCU_Operand_Size::WORD)
        return mcu.r[reg];
}

///////////////////////////////////////////////////////////////////////////////
// Computes the EA pointer for a given addressing mode. This operation is only
// valid for modes that refer to an address in memory - the address of a
// register or immediate cannot be taken.
constexpr uint32_t LoadEA(Mode_Ad8_Rn, mcu_t& mcu, uint8_t Rn, const I_CachedInstruction& instr)
{
    return (uint32_t)((GetPageForRegister(mcu, Rn) << 16) | (uint16_t)(mcu.r[Rn] + instr.ea_disp));
}

constexpr uint32_t LoadEA(Mode_Ad16_Rn, mcu_t& mcu, uint8_t Rn, const I_CachedInstruction& instr)
{
    return (uint32_t)((GetPageForRegister(mcu, Rn) << 16) | (uint16_t)(mcu.r[Rn] + instr.ea_disp));
}

constexpr uint32_t LoadEA(Mode_APreDecRn, mcu_t& mcu, uint8_t Rn, const I_CachedInstruction& instr)
{
    (void)instr;
    return (uint32_t)((GetPageForRegister(mcu, Rn) << 16) | mcu.r[Rn]);
}

constexpr uint32_t LoadEA(Mode_APostIncRn, mcu_t& mcu, uint8_t Rn, const I_CachedInstruction& instr)
{
    (void)instr;
    return (uint32_t)((GetPageForRegister(mcu, Rn) << 16) | mcu.r[Rn]);
}

constexpr uint32_t LoadEA(Mode_ARn, mcu_t& mcu, uint8_t Rn, const I_CachedInstruction& instr)
{
    (void)instr;
    return (uint32_t)((GetPageForRegister(mcu, Rn) << 16) | mcu.r[Rn]);
}

constexpr uint32_t LoadEA(Mode_Aaa8, const mcu_t& mcu, uint8_t Rn, const I_CachedInstruction& instr)
{
    (void)Rn;
    return (uint32_t)(mcu.br << 8) | instr.ea_data;
}

constexpr uint32_t LoadEA(Mode_Aaa16, const mcu_t& mcu, uint8_t Rn, const I_CachedInstruction& instr)
{
    (void)Rn;
    return (uint32_t)(mcu.dp << 16) | instr.ea_data;
}

///////////////////////////////////////////////////////////////////////////////
// Performs a load from EA treating EA as if it were a pointer. This may load
// from a register or a memory location, but this function abstracts over the
// exact method. To obtain the pointer itself, use LoadEA instead.
template <MCU_Operand_Size Sz>
auto LoadFromEA(Mode_Rn, mcu_t& mcu, const I_CachedInstruction& instr)
{
    return LoadFromReg<Sz>(mcu, instr.ea_reg);
}

template <MCU_Operand_Size Sz>
    requires(Sz == MCU_Operand_Size::BYTE)
uint8_t LoadFromEA(Mode_Imm8, mcu_t& mcu, const I_CachedInstruction& instr)
{
    (void)mcu;
    return (uint8_t)instr.ea_data;
}

template <MCU_Operand_Size Sz>
    requires(Sz == MCU_Operand_Size::WORD)
uint16_t LoadFromEA(Mode_Imm16, mcu_t& mcu, const I_CachedInstruction& instr)
{
    (void)mcu;
    return instr.ea_data;
}

template <MCU_Operand_Size Sz, typename Mode>
typename MCU_Operand_Size_Int<Sz>::Type LoadFromEA(Mode, mcu_t& mcu, const I_CachedInstruction& st)
{
    (void)st;
    const uint32_t addr = LoadEA(Mode{}, mcu, st.ea_reg, st);
    if constexpr (Sz == MCU_Operand_Size::BYTE)
        return MCU_Read(mcu, addr);
    else if constexpr (Sz == MCU_Operand_Size::WORD)
        return MCU_Read16(mcu, addr);
}

///////////////////////////////////////////////////////////////////////////////
// Performs a store to EA treating EA as if it were a pointer. This may store
// to a register or a memory location, but this function abstracts over the
// exact method. To obtain the pointer used for the store, use LoadEA.
template <MCU_Operand_Size Sz>
void StoreToEA(Mode_Rn, mcu_t& mcu, const I_CachedInstruction& st, typename MCU_Operand_Size_Int<Sz>::Type value)
{
    StoreToReg<Sz>(mcu, st.ea_reg, value);
}

template <MCU_Operand_Size Sz, typename Mode>
    requires(!std::is_same_v<Mode, Mode_Rn>)
void StoreToEA(Mode, mcu_t& mcu, const I_CachedInstruction& st, typename MCU_Operand_Size_Int<Sz>::Type value)
{
    const uint32_t addr = LoadEA(Mode{}, mcu, st.ea_reg, st);
    if constexpr (Sz == MCU_Operand_Size::BYTE)
        MCU_Write(mcu, addr, value);
    else if constexpr (Sz == MCU_Operand_Size::WORD)
        MCU_Write16(mcu, addr, value);
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
    fprintf(stderr, "I_WriteControlRegisterW: id (%d) not handled\n", cr);
}

template <MCU_Operand_Size Sz>
inline void StoreToCR(mcu_t& mcu, uint8_t cr, typename MCU_Operand_Size_Int<Sz>::Type value)
{
    if constexpr (Sz == MCU_Operand_Size::BYTE)
        StoreToCR_B(mcu, cr, value);
    else if constexpr (Sz == MCU_Operand_Size::WORD)
        StoreToCR_W(mcu, cr, value);
}

template <MCU_Operand_Size Sz>
inline typename MCU_Operand_Size_Int<Sz>::Type LoadFromCR(mcu_t& mcu, uint8_t cr)
{
    if constexpr (Sz == MCU_Operand_Size::BYTE)
        return LoadFromCR_B(mcu, cr);
    else if constexpr (Sz == MCU_Operand_Size::WORD)
        return LoadFromCR_W(mcu, cr);
}

template <MCU_Operand_Size Sz, typename State>
typename MCU_Operand_Size_Int<Sz>::Type LoadFromOpData(mcu_t& mcu, const State& st)
{
    (void)mcu;
    if constexpr (Sz == MCU_Operand_Size::BYTE)
        return (uint8_t)st.op_data;
    else if constexpr (Sz == MCU_Operand_Size::WORD)
        return st.op_data;
}

template <MCU_Operand_Size Sz>
void StoreToOpReg(mcu_t& mcu, const I_CachedInstruction& st, typename MCU_Operand_Size_Int<Sz>::Type value)
{
    StoreToReg<Sz>(mcu, st.op_reg, value);
}

template <MCU_Operand_Size Sz>
typename MCU_Operand_Size_Int<Sz>::Type LoadFromOpReg(mcu_t& mcu, const I_CachedInstruction& st)
{
    return LoadFromReg<Sz>(mcu, st.op_reg);
}

// CMP:G.B <EAs>, Rd
template <typename State>
inline void I_CMP_G_B_EAs_Rd(mcu_t& mcu, const I_CachedInstruction& st)
{
    InstructionScope<MCU_Operand_Size::BYTE, State> scope(mcu, st, 1);

    const uint8_t d_lo  = LoadFromOpReg<MCU_Operand_Size::BYTE>(mcu, st);
    const uint8_t ea_lo = LoadFromEA<MCU_Operand_Size::BYTE>(State{}, mcu, st);

    const uint16_t result_u = d_lo - ea_lo;
    const int16_t  result_s = (int8_t)d_lo - (int8_t)ea_lo;

    MCU_SetStatus(mcu, result_u & 0x80, STATUS_N);
    MCU_SetStatus(mcu, (uint8_t)result_u == 0, STATUS_Z);
    MCU_SetStatus(mcu, result_s < INT8_MIN || result_s > INT8_MAX, STATUS_V);
    MCU_SetStatus(mcu, result_u & 0x100, STATUS_C);
}

// CMP:G.W <EAs>, Rd
template <typename State>
inline void I_CMP_G_W_EAs_Rd(mcu_t& mcu, const I_CachedInstruction& st)
{
    InstructionScope<MCU_Operand_Size::WORD, State> scope(mcu, st, 1);

    const uint16_t d_word  = LoadFromOpReg<MCU_Operand_Size::WORD>(mcu, st);
    const uint16_t ea_word = LoadFromEA<MCU_Operand_Size::WORD>(State{}, mcu, st);

    const uint32_t result_u = d_word - ea_word;
    const int32_t  result_s = (int16_t)d_word - (int16_t)ea_word;

    MCU_SetStatus(mcu, result_u & 0x8000, STATUS_N);
    MCU_SetStatus(mcu, (uint16_t)result_u == 0, STATUS_Z);
    MCU_SetStatus(mcu, result_s < INT16_MIN || result_s > INT16_MAX, STATUS_V);
    MCU_SetStatus(mcu, result_u & 0x10000, STATUS_C);
}

// CMP:G.B #xx:8, <EAd>
template <typename State>
inline void I_CMP_G_B_imm8_EAd(mcu_t& mcu, const I_CachedInstruction& st)
{
    InstructionScope<MCU_Operand_Size::BYTE, State> scope(mcu, st, 2);

    const uint8_t ea_byte  = LoadFromEA<MCU_Operand_Size::BYTE>(State{}, mcu, st);
    const uint8_t imm_byte = LoadFromOpData<MCU_Operand_Size::BYTE>(mcu, st);

    const uint16_t result_u = ea_byte - imm_byte;
    const int16_t  result_s = (int8_t)ea_byte - (int8_t)imm_byte;

    MCU_SetStatus(mcu, result_u & 0x80, STATUS_N);
    MCU_SetStatus(mcu, (uint8_t)result_u == 0, STATUS_Z);
    MCU_SetStatus(mcu, result_s < INT8_MIN || result_s > INT8_MAX, STATUS_V);
    MCU_SetStatus(mcu, result_u & 0x100, STATUS_C);
}

// CMP:G.W #xx:8, <EAd>
template <typename State>
inline void I_CMP_G_W_imm8_EAd(mcu_t& mcu, const I_CachedInstruction& st)
{
    InstructionScope<MCU_Operand_Size::WORD, State> scope(mcu, st, 2);

    const uint16_t ea_word  = LoadFromEA<MCU_Operand_Size::WORD>(State{}, mcu, st);
    const uint16_t imm_word = SX(LoadFromOpData<MCU_Operand_Size::BYTE>(mcu, st));

    const uint32_t result_u = ea_word - imm_word;
    const int32_t  result_s = (int16_t)ea_word - (int16_t)imm_word;

    MCU_SetStatus(mcu, result_u & 0x8000, STATUS_N);
    MCU_SetStatus(mcu, (uint16_t)result_u == 0, STATUS_Z);
    MCU_SetStatus(mcu, result_s < INT16_MIN || result_s > INT16_MAX, STATUS_V);
    MCU_SetStatus(mcu, result_u & 0x10000, STATUS_C);
}

// CMP:G.B #xx:16, <EAd>
template <typename State>
inline void I_CMP_G_B_imm16_EAd(mcu_t& mcu, const I_CachedInstruction& st)
{
    InstructionScope<MCU_Operand_Size::BYTE, State> scope(mcu, st, 3);

    const uint8_t ea_byte  = LoadFromEA<MCU_Operand_Size::BYTE>(State{}, mcu, st);
    const uint8_t imm_byte = (uint8_t)LoadFromOpData<MCU_Operand_Size::WORD>(mcu, st);

    const uint16_t result_u = ea_byte - imm_byte;
    const int16_t  result_s = (int8_t)ea_byte - (int8_t)imm_byte;

    MCU_SetStatus(mcu, result_u & 0x80, STATUS_N);
    MCU_SetStatus(mcu, (uint8_t)result_u == 0, STATUS_Z);
    MCU_SetStatus(mcu, result_s < INT8_MIN || result_s > INT8_MAX, STATUS_V);
    MCU_SetStatus(mcu, result_u & 0x100, STATUS_C);
}

// CMP:G.W #xx:16, <EAd>
template <typename State>
inline void I_CMP_G_W_imm16_EAd(mcu_t& mcu, const I_CachedInstruction& st)
{
    InstructionScope<MCU_Operand_Size::WORD, State> scope(mcu, st, 3);

    const uint16_t ea_word  = LoadFromEA<MCU_Operand_Size::WORD>(State{}, mcu, st);
    const uint16_t imm_word = LoadFromOpData<MCU_Operand_Size::WORD>(mcu, st);

    const uint32_t result_u = ea_word - imm_word;
    const int32_t  result_s = (int16_t)ea_word - (int16_t)imm_word;

    MCU_SetStatus(mcu, result_u & 0x8000, STATUS_N);
    MCU_SetStatus(mcu, (uint16_t)result_u == 0, STATUS_Z);
    MCU_SetStatus(mcu, result_s < INT16_MIN || result_s > INT16_MAX, STATUS_V);
    MCU_SetStatus(mcu, result_u & 0x10000, STATUS_C);
}

// CLR.B <EAd>
template <typename State>
inline void I_CLR_B_EAd(mcu_t& mcu, const I_CachedInstruction& st)
{
    InstructionScope<MCU_Operand_Size::BYTE, State> scope(mcu, st, 1);

    StoreToEA<MCU_Operand_Size::BYTE>(State{}, mcu, st, 0);
    MCU_SetStatus(mcu, 0, STATUS_N);
    MCU_SetStatus(mcu, 1, STATUS_Z);
    MCU_SetStatus(mcu, 0, STATUS_V);
    MCU_SetStatus(mcu, 0, STATUS_C);
}

// CLR.W <EAd>
template <typename State>
inline void I_CLR_W_EAd(mcu_t& mcu, const I_CachedInstruction& st)
{
    InstructionScope<MCU_Operand_Size::WORD, State> scope(mcu, st, 1);

    StoreToEA<MCU_Operand_Size::WORD>(State{}, mcu, st, 0);
    MCU_SetStatus(mcu, 0, STATUS_N);
    MCU_SetStatus(mcu, 1, STATUS_Z);
    MCU_SetStatus(mcu, 0, STATUS_V);
    MCU_SetStatus(mcu, 0, STATUS_C);
}

// ADD:G.B <EAs>, Rd
template <typename Mode>
void I_ADD_G_B_EAs_Rd(mcu_t& mcu, const I_CachedInstruction& st)
{
    InstructionScope<MCU_Operand_Size::BYTE, Mode> scope(mcu, st, 1);

    const uint8_t data    = LoadFromEA<MCU_Operand_Size::BYTE>(Mode{}, mcu, st);
    const uint8_t operand = LoadFromOpReg<MCU_Operand_Size::BYTE>(mcu, st);

    const uint16_t add_u = operand + data;
    const int32_t  add_s = (int8_t)operand + (int8_t)data;

    StoreToOpReg<MCU_Operand_Size::BYTE>(mcu, st, (uint8_t)add_u);
    MCU_SetStatus(mcu, add_u & 0x80, STATUS_N);
    MCU_SetStatus(mcu, (add_u & 0xff) == 0, STATUS_Z);
    MCU_SetStatus(mcu, add_s < INT8_MIN || add_s > INT8_MAX, STATUS_V);
    MCU_SetStatus(mcu, add_u & 0x100, STATUS_C);
}

// ADD:G.W <EAs>, Rd
template <typename Mode>
void I_ADD_G_W_EAs_Rd(mcu_t& mcu, const I_CachedInstruction& st)
{
    InstructionScope<MCU_Operand_Size::WORD, Mode> scope(mcu, st, 1);

    const uint16_t data    = LoadFromEA<MCU_Operand_Size::WORD>(Mode{}, mcu, st);
    const uint16_t operand = LoadFromOpReg<MCU_Operand_Size::WORD>(mcu, st);

    const uint32_t add_u = operand + data;
    const int32_t  add_s = (int16_t)operand + (int16_t)data;

    StoreToOpReg<MCU_Operand_Size::WORD>(mcu, st, (uint16_t)add_u);
    MCU_SetStatus(mcu, add_u & 0x8000, STATUS_N);
    MCU_SetStatus(mcu, add_u == 0, STATUS_Z);
    MCU_SetStatus(mcu, add_s < INT16_MIN || add_s > INT16_MAX, STATUS_V);
    MCU_SetStatus(mcu, add_u & 0x10000, STATUS_C);
}

template <typename Mode>
inline void I_BSET_B_imm4_EAd(mcu_t& mcu, const I_CachedInstruction& st)
{
    InstructionScope<MCU_Operand_Size::BYTE, Mode> scope(mcu, st, 1);

    const uint8_t mask   = (uint8_t)(1 << st.op_data);
    const uint8_t data   = LoadFromEA<MCU_Operand_Size::BYTE>(Mode{}, mcu, st);
    const uint8_t result = data | mask;
    const bool    Z      = (data & mask) == 0;
    StoreToEA<MCU_Operand_Size::BYTE>(Mode{}, mcu, st, result);
    MCU_SetStatus(mcu, Z, STATUS_Z);
}

template <typename Mode>
inline void I_BSET_W_imm4_EAd(mcu_t& mcu, const I_CachedInstruction& st)
{
    InstructionScope<MCU_Operand_Size::WORD, Mode> scope(mcu, st, 1);

    const uint16_t mask   = (uint16_t)(1 << st.op_data);
    const uint16_t data   = LoadFromEA<MCU_Operand_Size::WORD>(Mode{}, mcu, st);
    const uint16_t result = data | mask;
    const bool     Z      = (data & mask) == 0;
    StoreToEA<MCU_Operand_Size::WORD>(Mode{}, mcu, st, result);
    MCU_SetStatus(mcu, Z, STATUS_Z);
}

template <typename Mode>
inline void I_BSET_B_Rs_EAd(mcu_t& mcu, const I_CachedInstruction& st)
{
    InstructionScope<MCU_Operand_Size::BYTE, Mode> scope(mcu, st, 1);

    const uint16_t bit    = LoadFromOpReg<MCU_Operand_Size::WORD>(mcu, st) & 0b1111;
    const uint8_t  mask   = (uint8_t)(1 << bit);
    const uint8_t  data   = LoadFromEA<MCU_Operand_Size::BYTE>(Mode{}, mcu, st);
    const uint8_t  result = data | mask;
    const bool     Z      = (data & mask) == 0;
    StoreToEA<MCU_Operand_Size::BYTE>(Mode{}, mcu, st, result);
    MCU_SetStatus(mcu, Z, STATUS_Z);
}

template <typename Mode>
inline void I_BSET_W_Rs_EAd(mcu_t& mcu, const I_CachedInstruction& st)
{
    InstructionScope<MCU_Operand_Size::WORD, Mode> scope(mcu, st, 1);

    const uint16_t bit    = LoadFromOpReg<MCU_Operand_Size::WORD>(mcu, st) & 0b1111;
    const uint16_t mask   = (uint16_t)(1 << bit);
    const uint16_t data   = LoadFromEA<MCU_Operand_Size::WORD>(Mode{}, mcu, st);
    const uint16_t result = data | mask;
    const bool     Z      = (data & mask) == 0;
    StoreToEA<MCU_Operand_Size::WORD>(Mode{}, mcu, st, result);
    MCU_SetStatus(mcu, Z, STATUS_Z);
}

template <typename Mode>
inline void I_BNOT_B_imm4_EAd(mcu_t& mcu, const I_CachedInstruction& st)
{
    InstructionScope<MCU_Operand_Size::BYTE, Mode> scope(mcu, st, 1);

    const uint8_t mask   = (uint8_t)(1 << st.op_data);
    const uint8_t data   = LoadFromEA<MCU_Operand_Size::BYTE>(Mode{}, mcu, st);
    const uint8_t result = data ^ mask;
    const bool    Z      = (data & mask) == 0;
    StoreToEA<MCU_Operand_Size::BYTE>(Mode{}, mcu, st, result);
    MCU_SetStatus(mcu, Z, STATUS_Z);
}

template <typename Mode>
inline void I_BNOT_W_imm4_EAd(mcu_t& mcu, const I_CachedInstruction& st)
{
    InstructionScope<MCU_Operand_Size::WORD, Mode> scope(mcu, st, 1);

    const uint16_t mask   = (uint16_t)(1 << st.op_data);
    const uint16_t data   = LoadFromEA<MCU_Operand_Size::WORD>(Mode{}, mcu, st);
    const uint16_t result = data ^ mask;
    const bool     Z      = (data & mask) == 0;
    StoreToEA<MCU_Operand_Size::WORD>(Mode{}, mcu, st, result);
    MCU_SetStatus(mcu, Z, STATUS_Z);
}

// BCLR.B #xx, <EAd>
template <typename State>
inline void I_BCLR_B_imm4_EAd(mcu_t& mcu, const I_CachedInstruction& st)
{
    InstructionScope<MCU_Operand_Size::BYTE, State> scope(mcu, st, 1);

    const uint8_t mask = (uint8_t)(1 << st.op_data);
    const uint8_t data = LoadFromEA<MCU_Operand_Size::BYTE>(State{}, mcu, st);
    MCU_SetStatus(mcu, (data & mask) == 0, STATUS_Z);
    StoreToEA<MCU_Operand_Size::BYTE>(State{}, mcu, st, data & (~mask));
}

// BCLR.W #xx, <EAd>
template <typename State>
inline void I_BCLR_W_imm4_EAd(mcu_t& mcu, const I_CachedInstruction& st)
{
    InstructionScope<MCU_Operand_Size::WORD, State> scope(mcu, st, 1);

    const uint16_t mask = (uint16_t)(1 << st.op_data);
    const uint16_t data = LoadFromEA<MCU_Operand_Size::WORD>(State{}, mcu, st);
    MCU_SetStatus(mcu, (data & mask) == 0, STATUS_Z);
    StoreToEA<MCU_Operand_Size::WORD>(State{}, mcu, st, data & (~mask));
}

// BCLR.B Rs,<EAd>
template <typename State>
inline void I_BCLR_B_Rs_EAd(mcu_t& mcu, const I_CachedInstruction& st)
{
    InstructionScope<MCU_Operand_Size::BYTE, State> scope(mcu, st, 1);

    const uint16_t bit  = LoadFromOpReg<MCU_Operand_Size::WORD>(mcu, st) & 0b1111;
    const uint8_t  mask = (uint8_t)(1 << bit);
    const uint8_t  data = LoadFromEA<MCU_Operand_Size::BYTE>(State{}, mcu, st);
    MCU_SetStatus(mcu, (data & mask) == 0, STATUS_Z);
    StoreToEA<MCU_Operand_Size::BYTE>(State{}, mcu, st, data & (~mask));
}

// BCLR.W Rs,<EAd>
template <typename State>
inline void I_BCLR_W_Rs_EAd(mcu_t& mcu, const I_CachedInstruction& st)
{
    InstructionScope<MCU_Operand_Size::WORD, State> scope(mcu, st, 1);

    const uint16_t bit  = LoadFromOpReg<MCU_Operand_Size::WORD>(mcu, st) & 0b1111;
    const uint16_t mask = (uint16_t)(1 << bit);
    const uint16_t data = LoadFromEA<MCU_Operand_Size::WORD>(State{}, mcu, st);
    MCU_SetStatus(mcu, (data & mask) == 0, STATUS_Z);
    StoreToEA<MCU_Operand_Size::WORD>(State{}, mcu, st, data & (~mask));
}

template <typename Mode>
inline void I_BTST_B_imm4_EAd(mcu_t& mcu, const I_CachedInstruction& st)
{
    InstructionScope<MCU_Operand_Size::BYTE, Mode> scope(mcu, st, 1);

    const uint8_t mask = (uint8_t)(1 << st.op_data);
    const uint8_t data = LoadFromEA<MCU_Operand_Size::BYTE>(Mode{}, mcu, st);
    const bool    Z    = (data & mask) == 0;
    MCU_SetStatus(mcu, Z, STATUS_Z);
}

template <typename Mode>
inline void I_BTST_W_imm4_EAd(mcu_t& mcu, const I_CachedInstruction& st)
{
    InstructionScope<MCU_Operand_Size::WORD, Mode> scope(mcu, st, 1);

    const uint16_t mask = (uint16_t)(1 << st.op_data);
    const uint16_t data = LoadFromEA<MCU_Operand_Size::WORD>(Mode{}, mcu, st);
    const bool     Z    = (data & mask) == 0;
    MCU_SetStatus(mcu, Z, STATUS_Z);
}

template <typename Mode>
inline void I_BTST_B_Rs_EAd(mcu_t& mcu, const I_CachedInstruction& st)
{
    InstructionScope<MCU_Operand_Size::BYTE, Mode> scope(mcu, st, 1);

    const uint8_t data  = LoadFromEA<MCU_Operand_Size::BYTE>(Mode{}, mcu, st);
    const uint8_t shift = LoadFromOpReg<MCU_Operand_Size::BYTE>(mcu, st) & 0b1111;
    const uint8_t mask  = (uint8_t)(1 << shift);
    const bool    Z     = (data & mask) == 0;
    MCU_SetStatus(mcu, Z, STATUS_Z);
}

template <typename Mode>
inline void I_BTST_W_Rs_EAd(mcu_t& mcu, const I_CachedInstruction& st)
{
    InstructionScope<MCU_Operand_Size::WORD, Mode> scope(mcu, st, 1);

    const uint16_t data  = LoadFromEA<MCU_Operand_Size::WORD>(Mode{}, mcu, st);
    const uint8_t  shift = LoadFromOpReg<MCU_Operand_Size::WORD>(mcu, st) & 0b1111;
    const uint16_t mask  = (uint16_t)(1 << shift);
    const bool     Z     = (data & mask) == 0;
    MCU_SetStatus(mcu, Z, STATUS_Z);
}

// MULXU.B <EAs>, Rd
template <typename State>
inline void I_MULXU_B_EAs_Rd(mcu_t& mcu, const I_CachedInstruction& st)
{
    InstructionScope<MCU_Operand_Size::BYTE, State> scope(mcu, st, 1);

    const uint8_t  data   = LoadFromEA<MCU_Operand_Size::BYTE>(State{}, mcu, st);
    const uint16_t result = data * LoadFromOpReg<MCU_Operand_Size::BYTE>(mcu, st);
    mcu.r[st.op_reg]      = result;
    MCU_SetStatus(mcu, result & 0x8000, STATUS_N);
    MCU_SetStatus(mcu, result == 0, STATUS_Z);
    MCU_SetStatus(mcu, 0, STATUS_V);
    MCU_SetStatus(mcu, 0, STATUS_C);
}

// MULXU.X <EAs>, Rd
template <typename State>
inline void I_MULXU_X_EAs_Rd(mcu_t& mcu, const I_CachedInstruction& st)
{
    InstructionScope<MCU_Operand_Size::WORD, State> scope(mcu, st, 1);

    const uint16_t data   = LoadFromEA<MCU_Operand_Size::WORD>(State{}, mcu, st);
    const uint32_t result = data * LoadFromOpReg<MCU_Operand_Size::WORD>(mcu, st);
    mcu.r[st.op_reg + 0]  = (uint16_t)(result >> 16);
    mcu.r[st.op_reg + 1]  = (uint16_t)result;
    MCU_SetStatus(mcu, result & 0x80000000, STATUS_N);
    MCU_SetStatus(mcu, result == 0, STATUS_Z);
    MCU_SetStatus(mcu, 0, STATUS_V);
    MCU_SetStatus(mcu, 0, STATUS_C);
}

// DIVXU.B <EAs>, Rd
template <typename State>
inline void I_DIVXU_B_EAs_Rd(mcu_t& mcu, const I_CachedInstruction& st)
{
    InstructionScope<MCU_Operand_Size::BYTE, State> scope(mcu, st, 1);

    const uint16_t op_value = LoadFromOpReg<MCU_Operand_Size::WORD>(mcu, st);

    const uint8_t d = LoadFromEA<MCU_Operand_Size::BYTE>(State{}, mcu, st);

    if (d == 0)
    {
        // TODO exception
        MCU_SetStatus(mcu, 0, STATUS_N);
        MCU_SetStatus(mcu, 1, STATUS_Z);
        MCU_SetStatus(mcu, 0, STATUS_V);
        MCU_SetStatus(mcu, 0, STATUS_C);
        return;
    }

    const uint16_t q = op_value / d;
    const uint16_t r = op_value % d;

    if (q > UINT8_MAX)
    {
        MCU_SetStatus(mcu, 0, STATUS_N);
        MCU_SetStatus(mcu, 0, STATUS_Z);
        MCU_SetStatus(mcu, 1, STATUS_V);
        MCU_SetStatus(mcu, 0, STATUS_C);
        return;
    }

    const uint16_t result = (uint16_t)((r << 8) | q);

    mcu.r[st.op_reg] = result;

    MCU_SetStatus(mcu, q & 0x80, STATUS_N);
    MCU_SetStatus(mcu, q == 0, STATUS_Z);
    MCU_SetStatus(mcu, 0, STATUS_V);
    MCU_SetStatus(mcu, 0, STATUS_C);
}

// DIVXU.W <EAs>, Rd
template <typename State>
inline void I_DIVXU_W_EAs_Rd(mcu_t& mcu, const I_CachedInstruction& st)
{
    InstructionScope<MCU_Operand_Size::WORD, State> scope(mcu, st, 1);

    const uint32_t op_value = static_cast<uint32_t>((mcu.r[st.op_reg] << 16) | mcu.r[st.op_reg + 1]);

    const uint16_t d = LoadFromEA<MCU_Operand_Size::WORD>(State{}, mcu, st);

    if (d == 0)
    {
        // TODO exception
        MCU_SetStatus(mcu, 0, STATUS_N);
        MCU_SetStatus(mcu, 1, STATUS_Z);
        MCU_SetStatus(mcu, 0, STATUS_V);
        MCU_SetStatus(mcu, 0, STATUS_C);
        return;
    }

    const uint32_t q = op_value / d;
    const uint32_t r = op_value % d;

    if (q > UINT16_MAX)
    {
        MCU_SetStatus(mcu, 0, STATUS_N);
        MCU_SetStatus(mcu, 0, STATUS_Z);
        MCU_SetStatus(mcu, 1, STATUS_V);
        MCU_SetStatus(mcu, 0, STATUS_C);
        return;
    }

    mcu.r[st.op_reg + 0] = (uint16_t)r;
    mcu.r[st.op_reg + 1] = (uint16_t)q;

    MCU_SetStatus(mcu, q & 0x8000, STATUS_N);
    MCU_SetStatus(mcu, q == 0, STATUS_Z);
    MCU_SetStatus(mcu, 0, STATUS_V);
    MCU_SetStatus(mcu, 0, STATUS_C);
}

//=============================================================================
// General instruction handlers
//=============================================================================

// MOV:G.B <EAs>, Rd
template <typename State>
inline void I_MOV_G_B_EAs_Rd(mcu_t& mcu, const I_CachedInstruction& st)
{
    InstructionScope<MCU_Operand_Size::BYTE, State> scope(mcu, st, 1);

    const uint8_t data = LoadFromEA<MCU_Operand_Size::BYTE>(State{}, mcu, st);
    StoreToOpReg<MCU_Operand_Size::BYTE>(mcu, st, data);
    MCU_SetStatus(mcu, data & 0x80, STATUS_N);
    MCU_SetStatus(mcu, data == 0, STATUS_Z);
    MCU_SetStatus(mcu, 0, STATUS_V);
}

// MOV:G.W <EAs>, Rd
template <typename State>
inline void I_MOV_G_W_EAs_Rd(mcu_t& mcu, const I_CachedInstruction& st)
{
    InstructionScope<MCU_Operand_Size::WORD, State> scope(mcu, st, 1);

    const uint16_t data = LoadFromEA<MCU_Operand_Size::WORD>(State{}, mcu, st);
    StoreToOpReg<MCU_Operand_Size::WORD>(mcu, st, data);
    MCU_SetStatus(mcu, data & 0x8000, STATUS_N);
    MCU_SetStatus(mcu, data == 0, STATUS_Z);
    MCU_SetStatus(mcu, 0, STATUS_V);
}

// MOV:G.B Rs, <EAd>
template <typename Mode>
inline void I_MOV_G_B_Rs_EAd(mcu_t& mcu, const I_CachedInstruction& st)
{
    InstructionScope<MCU_Operand_Size::BYTE, Mode> scope(mcu, st, 1);

    const uint8_t data = LoadFromOpReg<MCU_Operand_Size::BYTE>(mcu, st);
    StoreToEA<MCU_Operand_Size::BYTE>(Mode{}, mcu, st, data);
    MCU_SetStatus(mcu, data & 0x80, STATUS_N);
    MCU_SetStatus(mcu, data == 0, STATUS_Z);
    MCU_SetStatus(mcu, 0, STATUS_V);
}

// MOV:G.W Rs, <EAd>
template <typename Mode>
inline void I_MOV_G_W_Rs_EAd(mcu_t& mcu, const I_CachedInstruction& st)
{
    InstructionScope<MCU_Operand_Size::WORD, Mode> scope(mcu, st, 1);

    const uint16_t data = LoadFromOpReg<MCU_Operand_Size::WORD>(mcu, st);
    StoreToEA<MCU_Operand_Size::WORD>(Mode{}, mcu, st, data);
    MCU_SetStatus(mcu, data & 0x8000, STATUS_N);
    MCU_SetStatus(mcu, data == 0, STATUS_Z);
    MCU_SetStatus(mcu, 0, STATUS_V);
}

// MOV:G.B #xx:8, <EAd>
template <typename State>
inline void I_MOV_G_B_imm8_EAd(mcu_t& mcu, const I_CachedInstruction& st)
{
    InstructionScope<MCU_Operand_Size::BYTE, State> scope(mcu, st, 2);

    const uint8_t data = LoadFromOpData<MCU_Operand_Size::BYTE>(mcu, st);
    StoreToEA<MCU_Operand_Size::BYTE>(State{}, mcu, st, data);
    MCU_SetStatus(mcu, data & 0x80, STATUS_N);
    MCU_SetStatus(mcu, data == 0, STATUS_Z);
    MCU_SetStatus(mcu, 0, STATUS_V);
}

// MOV:G.W #xx:8, <EAd>
template <typename State>
inline void I_MOV_G_W_imm8_EAd(mcu_t& mcu, const I_CachedInstruction& st)
{
    InstructionScope<MCU_Operand_Size::WORD, State> scope(mcu, st, 2);

    const uint16_t data_sx = SX(LoadFromOpData<MCU_Operand_Size::BYTE>(mcu, st));
    StoreToEA<MCU_Operand_Size::WORD>(State{}, mcu, st, data_sx);
    MCU_SetStatus(mcu, data_sx & 0x8000, STATUS_N);
    MCU_SetStatus(mcu, data_sx == 0, STATUS_Z);
    MCU_SetStatus(mcu, 0, STATUS_V);
}

// MOV:G.B #xx:16, <EAd>
template <typename State>
inline void I_MOV_G_B_imm16_EAd(mcu_t& mcu, const I_CachedInstruction& st)
{
    InstructionScope<MCU_Operand_Size::BYTE, State> scope(mcu, st, 3);

    const uint8_t data_lo = (uint8_t)LoadFromOpData<MCU_Operand_Size::WORD>(mcu, st);
    StoreToEA<MCU_Operand_Size::BYTE>(State{}, mcu, st, data_lo);
    MCU_SetStatus(mcu, data_lo & 0x80, STATUS_N);
    MCU_SetStatus(mcu, data_lo == 0, STATUS_Z);
    MCU_SetStatus(mcu, 0, STATUS_V);
}

// MOV:G.W #xx:16, <EAd>
template <typename State>
inline void I_MOV_G_W_imm16_EAd(mcu_t& mcu, const I_CachedInstruction& st)
{
    InstructionScope<MCU_Operand_Size::WORD, State> scope(mcu, st, 3);

    const uint16_t data = LoadFromOpData<MCU_Operand_Size::WORD>(mcu, st);
    StoreToEA<MCU_Operand_Size::WORD>(State{}, mcu, st, data);
    MCU_SetStatus(mcu, data & 0x8000, STATUS_N);
    MCU_SetStatus(mcu, data == 0, STATUS_Z);
    MCU_SetStatus(mcu, 0, STATUS_V);
}

// SWAP Rd
// This instruction defined for BYTE size despite operating on the entire register WORD.
template <typename Mode>
inline void I_SWAP_B_Rd(mcu_t& mcu, const I_CachedInstruction& st)
{
    InstructionScope<MCU_Operand_Size::BYTE, Mode> scope(mcu, st, 1);

    const uint16_t value = std::byteswap(mcu.r[st.ea_reg]);
    mcu.r[st.ea_reg]     = value;
    MCU_SetStatus(mcu, value & 0x8000, STATUS_N);
    MCU_SetStatus(mcu, value == 0, STATUS_Z);
    MCU_SetStatus(mcu, 0, STATUS_V);
}

// XCH Rs, Rd
// This instruction defined for WORD size only.
template <typename Mode>
inline void I_XCH_W_Rs_Rd(mcu_t& mcu, const I_CachedInstruction& instr)
{
    InstructionScope<MCU_Operand_Size::WORD, Mode> scope(mcu, instr, 1);

    const uint16_t tmp  = mcu.r[instr.ea_reg];
    mcu.r[instr.ea_reg] = mcu.r[instr.op_reg];
    mcu.r[instr.op_reg] = tmp;
}

// ADD:Q.B #1, <EAd>
// ADD:Q.B #2, <EAd>
// ADD:Q.B #-1, <EAd>
// ADD:Q.B #-2, <EAd>
template <typename State, int8_t N>
inline void I_ADD_Q_B_n(mcu_t& mcu, const I_CachedInstruction& st)
{
    InstructionScope<MCU_Operand_Size::BYTE, State> scope(mcu, st, 1);

    const uint8_t ea_byte = LoadFromEA<MCU_Operand_Size::BYTE>(State{}, mcu, st);

    const uint16_t result_u = ea_byte + (uint8_t)N;
    const int16_t  result_s = (int8_t)ea_byte + N;

    StoreToEA<MCU_Operand_Size::BYTE>(State{}, mcu, st, (uint8_t)result_u);
    MCU_SetStatus(mcu, result_u & 0x80, STATUS_N);
    MCU_SetStatus(mcu, (uint8_t)result_u == 0, STATUS_Z);
    MCU_SetStatus(mcu, result_s < INT8_MIN || result_s > INT8_MAX, STATUS_V);
    MCU_SetStatus(mcu, result_u & 0x100, STATUS_C);
}

// ADD:Q.W #1, <EAd>
// ADD:Q.W #2, <EAd>
// ADD:Q.W #-1, <EAd>
// ADD:Q.W #-2, <EAd>
template <typename State, int8_t N>
inline void I_ADD_Q_W_n(mcu_t& mcu, const I_CachedInstruction& st)
{
    InstructionScope<MCU_Operand_Size::WORD, State> scope(mcu, st, 1);

    const uint16_t ea_word = LoadFromEA<MCU_Operand_Size::WORD>(State{}, mcu, st);

    const uint32_t result_u = ea_word + (uint16_t)N;
    const int32_t  result_s = (int16_t)ea_word + N;

    StoreToEA<MCU_Operand_Size::WORD>(State{}, mcu, st, (uint16_t)result_u);
    MCU_SetStatus(mcu, result_u & 0x8000, STATUS_N);
    MCU_SetStatus(mcu, (uint16_t)result_u == 0, STATUS_Z);
    MCU_SetStatus(mcu, result_s < INT16_MIN || result_s > INT16_MAX, STATUS_V);
    MCU_SetStatus(mcu, result_u & 0x10000, STATUS_C);
}

template <typename State>
inline void I_ADDX_B_EAs_Rd(mcu_t& mcu, const I_CachedInstruction& st)
{
    InstructionScope<MCU_Operand_Size::BYTE, State> scope(mcu, st, 1);

    const bool old_C = mcu.sr & STATUS_C;
    const bool old_Z = mcu.sr & STATUS_Z;

    const uint8_t data    = LoadFromEA<MCU_Operand_Size::BYTE>(State{}, mcu, st);
    const uint8_t operand = LoadFromOpReg<MCU_Operand_Size::BYTE>(mcu, st);

    const uint16_t add_u = (uint16_t)(operand + data + old_C);
    const int16_t  add_s = (int16_t)((int8_t)operand + (int8_t)data + old_C);

    StoreToOpReg<MCU_Operand_Size::BYTE>(mcu, st, (uint8_t)add_u);
    MCU_SetStatus(mcu, add_u & 0x80, STATUS_N);
    MCU_SetStatus(mcu, old_Z && ((uint8_t)add_u == 0), STATUS_Z);
    MCU_SetStatus(mcu, add_s < INT8_MIN || add_s > INT8_MAX, STATUS_V);
    MCU_SetStatus(mcu, add_u & 0x100, STATUS_C);
}

template <typename State>
inline void I_ADDX_W_EAs_Rd(mcu_t& mcu, const I_CachedInstruction& st)
{
    InstructionScope<MCU_Operand_Size::WORD, State> scope(mcu, st, 1);

    const bool old_C = mcu.sr & STATUS_C;
    const bool old_Z = mcu.sr & STATUS_Z;

    const uint16_t data    = LoadFromEA<MCU_Operand_Size::WORD>(State{}, mcu, st);
    const uint16_t operand = LoadFromOpReg<MCU_Operand_Size::WORD>(mcu, st);

    const uint32_t add_u = operand + data + old_C;
    const int32_t  add_s = (int8_t)operand + (int8_t)data + old_C;

    StoreToOpReg<MCU_Operand_Size::WORD>(mcu, st, (uint16_t)add_u);
    MCU_SetStatus(mcu, add_u & 0x8000, STATUS_N);
    MCU_SetStatus(mcu, old_Z && ((uint16_t)add_u == 0), STATUS_Z);
    MCU_SetStatus(mcu, add_s < INT16_MIN || add_s > INT16_MAX, STATUS_V);
    MCU_SetStatus(mcu, add_u & 0x10000, STATUS_C);
}

template <typename State>
inline void I_SUB_B_EAs_Rd(mcu_t& mcu, const I_CachedInstruction& st)
{
    InstructionScope<MCU_Operand_Size::BYTE, State> scope(mcu, st, 1);

    const uint8_t EAs = LoadFromEA<MCU_Operand_Size::BYTE>(State{}, mcu, st);
    const uint8_t Rd  = LoadFromOpReg<MCU_Operand_Size::BYTE>(mcu, st);

    const uint16_t sub_u = Rd - EAs;
    const int16_t  sub_s = (int8_t)Rd - (int8_t)EAs;
    StoreToOpReg<MCU_Operand_Size::BYTE>(mcu, st, (uint8_t)sub_u);

    const bool N = sub_u & 0x80;
    const bool Z = sub_u == 0;
    const bool V = sub_s < INT8_MIN || sub_s > INT8_MAX;
    const bool C = sub_u & 0x100;
    MCU_SetStatus(mcu, N, STATUS_N);
    MCU_SetStatus(mcu, Z, STATUS_Z);
    MCU_SetStatus(mcu, V, STATUS_V);
    MCU_SetStatus(mcu, C, STATUS_C);
}

template <typename State>
inline void I_SUB_W_EAs_Rd(mcu_t& mcu, const I_CachedInstruction& st)
{
    InstructionScope<MCU_Operand_Size::WORD, State> scope(mcu, st, 1);

    const uint16_t EAs = LoadFromEA<MCU_Operand_Size::WORD>(State{}, mcu, st);
    const uint16_t Rd  = LoadFromOpReg<MCU_Operand_Size::WORD>(mcu, st);

    const uint32_t sub_u = Rd - EAs;
    const int32_t  sub_s = (int16_t)Rd - (int16_t)EAs;
    StoreToOpReg<MCU_Operand_Size::WORD>(mcu, st, (uint16_t)sub_u);

    const bool N = sub_u & 0x8000;
    const bool Z = sub_u == 0;
    const bool V = sub_s < INT16_MIN || sub_s > INT16_MAX;
    const bool C = sub_u & 0x10000;
    MCU_SetStatus(mcu, N, STATUS_N);
    MCU_SetStatus(mcu, Z, STATUS_Z);
    MCU_SetStatus(mcu, V, STATUS_V);
    MCU_SetStatus(mcu, C, STATUS_C);
}

template <typename State>
inline void I_SUBX_B_EAs_Rd(mcu_t& mcu, const I_CachedInstruction& st)
{
    InstructionScope<MCU_Operand_Size::BYTE, State> scope(mcu, st, 1);

    const uint8_t sub_C = (uint8_t)((mcu.sr & STATUS_C) != 0);
    const uint8_t EAs   = LoadFromEA<MCU_Operand_Size::BYTE>(State{}, mcu, st);
    const uint8_t Rd    = LoadFromOpReg<MCU_Operand_Size::BYTE>(mcu, st);

    const uint16_t sub_u = (uint16_t)(Rd - EAs - sub_C);
    const int16_t  sub_s = (int16_t)((int8_t)Rd - (int8_t)EAs - (int8_t)sub_C);
    StoreToOpReg<MCU_Operand_Size::BYTE>(mcu, st, (uint8_t)sub_u);

    const bool N = sub_u & 0x80;
    const bool Z = sub_u == 0;
    const bool V = sub_s < INT8_MIN || sub_s > INT8_MAX;
    const bool C = sub_u & 0x100;
    MCU_SetStatus(mcu, N, STATUS_N);
    MCU_SetStatus(mcu, Z, STATUS_Z);
    MCU_SetStatus(mcu, V, STATUS_V);
    MCU_SetStatus(mcu, C, STATUS_C);
}

template <typename State>
inline void I_SUBX_W_EAs_Rd(mcu_t& mcu, const I_CachedInstruction& st)
{
    InstructionScope<MCU_Operand_Size::WORD, State> scope(mcu, st, 1);

    const uint8_t  sub_C = (uint8_t)((mcu.sr & STATUS_C) != 0);
    const uint16_t EAs   = LoadFromEA<MCU_Operand_Size::WORD>(State{}, mcu, st);
    const uint16_t Rd    = LoadFromOpReg<MCU_Operand_Size::WORD>(mcu, st);

    const uint32_t sub_u = Rd - EAs - sub_C;
    const int32_t  sub_s = (int16_t)Rd - (int16_t)EAs - (int16_t)sub_C;
    StoreToOpReg<MCU_Operand_Size::WORD>(mcu, st, (uint16_t)sub_u);

    const bool N = sub_u & 0x8000;
    const bool Z = sub_u == 0;
    const bool V = sub_s < INT16_MIN || sub_s > INT16_MAX;
    const bool C = sub_u & 0x10000;
    MCU_SetStatus(mcu, N, STATUS_N);
    MCU_SetStatus(mcu, Z, STATUS_Z);
    MCU_SetStatus(mcu, V, STATUS_V);
    MCU_SetStatus(mcu, C, STATUS_C);
}

template <typename State>
inline void I_SUBS_B_EAs_Rd(mcu_t& mcu, const I_CachedInstruction& st)
{
    InstructionScope<MCU_Operand_Size::BYTE, State> scope(mcu, st, 1);

    const uint16_t EAs = SX(LoadFromEA<MCU_Operand_Size::BYTE>(State{}, mcu, st));
    // Rd always accessed as word in this form
    const uint16_t Rd = LoadFromOpReg<MCU_Operand_Size::WORD>(mcu, st);

    const uint16_t sub_u = Rd - EAs;
    StoreToOpReg<MCU_Operand_Size::WORD>(mcu, st, sub_u);
}

template <typename State>
inline void I_SUBS_W_EAs_Rd(mcu_t& mcu, const I_CachedInstruction& st)
{
    InstructionScope<MCU_Operand_Size::WORD, State> scope(mcu, st, 1);

    const uint16_t EAs = LoadFromEA<MCU_Operand_Size::WORD>(State{}, mcu, st);
    const uint16_t Rd  = LoadFromOpReg<MCU_Operand_Size::WORD>(mcu, st);

    const uint32_t sub_u = Rd - EAs;
    StoreToOpReg<MCU_Operand_Size::WORD>(mcu, st, (uint16_t)sub_u);
}

template <typename State>
inline void I_TST_B_EAd(mcu_t& mcu, const I_CachedInstruction& st)
{
    InstructionScope<MCU_Operand_Size::BYTE, State> scope(mcu, st, 1);

    const uint8_t value = LoadFromEA<MCU_Operand_Size::BYTE>(State{}, mcu, st);
    MCU_SetStatus(mcu, value & 0x80, STATUS_N);
    MCU_SetStatus(mcu, value == 0, STATUS_Z);
    MCU_SetStatus(mcu, 0, STATUS_V);
    MCU_SetStatus(mcu, 0, STATUS_C);
}

template <typename State>
inline void I_TST_W_EAd(mcu_t& mcu, const I_CachedInstruction& st)
{
    InstructionScope<MCU_Operand_Size::WORD, State> scope(mcu, st, 1);

    const uint16_t value = LoadFromEA<MCU_Operand_Size::WORD>(State{}, mcu, st);
    MCU_SetStatus(mcu, value & 0x8000, STATUS_N);
    MCU_SetStatus(mcu, value == 0, STATUS_Z);
    MCU_SetStatus(mcu, 0, STATUS_V);
    MCU_SetStatus(mcu, 0, STATUS_C);
}

template <typename State>
inline void I_NEG_B_EAd(mcu_t& mcu, const I_CachedInstruction& st)
{
    InstructionScope<MCU_Operand_Size::BYTE, State> scope(mcu, st, 1);

    const uint8_t value = LoadFromEA<MCU_Operand_Size::BYTE>(State{}, mcu, st);

    const uint16_t neg_u = -value;
    const int16_t  neg_s = -(int8_t)value;

    StoreToEA<MCU_Operand_Size::BYTE>(State{}, mcu, st, (uint8_t)neg_u);
    MCU_SetStatus(mcu, neg_u & 0x80, STATUS_N);
    MCU_SetStatus(mcu, neg_u == 0, STATUS_Z);
    MCU_SetStatus(mcu, neg_s < INT8_MIN || neg_s > INT8_MAX, STATUS_V);
    MCU_SetStatus(mcu, neg_u & 0x100, STATUS_C);
}

template <typename State>
inline void I_NEG_W_EAd(mcu_t& mcu, const I_CachedInstruction& st)
{
    InstructionScope<MCU_Operand_Size::WORD, State> scope(mcu, st, 1);

    const uint16_t value = LoadFromEA<MCU_Operand_Size::WORD>(State{}, mcu, st);

    const uint32_t neg_u = -value;
    const int32_t  neg_s = -(int16_t)value;

    StoreToEA<MCU_Operand_Size::WORD>(State{}, mcu, st, (uint16_t)neg_u);
    MCU_SetStatus(mcu, neg_u & 0x8000, STATUS_N);
    MCU_SetStatus(mcu, neg_u == 0, STATUS_Z);
    MCU_SetStatus(mcu, neg_s < INT16_MIN || neg_s > INT16_MAX, STATUS_V);
    MCU_SetStatus(mcu, neg_u & 0x10000, STATUS_C);
}

// SHLR.B <EAd>
template <typename State>
inline void I_SHLL_B_EAd(mcu_t& mcu, const I_CachedInstruction& st)
{
    InstructionScope<MCU_Operand_Size::BYTE, State> scope(mcu, st, 1);

    const uint8_t val_old = LoadFromEA<MCU_Operand_Size::BYTE>(State{}, mcu, st);
    const uint8_t val_new = (uint8_t)(val_old << 1);
    StoreToEA<MCU_Operand_Size::BYTE>(State{}, mcu, st, val_new);

    const bool N = val_new & 0x80;
    const bool Z = val_new == 0;
    const bool V = 0;
    const bool C = val_old & 0x80;

    MCU_SetStatus(mcu, N, STATUS_N);
    MCU_SetStatus(mcu, Z, STATUS_Z);
    MCU_SetStatus(mcu, V, STATUS_V);
    MCU_SetStatus(mcu, C, STATUS_C);
}

// SHLR.W <EAd>
template <typename State>
inline void I_SHLL_W_EAd(mcu_t& mcu, const I_CachedInstruction& st)
{
    InstructionScope<MCU_Operand_Size::WORD, State> scope(mcu, st, 1);

    const uint16_t val_old = LoadFromEA<MCU_Operand_Size::WORD>(State{}, mcu, st);
    const uint16_t val_new = (uint16_t)(val_old << 1);
    StoreToEA<MCU_Operand_Size::WORD>(State{}, mcu, st, val_new);

    const bool N = val_new & 0x8000;
    const bool Z = val_new == 0;
    const bool V = 0;
    const bool C = val_old & 0x8000;

    MCU_SetStatus(mcu, N, STATUS_N);
    MCU_SetStatus(mcu, Z, STATUS_Z);
    MCU_SetStatus(mcu, V, STATUS_V);
    MCU_SetStatus(mcu, C, STATUS_C);
}

// SHLR.B <EAd>
template <typename State>
inline void I_SHLR_B_EAd(mcu_t& mcu, const I_CachedInstruction& st)
{
    InstructionScope<MCU_Operand_Size::BYTE, State> scope(mcu, st, 1);

    const uint8_t val_old = LoadFromEA<MCU_Operand_Size::BYTE>(State{}, mcu, st);
    const uint8_t val_new = val_old >> 1;
    StoreToEA<MCU_Operand_Size::BYTE>(State{}, mcu, st, val_new);

    const bool N = 0;
    const bool Z = val_new == 0;
    const bool V = 0;
    const bool C = val_old & 1;

    MCU_SetStatus(mcu, N, STATUS_N);
    MCU_SetStatus(mcu, Z, STATUS_Z);
    MCU_SetStatus(mcu, V, STATUS_V);
    MCU_SetStatus(mcu, C, STATUS_C);
}

// SHLR.W <EAd>
template <typename State>
inline void I_SHLR_W_EAd(mcu_t& mcu, const I_CachedInstruction& st)
{
    InstructionScope<MCU_Operand_Size::WORD, State> scope(mcu, st, 1);

    const uint16_t val_old = LoadFromEA<MCU_Operand_Size::WORD>(State{}, mcu, st);
    const uint16_t val_new = val_old >> 1;
    StoreToEA<MCU_Operand_Size::WORD>(State{}, mcu, st, val_new);

    const bool N = 0;
    const bool Z = val_new == 0;
    const bool V = 0;
    const bool C = val_old & 1;

    MCU_SetStatus(mcu, N, STATUS_N);
    MCU_SetStatus(mcu, Z, STATUS_Z);
    MCU_SetStatus(mcu, V, STATUS_V);
    MCU_SetStatus(mcu, C, STATUS_C);
}

// LDC.B <EAs>, CR
template <typename State>
inline void I_LDC_B_EAs_CR(mcu_t& mcu, const I_CachedInstruction& st)
{
    InstructionScope<MCU_Operand_Size::BYTE, State> scope(mcu, st, 1);

    const uint8_t byte = LoadFromEA<MCU_Operand_Size::BYTE>(State{}, mcu, st);

    StoreToCR_B(mcu, st.op_c, byte);
    mcu.ex_ignore = 1;
}

// LDC.W <EAs>, CR
template <typename State>
inline void I_LDC_W_EAs_CR(mcu_t& mcu, const I_CachedInstruction& st)
{
    InstructionScope<MCU_Operand_Size::BYTE, State> scope(mcu, st, 1);

    const uint16_t word = LoadFromEA<MCU_Operand_Size::WORD>(State{}, mcu, st);

    StoreToCR_W(mcu, st.op_c, word);
    mcu.ex_ignore = 1;
}

// STC.B CR, <EAd>
template <typename State>
void I_STC_B_CR_EAd(mcu_t& mcu, const I_CachedInstruction& st)
{
    InstructionScope<MCU_Operand_Size::BYTE, State> scope(mcu, st, 1);

    const uint8_t value = LoadFromCR_B(mcu, st.op_c);
    StoreToEA<MCU_Operand_Size::BYTE>(State{}, mcu, st, value);
}

// STC.W CR, <EAd>
template <typename State>
void I_STC_W_CR_EAd(mcu_t& mcu, const I_CachedInstruction& st)
{
    InstructionScope<MCU_Operand_Size::WORD, State> scope(mcu, st, 1);

    const uint16_t value = LoadFromCR_W(mcu, st.op_c);
    StoreToEA<MCU_Operand_Size::WORD>(State{}, mcu, st, value);
}

// ANDC.B #xx:8, CR
template <typename Mode, uint8_t CR>
inline void I_ANDC_B_imm8_CR(mcu_t& mcu, const I_CachedInstruction& st)
{
    InstructionScope<MCU_Operand_Size::BYTE, Mode> scope(mcu, st, 1);

    const uint8_t result = LoadFromCR_B(mcu, CR) & (uint8_t)st.ea_data;
    StoreToCR_B(mcu, CR, result);
    mcu.ex_ignore = 1;

    const bool N = result & 0x80;
    const bool Z = result == 0;
    const bool V = 0;
    MCU_SetStatus(mcu, N, STATUS_N);
    MCU_SetStatus(mcu, Z, STATUS_Z);
    MCU_SetStatus(mcu, V, STATUS_V);
}

// ANDC.W #xx:16, CR
template <typename Mode, uint8_t CR>
inline void I_ANDC_W_imm16_CR(mcu_t& mcu, const I_CachedInstruction& st)
{
    InstructionScope<MCU_Operand_Size::WORD, Mode> scope(mcu, st, 1);

    const uint16_t result = LoadFromCR_W(mcu, CR) & st.ea_data;
    StoreToCR_W(mcu, CR, result);
    mcu.ex_ignore = 1;

    const bool N = result & 0x8000;
    const bool Z = result == 0;
    const bool V = 0;
    MCU_SetStatus(mcu, N, STATUS_N);
    MCU_SetStatus(mcu, Z, STATUS_Z);
    MCU_SetStatus(mcu, V, STATUS_V);
}

// ORC.B #xx:8, CR
template <typename Mode, uint8_t CR>
inline void I_ORC_B_imm8_CR(mcu_t& mcu, const I_CachedInstruction& st)
{
    InstructionScope<MCU_Operand_Size::BYTE, Mode> scope(mcu, st, 1);

    const uint8_t result = (uint8_t)(LoadFromCR_B(mcu, CR) | st.ea_data);
    StoreToCR_B(mcu, CR, result);
    mcu.ex_ignore = 1;

    const bool N = result & 0x80;
    const bool Z = result == 0;
    const bool V = 0;
    MCU_SetStatus(mcu, N, STATUS_N);
    MCU_SetStatus(mcu, Z, STATUS_Z);
    MCU_SetStatus(mcu, V, STATUS_V);
}

// ORC.W #xx:16, CR
template <typename Mode, uint8_t CR>
inline void I_ORC_W_imm16_CR(mcu_t& mcu, const I_CachedInstruction& st)
{
    InstructionScope<MCU_Operand_Size::WORD, Mode> scope(mcu, st, 1);

    const uint16_t result = LoadFromCR_W(mcu, CR) | st.ea_data;
    StoreToCR_W(mcu, CR, result);
    mcu.ex_ignore = 1;

    const bool N = result & 0x8000;
    const bool Z = result == 0;
    const bool V = 0;
    MCU_SetStatus(mcu, N, STATUS_N);
    MCU_SetStatus(mcu, Z, STATUS_Z);
    MCU_SetStatus(mcu, V, STATUS_V);
}

//=============================================================================
// End general instruction handlers
//=============================================================================

// CMP:E #xx:8,Rd
template <uint8_t R>
inline void I_CMP_E_imm8_Rd(mcu_t& mcu, const I_CachedInstruction& st)
{
    const uint8_t R_lo = (uint8_t)mcu.r[R];
    const uint8_t data = (uint8_t)st.op_data;

    const uint16_t result_u = R_lo - data;
    const int16_t  result_s = (int8_t)R_lo - (int8_t)data;

    MCU_SetStatus(mcu, result_u & 0x80, STATUS_N);
    MCU_SetStatus(mcu, (uint8_t)result_u == 0, STATUS_Z);
    MCU_SetStatus(mcu, result_s < INT8_MIN || result_s > INT8_MAX, STATUS_V);
    MCU_SetStatus(mcu, result_u & 0x100, STATUS_C);

    mcu.pc += 2;
}

void I_Bcc_d8_BRA(mcu_t& mcu, const I_CachedInstruction& st);
void I_Bcc_d8_BRN(mcu_t& mcu, const I_CachedInstruction& st);
void I_Bcc_d8_BHI(mcu_t& mcu, const I_CachedInstruction& st);
void I_Bcc_d8_BLS(mcu_t& mcu, const I_CachedInstruction& st);
void I_Bcc_d8_BCC(mcu_t& mcu, const I_CachedInstruction& st);
void I_Bcc_d8_BCS(mcu_t& mcu, const I_CachedInstruction& st);
void I_Bcc_d8_BNE(mcu_t& mcu, const I_CachedInstruction& st);
void I_Bcc_d8_BEQ(mcu_t& mcu, const I_CachedInstruction& st);
void I_Bcc_d8_BVC(mcu_t& mcu, const I_CachedInstruction& st);
void I_Bcc_d8_BVS(mcu_t& mcu, const I_CachedInstruction& st);
void I_Bcc_d8_BPL(mcu_t& mcu, const I_CachedInstruction& st);
void I_Bcc_d8_BMI(mcu_t& mcu, const I_CachedInstruction& st);
void I_Bcc_d8_BGE(mcu_t& mcu, const I_CachedInstruction& st);
void I_Bcc_d8_BLT(mcu_t& mcu, const I_CachedInstruction& st);
void I_Bcc_d8_BGT(mcu_t& mcu, const I_CachedInstruction& st);
void I_Bcc_d8_BLE(mcu_t& mcu, const I_CachedInstruction& st);

void I_RTE(mcu_t& mcu, const I_CachedInstruction& st);

void I_NOP(mcu_t&, const I_CachedInstruction&);

// EXTS Rd
// This instruction defined for BYTE size despite operating on the entire register WORD.
template <typename Mode>
inline void I_EXTS_B_Rd(mcu_t& mcu, const I_CachedInstruction& st)
{
    InstructionScope<MCU_Operand_Size::BYTE, Mode> scope(mcu, st, 1);

    mcu.r[st.ea_reg] = SX(static_cast<uint8_t>(mcu.r[st.ea_reg]));
    MCU_SetStatus(mcu, mcu.r[st.ea_reg] & 0x8000, STATUS_N);
    MCU_SetStatus(mcu, mcu.r[st.ea_reg] == 0, STATUS_Z);
    MCU_SetStatus(mcu, 0, STATUS_V);
    MCU_SetStatus(mcu, 0, STATUS_C);
}

// EXTU Rd
// This instruction defined for BYTE size despite operating on the entire register WORD.
template <typename Mode>
inline void I_EXTU_B_Rd(mcu_t& mcu, const I_CachedInstruction& st)
{
    InstructionScope<MCU_Operand_Size::BYTE, Mode> scope(mcu, st, 1);

    mcu.r[st.ea_reg] = (uint8_t)mcu.r[st.ea_reg];
    MCU_SetStatus(mcu, 0, STATUS_N);
    MCU_SetStatus(mcu, mcu.r[st.ea_reg] == 0, STATUS_Z);
    MCU_SetStatus(mcu, 0, STATUS_V);
    MCU_SetStatus(mcu, 0, STATUS_C);
}

template <typename Mode>
inline void I_NOT_B_EAd(mcu_t& mcu, const I_CachedInstruction& st)
{
    InstructionScope<MCU_Operand_Size::BYTE, Mode> scope(mcu, st, 1);

    const uint8_t value = ~LoadFromEA<MCU_Operand_Size::BYTE>(Mode{}, mcu, st);
    StoreToEA<MCU_Operand_Size::BYTE>(Mode{}, mcu, st, value);
    MCU_SetStatus(mcu, value & 0x80, STATUS_N);
    MCU_SetStatus(mcu, value == 0, STATUS_Z);
    MCU_SetStatus(mcu, 0, STATUS_V);
}

template <typename Mode>
inline void I_NOT_W_EAd(mcu_t& mcu, const I_CachedInstruction& st)
{
    InstructionScope<MCU_Operand_Size::WORD, Mode> scope(mcu, st, 1);

    const uint16_t value = ~LoadFromEA<MCU_Operand_Size::WORD>(Mode{}, mcu, st);
    StoreToEA<MCU_Operand_Size::WORD>(Mode{}, mcu, st, value);
    MCU_SetStatus(mcu, value & 0x8000, STATUS_N);
    MCU_SetStatus(mcu, value == 0, STATUS_Z);
    MCU_SetStatus(mcu, 0, STATUS_V);
}

template <typename Mode>
inline void I_ADDS_B_EAs_Rd(mcu_t& mcu, const I_CachedInstruction& st)
{
    InstructionScope<MCU_Operand_Size::BYTE, Mode> scope(mcu, st, 1);

    const uint16_t old_reg = LoadFromOpReg<MCU_Operand_Size::WORD>(mcu, st);
    const uint16_t value   = SX(LoadFromEA<MCU_Operand_Size::BYTE>(Mode{}, mcu, st));
    StoreToOpReg<MCU_Operand_Size::WORD>(mcu, st, old_reg + value);
}

template <typename Mode>
inline void I_ADDS_W_EAs_Rd(mcu_t& mcu, const I_CachedInstruction& st)
{
    InstructionScope<MCU_Operand_Size::WORD, Mode> scope(mcu, st, 1);

    const uint16_t old_reg = LoadFromOpReg<MCU_Operand_Size::WORD>(mcu, st);
    const uint16_t value   = LoadFromEA<MCU_Operand_Size::WORD>(Mode{}, mcu, st);
    StoreToOpReg<MCU_Operand_Size::WORD>(mcu, st, old_reg + value);
}

template <typename Mode>
inline void I_ROTL_B_EAd(mcu_t& mcu, const I_CachedInstruction& st)
{
    InstructionScope<MCU_Operand_Size::BYTE, Mode> scope(mcu, st, 1);

    const uint8_t input  = LoadFromEA<MCU_Operand_Size::BYTE>(Mode{}, mcu, st);
    const bool    msb    = input & 0x80;
    const uint8_t result = static_cast<uint8_t>((input << 1) | static_cast<uint8_t>(msb));
    StoreToEA<MCU_Operand_Size::BYTE>(Mode{}, mcu, st, result);
    MCU_SetStatus(mcu, result & 0x80, STATUS_N);
    MCU_SetStatus(mcu, result == 0, STATUS_Z);
    MCU_SetStatus(mcu, 0, STATUS_V);
    MCU_SetStatus(mcu, msb, STATUS_C);
}

template <typename Mode>
inline void I_ROTL_W_EAd(mcu_t& mcu, const I_CachedInstruction& st)
{
    InstructionScope<MCU_Operand_Size::WORD, Mode> scope(mcu, st, 1);

    const uint16_t input  = LoadFromEA<MCU_Operand_Size::WORD>(Mode{}, mcu, st);
    const bool     msb    = input & 0x8000;
    const uint16_t result = (uint16_t)(input << 1) | (uint16_t)msb;
    StoreToEA<MCU_Operand_Size::WORD>(Mode{}, mcu, st, result);
    MCU_SetStatus(mcu, result & 0x8000, STATUS_N);
    MCU_SetStatus(mcu, result == 0, STATUS_Z);
    MCU_SetStatus(mcu, 0, STATUS_V);
    MCU_SetStatus(mcu, msb, STATUS_C);
}

template <typename Mode>
inline void I_ROTR_B_EAd(mcu_t& mcu, const I_CachedInstruction& st)
{
    InstructionScope<MCU_Operand_Size::BYTE, Mode> scope(mcu, st, 1);

    const uint8_t input  = LoadFromEA<MCU_Operand_Size::BYTE>(Mode{}, mcu, st);
    const bool    lsb    = input & 1;
    const uint8_t result = static_cast<uint8_t>((input >> 1) | (lsb << 7));
    StoreToEA<MCU_Operand_Size::BYTE>(Mode{}, mcu, st, result);
    MCU_SetStatus(mcu, result & 0x80, STATUS_N);
    MCU_SetStatus(mcu, result == 0, STATUS_Z);
    MCU_SetStatus(mcu, 0, STATUS_V);
    MCU_SetStatus(mcu, lsb, STATUS_C);
}

template <typename Mode>
inline void I_ROTR_W_EAd(mcu_t& mcu, const I_CachedInstruction& st)
{
    InstructionScope<MCU_Operand_Size::WORD, Mode> scope(mcu, st, 1);

    const uint16_t input  = LoadFromEA<MCU_Operand_Size::WORD>(Mode{}, mcu, st);
    const bool     lsb    = input & 1;
    const uint16_t result = static_cast<uint16_t>((input << 1) | (lsb << 7));
    StoreToEA<MCU_Operand_Size::WORD>(Mode{}, mcu, st, result);
    MCU_SetStatus(mcu, result & 0x8000, STATUS_N);
    MCU_SetStatus(mcu, result == 0, STATUS_Z);
    MCU_SetStatus(mcu, 0, STATUS_V);
    MCU_SetStatus(mcu, lsb, STATUS_C);
}

template <typename Mode>
inline void I_XOR_B_EAs_Rd(mcu_t& mcu, const I_CachedInstruction& st)
{
    InstructionScope<MCU_Operand_Size::BYTE, Mode> scope(mcu, st, 1);

    const uint8_t EAs    = LoadFromEA<MCU_Operand_Size::BYTE>(Mode{}, mcu, st);
    const uint8_t Rd     = LoadFromOpReg<MCU_Operand_Size::BYTE>(mcu, st);
    const uint8_t result = EAs ^ Rd;

    StoreToOpReg<MCU_Operand_Size::BYTE>(mcu, st, result);
    MCU_SetStatus(mcu, result & 0x80, STATUS_N);
    MCU_SetStatus(mcu, result == 0, STATUS_Z);
    MCU_SetStatus(mcu, 0, STATUS_V);
}

template <typename Mode>
inline void I_XOR_W_EAs_Rd(mcu_t& mcu, const I_CachedInstruction& st)
{
    InstructionScope<MCU_Operand_Size::WORD, Mode> scope(mcu, st, 1);

    const uint16_t EAs    = LoadFromEA<MCU_Operand_Size::WORD>(Mode{}, mcu, st);
    const uint16_t Rd     = LoadFromOpReg<MCU_Operand_Size::WORD>(mcu, st);
    const uint16_t result = EAs ^ Rd;

    StoreToOpReg<MCU_Operand_Size::WORD>(mcu, st, result);
    MCU_SetStatus(mcu, result & 0x8000, STATUS_N);
    MCU_SetStatus(mcu, result == 0, STATUS_Z);
    MCU_SetStatus(mcu, 0, STATUS_V);
}

template <typename Mode>
inline void I_OR_B_EAs_Rd(mcu_t& mcu, const I_CachedInstruction& st)
{
    InstructionScope<MCU_Operand_Size::BYTE, Mode> scope(mcu, st, 1);

    const uint8_t EAs    = LoadFromEA<MCU_Operand_Size::BYTE>(Mode{}, mcu, st);
    const uint8_t Rd     = LoadFromOpReg<MCU_Operand_Size::BYTE>(mcu, st);
    const uint8_t result = EAs | Rd;

    StoreToOpReg<MCU_Operand_Size::BYTE>(mcu, st, result);
    MCU_SetStatus(mcu, result & 0x80, STATUS_N);
    MCU_SetStatus(mcu, result == 0, STATUS_Z);
    MCU_SetStatus(mcu, 0, STATUS_V);
}

template <typename Mode>
inline void I_OR_W_EAs_Rd(mcu_t& mcu, const I_CachedInstruction& st)
{
    InstructionScope<MCU_Operand_Size::WORD, Mode> scope(mcu, st, 1);

    const uint16_t EAs    = LoadFromEA<MCU_Operand_Size::WORD>(Mode{}, mcu, st);
    const uint16_t Rd     = LoadFromOpReg<MCU_Operand_Size::WORD>(mcu, st);
    const uint16_t result = EAs | Rd;

    StoreToOpReg<MCU_Operand_Size::WORD>(mcu, st, result);
    MCU_SetStatus(mcu, result & 0x8000, STATUS_N);
    MCU_SetStatus(mcu, result == 0, STATUS_Z);
    MCU_SetStatus(mcu, 0, STATUS_V);
}

template <typename Mode>
inline void I_AND_B_EAs_Rd(mcu_t& mcu, const I_CachedInstruction& st)
{
    InstructionScope<MCU_Operand_Size::BYTE, Mode> scope(mcu, st, 1);

    const uint8_t EAs    = LoadFromEA<MCU_Operand_Size::BYTE>(Mode{}, mcu, st);
    const uint8_t Rd     = LoadFromOpReg<MCU_Operand_Size::BYTE>(mcu, st);
    const uint8_t result = EAs & Rd;

    StoreToOpReg<MCU_Operand_Size::BYTE>(mcu, st, result);
    MCU_SetStatus(mcu, result & 0x80, STATUS_N);
    MCU_SetStatus(mcu, result == 0, STATUS_Z);
    MCU_SetStatus(mcu, 0, STATUS_V);
}

template <typename Mode>
inline void I_AND_W_EAs_Rd(mcu_t& mcu, const I_CachedInstruction& st)
{
    InstructionScope<MCU_Operand_Size::WORD, Mode> scope(mcu, st, 1);

    const uint16_t EAs    = LoadFromEA<MCU_Operand_Size::WORD>(Mode{}, mcu, st);
    const uint16_t Rd     = LoadFromOpReg<MCU_Operand_Size::WORD>(mcu, st);
    const uint16_t result = EAs & Rd;

    StoreToOpReg<MCU_Operand_Size::WORD>(mcu, st, result);
    MCU_SetStatus(mcu, result & 0x8000, STATUS_N);
    MCU_SetStatus(mcu, result == 0, STATUS_Z);
    MCU_SetStatus(mcu, 0, STATUS_V);
}

template <uint8_t Rn>
inline void I_CMP_I_W_imm16_Rd(mcu_t& mcu, const I_CachedInstruction& st)
{
    // behave as register-direct CMP:G.W #xx:16,EAd
    I_CMP_G_W_imm16_EAd<Mode_Rn>(mcu, st);
    // TODO/FIXME
    --mcu.pc;
}

template <uint8_t Rn>
inline void I_MOV_E_imm8_Rd(mcu_t& mcu, const I_CachedInstruction& st)
{
    // behave as register-direct MOV:G.B #xx:8,EAd
    I_MOV_G_B_imm8_EAd<Mode_Rn>(mcu, st);
    // TODO/FIXME
    --mcu.pc;
}

template <uint8_t Rn>
inline void I_MOV_L_B_aa8_Rd(mcu_t& mcu, const I_CachedInstruction& st)
{
    // behave as @aa:8 MOV:G.B EAs,Rd
    I_MOV_G_B_EAs_Rd<Mode_Aaa8>(mcu, st);
    // TODO/FIXME
    --mcu.pc;
}

template <uint8_t Rn>
inline void I_MOV_L_W_aa8_Rd(mcu_t& mcu, const I_CachedInstruction& st)
{
    // behave as @aa:8 MOV:G.W EAs,Rd
    I_MOV_G_W_EAs_Rd<Mode_Aaa8>(mcu, st);
    // TODO/FIXME
    --mcu.pc;
}

template <uint8_t Rn>
inline void I_MOV_I_W_imm16_Rd(mcu_t& mcu, const I_CachedInstruction& st)
{
    // behave as register-direct MOV:G.W #xx:16,EAd
    I_MOV_G_W_imm16_EAd<Mode_Rn>(mcu, st);
    // TODO/FIXME
    --mcu.pc;
}

template <uint8_t Rn>
inline void I_MOV_S_B_Rs_aa8(mcu_t& mcu, const I_CachedInstruction& st)
{
    // behave as @aa:8 MOV:G.B Rs,EAd
    I_MOV_G_B_Rs_EAd<Mode_Aaa8>(mcu, st);
    // TODO/FIXME
    --mcu.pc;
}

template <uint8_t Rn>
inline void I_MOV_S_W_Rs_aa8(mcu_t& mcu, const I_CachedInstruction& st)
{
    // behave as @aa:8 MOV:G.W Rs,EAd
    I_MOV_G_W_Rs_EAd<Mode_Aaa8>(mcu, st);
    // TODO/FIXME
    --mcu.pc;
}

template <uint8_t Rn>
inline void I_SCB_F(mcu_t& mcu, const I_CachedInstruction& st)
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
inline void I_SCB_NE(mcu_t& mcu, const I_CachedInstruction& st)
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
inline void I_SCB_EQ(mcu_t& mcu, const I_CachedInstruction& st)
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

inline void I_RTS(mcu_t& mcu, const I_CachedInstruction& st)
{
    (void)st;
    mcu.pc = MCU_PopStack(mcu);
}

inline void I_PRTS(mcu_t& mcu, const I_CachedInstruction& st)
{
    (void)st;
    mcu.cp = (uint8_t)MCU_PopStack(mcu);
    mcu.pc = MCU_PopStack(mcu);
}

inline void I_SLEEP(mcu_t& mcu, const I_CachedInstruction& st)
{
    (void)st;
    mcu.sleep = 1;
    ++mcu.pc;
}

// STM <register list>,@-SP
inline void I_STM(mcu_t& mcu, const I_CachedInstruction& st)
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
inline void I_STM_Fast(mcu_t& mcu, const I_CachedInstruction& st)
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
inline void I_LDM(mcu_t& mcu, const I_CachedInstruction& st)
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
inline void I_LDM_Fast(mcu_t& mcu, const I_CachedInstruction& st)
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

inline void I_JMP_ARn(mcu_t& mcu, const I_CachedInstruction& st)
{
    mcu.pc = mcu.r[st.op_reg];
}

inline void I_JMP_aa16(mcu_t& mcu, const I_CachedInstruction& st)
{
    mcu.pc = st.br_true;
}

inline void I_JSR_aa16(mcu_t& mcu, const I_CachedInstruction& st)
{
    MCU_PushStack(mcu, st.br_false);
    mcu.pc = st.br_true;
}

inline void I_JSR_ARn(mcu_t& mcu, const I_CachedInstruction& st)
{
    MCU_PushStack(mcu, st.br_false);
    mcu.pc = mcu.r[st.op_reg];
}

inline void I_PJMP_aa24(mcu_t& mcu, const I_CachedInstruction& st)
{
    mcu.cp = st.op_page;
    mcu.pc = st.op_data;
}

inline void I_PJSR_aa24(mcu_t& mcu, const I_CachedInstruction& st)
{
    MCU_PushStack(mcu, st.br_false);
    MCU_PushStack(mcu, mcu.cp);
    mcu.cp = st.op_page;
    mcu.pc = st.br_true;
}

inline void I_PJMP_ARn(mcu_t& mcu, const I_CachedInstruction& st)
{
    mcu.cp = (uint8_t)mcu.r[st.op_reg];
    mcu.pc = mcu.r[st.op_reg + 1];
}

inline void I_TRAPA_imm4(mcu_t& mcu, const I_CachedInstruction& st)
{
    MCU_Interrupt_TRAPA(mcu, (uint8_t)st.op_data);
    mcu.pc += 2;
}
