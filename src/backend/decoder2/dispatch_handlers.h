// This file contains functions that decode an instruction and call into its
// instruction handler. These functions are instantiated by individual
// addressing mode dispatchers and mapped to one of 256 opcodes within that
// addressing mode.
//
// The addressing mode is passed as a template parameter and forwarded to the
// underlying instruction handler so it knows how to extract information from
// the EA field without having to determine the addressing mode at runtime.

#pragma once

#include "dispatch.h"
#include "instruction_handlers.h"
#include "mcu.h"

//=============================================================================
// General format instructions
//=============================================================================
template <MCU_Operand_Size Sz, uint8_t OpReg, typename Mode>
void D_MOV_G_EAs_Rd(mcu_t& mcu, uint32_t instr_start, uint8_t byte, I_CachedInstruction instr)
{
    (void)byte;

    instr.op_reg = OpReg;
    if constexpr (Sz == MCU_Operand_Size::BYTE)
    {
        mcu.icache.DoCache(mcu, instr_start, I_MOV_G_B_EAs_Rd<Mode>, instr);
    }
    else if constexpr (Sz == MCU_Operand_Size::WORD)
    {
        mcu.icache.DoCache(mcu, instr_start, I_MOV_G_W_EAs_Rd<Mode>, instr);
    }
}

template <MCU_Operand_Size Sz, uint8_t OpReg, typename Mode>
void D_MOV_G_Rs_EAd(mcu_t& mcu, uint32_t instr_start, uint8_t byte, I_CachedInstruction instr)
{
    (void)byte;

    instr.op_reg = OpReg;
    if constexpr (Sz == MCU_Operand_Size::BYTE)
    {
        mcu.icache.DoCache(mcu, instr_start, I_MOV_G_B_Rs_EAd<Mode>, instr);
    }
    else if constexpr (Sz == MCU_Operand_Size::WORD)
    {
        mcu.icache.DoCache(mcu, instr_start, I_MOV_G_W_Rs_EAd<Mode>, instr);
    }
}

template <MCU_Operand_Size Sz, typename Mode>
void D_MOV_G_imm8_EAd(mcu_t& mcu, uint32_t instr_start, uint8_t byte, I_CachedInstruction instr)
{
    (void)byte;

    instr.op_data = mcu.coder.ReadU8(mcu);
    if constexpr (Sz == MCU_Operand_Size::BYTE)
    {
        mcu.icache.DoCache(mcu, instr_start, I_MOV_G_B_imm8_EAd<Mode>, instr);
    }
    else if constexpr (Sz == MCU_Operand_Size::WORD)
    {
        mcu.icache.DoCache(mcu, instr_start, I_MOV_G_W_imm8_EAd<Mode>, instr);
    }
}

template <MCU_Operand_Size Sz, typename Mode>
void D_MOV_G_imm16_EAd(mcu_t& mcu, uint32_t instr_start, uint8_t byte, I_CachedInstruction instr)
{
    (void)byte;

    instr.op_data = mcu.coder.ReadU16(mcu);
    if constexpr (Sz == MCU_Operand_Size::BYTE)
    {
        mcu.icache.DoCache(mcu, instr_start, I_MOV_G_B_imm16_EAd<Mode>, instr);
    }
    else if constexpr (Sz == MCU_Operand_Size::WORD)
    {
        mcu.icache.DoCache(mcu, instr_start, I_MOV_G_W_imm16_EAd<Mode>, instr);
    }
}

template <MCU_Operand_Size Sz, typename Mode>
void D_CMP_G_imm8_EAd(mcu_t& mcu, uint32_t instr_start, uint8_t byte, I_CachedInstruction instr)
{
    (void)byte;

    instr.op_data = mcu.coder.ReadU8(mcu);
    if constexpr (Sz == MCU_Operand_Size::BYTE)
    {
        mcu.icache.DoCache(mcu, instr_start, I_CMP_G_B_imm8_EAd<Mode>, instr);
    }
    else if constexpr (Sz == MCU_Operand_Size::WORD)
    {
        mcu.icache.DoCache(mcu, instr_start, I_CMP_G_W_imm8_EAd<Mode>, instr);
    }
}

template <MCU_Operand_Size Sz, typename Mode>
void D_CMP_G_imm16_EAd(mcu_t& mcu, uint32_t instr_start, uint8_t byte, I_CachedInstruction instr)
{
    (void)byte;

    instr.op_data = mcu.coder.ReadU16(mcu);
    if constexpr (Sz == MCU_Operand_Size::BYTE)
    {
        mcu.icache.DoCache(mcu, instr_start, I_CMP_G_B_imm16_EAd<Mode>, instr);
    }
    else if constexpr (Sz == MCU_Operand_Size::WORD)
    {
        mcu.icache.DoCache(mcu, instr_start, I_CMP_G_W_imm16_EAd<Mode>, instr);
    }
}

// Manual lists this as a special format instruction but it allows arbitrary
// addressing modes so we treat it as a general instruction.
template <MCU_Operand_Size Sz, typename Mode, int8_t N>
void D_ADD_Q_n_EAd(mcu_t& mcu, uint32_t instr_start, uint8_t byte, I_CachedInstruction instr)
{
    (void)byte;

    if constexpr (Sz == MCU_Operand_Size::BYTE)
    {
        mcu.icache.DoCache(mcu, instr_start, I_ADD_Q_B_n<Mode, N>, instr);
    }
    else if constexpr (Sz == MCU_Operand_Size::WORD)
    {
        mcu.icache.DoCache(mcu, instr_start, I_ADD_Q_W_n<Mode, N>, instr);
    }
}

template <typename Mode>
void D_SWAP_Rd(mcu_t& mcu, uint32_t instr_start, uint8_t byte, I_CachedInstruction instr)
{
    (void)byte;

    mcu.icache.DoCache(mcu, instr_start, I_SWAP_B_Rd<Mode>, instr);
}

template <uint8_t OpReg, typename Mode>
void D_XCH_Rs_Rd(mcu_t& mcu, uint32_t instr_start, uint8_t byte, I_CachedInstruction instr)
{
    (void)byte;

    instr.op_reg = OpReg;
    mcu.icache.DoCache(mcu, instr_start, I_XCH_W_Rs_Rd<Mode>, instr);
}

template <MCU_Operand_Size Sz, uint8_t OpReg, typename Mode>
void D_ADD_G_EAs_Rd(mcu_t& mcu, uint32_t instr_start, uint8_t byte, I_CachedInstruction instr)
{
    (void)byte;

    instr.op_reg = OpReg;
    if constexpr (Sz == MCU_Operand_Size::BYTE)
    {
        mcu.icache.DoCache(mcu, instr_start, I_ADD_G_B_EAs_Rd<Mode>, instr);
    }
    else if constexpr (Sz == MCU_Operand_Size::WORD)
    {
        mcu.icache.DoCache(mcu, instr_start, I_ADD_G_W_EAs_Rd<Mode>, instr);
    }
}

template <MCU_Operand_Size Sz, uint8_t OpReg, typename Mode>
void D_ADDX_EAs_Rd(mcu_t& mcu, uint32_t instr_start, uint8_t byte, I_CachedInstruction instr)
{
    (void)byte;

    instr.op_reg = OpReg;
    if constexpr (Sz == MCU_Operand_Size::BYTE)
    {
        mcu.icache.DoCache(mcu, instr_start, I_ADDX_B_EAs_Rd<Mode>, instr);
    }
    else if constexpr (Sz == MCU_Operand_Size::WORD)
    {
        mcu.icache.DoCache(mcu, instr_start, I_ADDX_W_EAs_Rd<Mode>, instr);
    }
}

template <MCU_Operand_Size Sz, uint8_t OpReg, typename Mode>
void D_ADDS_EAs_Rd(mcu_t& mcu, uint32_t instr_start, uint8_t byte, I_CachedInstruction instr)
{
    (void)byte;

    instr.op_reg = OpReg;
    if constexpr (Sz == MCU_Operand_Size::BYTE)
    {
        mcu.icache.DoCache(mcu, instr_start, I_ADDS_B_EAs_Rd<Mode>, instr);
    }
    else if constexpr (Sz == MCU_Operand_Size::WORD)
    {
        mcu.icache.DoCache(mcu, instr_start, I_ADDS_W_EAs_Rd<Mode>, instr);
    }
}

template <MCU_Operand_Size Sz, uint8_t OpReg, typename Mode>
void D_CMP_G_EAs_Rd(mcu_t& mcu, uint32_t instr_start, uint8_t byte, I_CachedInstruction instr)
{
    (void)byte;

    instr.op_reg = OpReg;
    if constexpr (Sz == MCU_Operand_Size::BYTE)
    {
        mcu.icache.DoCache(mcu, instr_start, I_CMP_G_B_EAs_Rd<Mode>, instr);
    }
    else if constexpr (Sz == MCU_Operand_Size::WORD)
    {
        mcu.icache.DoCache(mcu, instr_start, I_CMP_G_W_EAs_Rd<Mode>, instr);
    }
}

template <MCU_Operand_Size Sz, typename Mode>
void D_SHLL_EAd(mcu_t& mcu, uint32_t instr_start, uint8_t byte, I_CachedInstruction instr)
{
    (void)byte;

    if constexpr (Sz == MCU_Operand_Size::BYTE)
    {
        mcu.icache.DoCache(mcu, instr_start, I_SHLL_B_EAd<Mode>, instr);
    }
    else if constexpr (Sz == MCU_Operand_Size::WORD)
    {
        mcu.icache.DoCache(mcu, instr_start, I_SHLL_W_EAd<Mode>, instr);
    }
}

template <MCU_Operand_Size Sz, typename Mode>
void D_SHLR_EAd(mcu_t& mcu, uint32_t instr_start, uint8_t byte, I_CachedInstruction instr)
{
    (void)byte;

    if constexpr (Sz == MCU_Operand_Size::BYTE)
    {
        mcu.icache.DoCache(mcu, instr_start, I_SHLR_B_EAd<Mode>, instr);
    }
    else if constexpr (Sz == MCU_Operand_Size::WORD)
    {
        mcu.icache.DoCache(mcu, instr_start, I_SHLR_W_EAd<Mode>, instr);
    }
}

template <MCU_Operand_Size Sz, typename Mode>
void D_SHAL_EAd(mcu_t& mcu, uint32_t instr_start, uint8_t byte, I_CachedInstruction instr)
{
    (void)byte;

    if constexpr (Sz == MCU_Operand_Size::BYTE)
    {
        mcu.icache.DoCache(mcu, instr_start, I_SHAL_B_EAd<Mode>, instr);
    }
    else if constexpr (Sz == MCU_Operand_Size::WORD)
    {
        mcu.icache.DoCache(mcu, instr_start, I_SHAL_W_EAd<Mode>, instr);
    }
}

template <MCU_Operand_Size Sz, typename Mode>
void D_SHAR_EAd(mcu_t& mcu, uint32_t instr_start, uint8_t byte, I_CachedInstruction instr)
{
    (void)byte;

    if constexpr (Sz == MCU_Operand_Size::BYTE)
    {
        mcu.icache.DoCache(mcu, instr_start, I_SHAR_B_EAd<Mode>, instr);
    }
    else if constexpr (Sz == MCU_Operand_Size::WORD)
    {
        mcu.icache.DoCache(mcu, instr_start, I_SHAR_W_EAd<Mode>, instr);
    }
}

template <MCU_Operand_Size Sz, typename Mode>
void D_NEG_EAd(mcu_t& mcu, uint32_t instr_start, uint8_t byte, I_CachedInstruction instr)
{
    (void)byte;

    if constexpr (Sz == MCU_Operand_Size::BYTE)
    {
        mcu.icache.DoCache(mcu, instr_start, I_NEG_B_EAd<Mode>, instr);
    }
    else if constexpr (Sz == MCU_Operand_Size::WORD)
    {
        mcu.icache.DoCache(mcu, instr_start, I_NEG_W_EAd<Mode>, instr);
    }
}

template <MCU_Operand_Size Sz, typename Mode>
void D_CLR_EAd(mcu_t& mcu, uint32_t instr_start, uint8_t byte, I_CachedInstruction instr)
{
    (void)byte;

    if constexpr (Sz == MCU_Operand_Size::BYTE)
    {
        mcu.icache.DoCache(mcu, instr_start, I_CLR_B_EAd<Mode>, instr);
    }
    else if constexpr (Sz == MCU_Operand_Size::WORD)
    {
        mcu.icache.DoCache(mcu, instr_start, I_CLR_W_EAd<Mode>, instr);
    }
}

template <MCU_Operand_Size Sz, typename Mode>
void D_TST_EAd(mcu_t& mcu, uint32_t instr_start, uint8_t byte, I_CachedInstruction instr)
{
    (void)byte;

    if constexpr (Sz == MCU_Operand_Size::BYTE)
    {
        mcu.icache.DoCache(mcu, instr_start, I_TST_B_EAd<Mode>, instr);
    }
    else if constexpr (Sz == MCU_Operand_Size::WORD)
    {
        mcu.icache.DoCache(mcu, instr_start, I_TST_W_EAd<Mode>, instr);
    }
}

template <MCU_Operand_Size Sz, uint8_t OpReg, typename Mode>
void D_SUB_EAs_Rd(mcu_t& mcu, uint32_t instr_start, uint8_t byte, I_CachedInstruction instr)
{
    (void)byte;

    instr.op_reg = OpReg;
    if constexpr (Sz == MCU_Operand_Size::BYTE)
    {
        mcu.icache.DoCache(mcu, instr_start, I_SUB_B_EAs_Rd<Mode>, instr);
    }
    else if constexpr (Sz == MCU_Operand_Size::WORD)
    {
        mcu.icache.DoCache(mcu, instr_start, I_SUB_W_EAs_Rd<Mode>, instr);
    }
}

template <MCU_Operand_Size Sz, uint8_t OpReg, typename Mode>
void D_SUBS_EAs_Rd(mcu_t& mcu, uint32_t instr_start, uint8_t byte, I_CachedInstruction instr)
{
    (void)byte;

    instr.op_reg = OpReg;
    if constexpr (Sz == MCU_Operand_Size::BYTE)
    {
        mcu.icache.DoCache(mcu, instr_start, I_SUBS_B_EAs_Rd<Mode>, instr);
    }
    else if constexpr (Sz == MCU_Operand_Size::WORD)
    {
        mcu.icache.DoCache(mcu, instr_start, I_SUBS_W_EAs_Rd<Mode>, instr);
    }
}

template <MCU_Operand_Size Sz, uint8_t OpReg, typename Mode>
void D_SUBX_EAs_Rd(mcu_t& mcu, uint32_t instr_start, uint8_t byte, I_CachedInstruction instr)
{
    (void)byte;

    instr.op_reg = OpReg;
    if constexpr (Sz == MCU_Operand_Size::BYTE)
    {
        mcu.icache.DoCache(mcu, instr_start, I_SUBX_B_EAs_Rd<Mode>, instr);
    }
    else if constexpr (Sz == MCU_Operand_Size::WORD)
    {
        mcu.icache.DoCache(mcu, instr_start, I_SUBX_W_EAs_Rd<Mode>, instr);
    }
}

template <typename Mode>
void D_EXTS_Rd(mcu_t& mcu, uint32_t instr_start, uint8_t byte, I_CachedInstruction instr)
{
    (void)byte;

    static_assert(std::is_same_v<Mode, Mode_Rn>);
    mcu.icache.DoCache(mcu, instr_start, I_EXTS_B_Rd<Mode>, instr);
}

template <typename Mode>
void D_EXTU_Rd(mcu_t& mcu, uint32_t instr_start, uint8_t byte, I_CachedInstruction instr)
{
    (void)byte;

    static_assert(std::is_same_v<Mode, Mode_Rn>);
    mcu.icache.DoCache(mcu, instr_start, I_EXTU_B_Rd<Mode>, instr);
}

template <MCU_Operand_Size Sz, typename Mode>
void D_NOT_EAd(mcu_t& mcu, uint32_t instr_start, uint8_t byte, I_CachedInstruction instr)
{
    (void)byte;

    if constexpr (Sz == MCU_Operand_Size::BYTE)
    {
        mcu.icache.DoCache(mcu, instr_start, I_NOT_B_EAd<Mode>, instr);
    }
    else if constexpr (Sz == MCU_Operand_Size::WORD)
    {
        mcu.icache.DoCache(mcu, instr_start, I_NOT_W_EAd<Mode>, instr);
    }
}

template <MCU_Operand_Size Sz, uint8_t OpReg, typename Mode>
void D_MULXU_EAs_Rd(mcu_t& mcu, uint32_t instr_start, uint8_t byte, I_CachedInstruction instr)
{
    (void)byte;

    instr.op_reg = OpReg;
    if constexpr (Sz == MCU_Operand_Size::BYTE)
    {
        mcu.icache.DoCache(mcu, instr_start, I_MULXU_B_EAs_Rd<Mode>, instr);
    }
    else if constexpr (Sz == MCU_Operand_Size::WORD)
    {
        mcu.icache.DoCache(mcu, instr_start, I_MULXU_X_EAs_Rd<Mode>, instr);
    }
}

template <MCU_Operand_Size Sz, uint8_t OpReg, typename Mode>
void D_DIVXU_EAs_Rd(mcu_t& mcu, uint32_t instr_start, uint8_t byte, I_CachedInstruction instr)
{
    (void)byte;

    instr.op_reg = OpReg;
    if constexpr (Sz == MCU_Operand_Size::BYTE)
    {
        mcu.icache.DoCache(mcu, instr_start, I_DIVXU_B_EAs_Rd<Mode>, instr);
    }
    else if constexpr (Sz == MCU_Operand_Size::WORD)
    {
        mcu.icache.DoCache(mcu, instr_start, I_DIVXU_W_EAs_Rd<Mode>, instr);
    }
}

template <MCU_Operand_Size Sz, uint8_t Imm4, typename Mode>
void D_BCLR_imm4_EAd(mcu_t& mcu, uint32_t instr_start, uint8_t byte, I_CachedInstruction instr)
{
    (void)byte;

    instr.op_data = Imm4 & 0b1111;
    if constexpr (Sz == MCU_Operand_Size::BYTE)
    {
        mcu.icache.DoCache(mcu, instr_start, I_BCLR_B_imm4_EAd<Mode>, instr);
    }
    else if constexpr (Sz == MCU_Operand_Size::WORD)
    {
        mcu.icache.DoCache(mcu, instr_start, I_BCLR_W_imm4_EAd<Mode>, instr);
    }
}

template <MCU_Operand_Size Sz, uint8_t Rs, typename Mode>
void D_BCLR_Rs_EAd(mcu_t& mcu, uint32_t instr_start, uint8_t byte, I_CachedInstruction instr)
{
    (void)byte;

    instr.op_reg = Rs;
    if constexpr (Sz == MCU_Operand_Size::BYTE)
    {
        mcu.icache.DoCache(mcu, instr_start, I_BCLR_B_Rs_EAd<Mode>, instr);
    }
    else if constexpr (Sz == MCU_Operand_Size::WORD)
    {
        mcu.icache.DoCache(mcu, instr_start, I_BCLR_W_Rs_EAd<Mode>, instr);
    }
}

template <MCU_Operand_Size Sz, uint8_t Imm4, typename Mode>
void D_BNOT_imm4_EAd(mcu_t& mcu, uint32_t instr_start, uint8_t byte, I_CachedInstruction instr)
{
    (void)byte;

    instr.op_data = Imm4 & 0b1111;
    if constexpr (Sz == MCU_Operand_Size::BYTE)
    {
        mcu.icache.DoCache(mcu, instr_start, I_BNOT_B_imm4_EAd<Mode>, instr);
    }
    else if constexpr (Sz == MCU_Operand_Size::WORD)
    {
        mcu.icache.DoCache(mcu, instr_start, I_BNOT_W_imm4_EAd<Mode>, instr);
    }
}

template <MCU_Operand_Size Sz, typename Mode>
void D_ROTL_EAd(mcu_t& mcu, uint32_t instr_start, uint8_t byte, I_CachedInstruction instr)
{
    (void)byte;

    if constexpr (Sz == MCU_Operand_Size::BYTE)
    {
        mcu.icache.DoCache(mcu, instr_start, I_ROTL_B_EAd<Mode>, instr);
    }
    else if constexpr (Sz == MCU_Operand_Size::WORD)
    {
        mcu.icache.DoCache(mcu, instr_start, I_ROTL_W_EAd<Mode>, instr);
    }
}

template <MCU_Operand_Size Sz, typename Mode>
void D_ROTR_EAd(mcu_t& mcu, uint32_t instr_start, uint8_t byte, I_CachedInstruction instr)
{
    (void)byte;

    if constexpr (Sz == MCU_Operand_Size::BYTE)
    {
        mcu.icache.DoCache(mcu, instr_start, I_ROTR_B_EAd<Mode>, instr);
    }
    else if constexpr (Sz == MCU_Operand_Size::WORD)
    {
        mcu.icache.DoCache(mcu, instr_start, I_ROTR_W_EAd<Mode>, instr);
    }
}

template <MCU_Operand_Size Sz, typename Mode>
void D_ROTXL_EAd(mcu_t& mcu, uint32_t instr_start, uint8_t byte, I_CachedInstruction instr)
{
    (void)byte;

    if constexpr (Sz == MCU_Operand_Size::BYTE)
    {
        mcu.icache.DoCache(mcu, instr_start, I_ROTXL_B_EAd<Mode>, instr);
    }
    else if constexpr (Sz == MCU_Operand_Size::WORD)
    {
        mcu.icache.DoCache(mcu, instr_start, I_ROTXL_W_EAd<Mode>, instr);
    }
}

template <MCU_Operand_Size Sz, uint8_t Imm4, typename Mode>
void D_BSET_imm4_EAd(mcu_t& mcu, uint32_t instr_start, uint8_t byte, I_CachedInstruction instr)
{
    (void)byte;

    instr.op_data = Imm4 & 0b1111;
    if constexpr (Sz == MCU_Operand_Size::BYTE)
    {
        mcu.icache.DoCache(mcu, instr_start, I_BSET_B_imm4_EAd<Mode>, instr);
    }
    else if constexpr (Sz == MCU_Operand_Size::WORD)
    {
        mcu.icache.DoCache(mcu, instr_start, I_BSET_W_imm4_EAd<Mode>, instr);
    }
}

template <MCU_Operand_Size Sz, uint8_t Rs, typename Mode>
void D_BSET_Rs_EAd(mcu_t& mcu, uint32_t instr_start, uint8_t byte, I_CachedInstruction instr)
{
    (void)byte;

    instr.op_reg = Rs;
    if constexpr (Sz == MCU_Operand_Size::BYTE)
    {
        mcu.icache.DoCache(mcu, instr_start, I_BSET_B_Rs_EAd<Mode>, instr);
    }
    else if constexpr (Sz == MCU_Operand_Size::WORD)
    {
        mcu.icache.DoCache(mcu, instr_start, I_BSET_W_Rs_EAd<Mode>, instr);
    }
}

template <MCU_Operand_Size Sz, uint8_t Imm4, typename Mode>
void D_BTST_imm4_EAd(mcu_t& mcu, uint32_t instr_start, uint8_t byte, I_CachedInstruction instr)
{
    (void)byte;

    instr.op_data = Imm4 & 0b1111;
    if constexpr (Sz == MCU_Operand_Size::BYTE)
    {
        mcu.icache.DoCache(mcu, instr_start, I_BTST_B_imm4_EAd<Mode>, instr);
    }
    else if constexpr (Sz == MCU_Operand_Size::WORD)
    {
        mcu.icache.DoCache(mcu, instr_start, I_BTST_W_imm4_EAd<Mode>, instr);
    }
}

template <MCU_Operand_Size Sz, uint8_t OpReg, typename Mode>
void D_BTST_Rs_EAd(mcu_t& mcu, uint32_t instr_start, uint8_t byte, I_CachedInstruction instr)
{
    (void)byte;

    instr.op_reg = OpReg;
    if constexpr (Sz == MCU_Operand_Size::BYTE)
    {
        mcu.icache.DoCache(mcu, instr_start, I_BTST_B_Rs_EAd<Mode>, instr);
    }
    else if constexpr (Sz == MCU_Operand_Size::WORD)
    {
        mcu.icache.DoCache(mcu, instr_start, I_BTST_W_Rs_EAd<Mode>, instr);
    }
}

template <MCU_Operand_Size Sz, uint8_t CR, typename Mode>
void D_STC_CR_EAd(mcu_t& mcu, uint32_t instr_start, uint8_t byte, I_CachedInstruction instr)
{
    (void)byte;

    instr.op_c = CR;
    if constexpr (Sz == MCU_Operand_Size::BYTE)
    {
        mcu.icache.DoCache(mcu, instr_start, I_STC_B_CR_EAd<Mode>, instr);
    }
    else if constexpr (Sz == MCU_Operand_Size::WORD)
    {
        mcu.icache.DoCache(mcu, instr_start, I_STC_W_CR_EAd<Mode>, instr);
    }
}

template <MCU_Operand_Size Sz, uint8_t CR, typename Mode>
void D_LDC_EAs_CR(mcu_t& mcu, uint32_t instr_start, uint8_t byte, I_CachedInstruction instr)
{
    (void)byte;

    instr.op_c = CR;
    if constexpr (Sz == MCU_Operand_Size::BYTE)
    {
        mcu.icache.DoCache(mcu, instr_start, I_LDC_B_EAs_CR<Mode>, instr);
    }
    else if constexpr (Sz == MCU_Operand_Size::WORD)
    {
        mcu.icache.DoCache(mcu, instr_start, I_LDC_W_EAs_CR<Mode>, instr);
    }
}

template <MCU_Operand_Size Sz, uint8_t OpReg, typename Mode>
void D_XOR_EAs_Rd(mcu_t& mcu, uint32_t instr_start, uint8_t byte, I_CachedInstruction instr)
{
    (void)byte;

    instr.op_reg = OpReg;
    if constexpr (Sz == MCU_Operand_Size::BYTE)
    {
        mcu.icache.DoCache(mcu, instr_start, I_XOR_B_EAs_Rd<Mode>, instr);
    }
    else if constexpr (Sz == MCU_Operand_Size::WORD)
    {
        mcu.icache.DoCache(mcu, instr_start, I_XOR_W_EAs_Rd<Mode>, instr);
    }
}

template <MCU_Operand_Size Sz, uint8_t OpReg, typename Mode>
void D_OR_EAs_Rd(mcu_t& mcu, uint32_t instr_start, uint8_t byte, I_CachedInstruction instr)
{
    (void)byte;

    instr.op_reg = OpReg;
    if constexpr (Sz == MCU_Operand_Size::BYTE)
    {
        mcu.icache.DoCache(mcu, instr_start, I_OR_B_EAs_Rd<Mode>, instr);
    }
    else if constexpr (Sz == MCU_Operand_Size::WORD)
    {
        mcu.icache.DoCache(mcu, instr_start, I_OR_W_EAs_Rd<Mode>, instr);
    }
}

template <MCU_Operand_Size Sz, uint8_t OpReg, typename Mode>
void D_AND_EAs_Rd(mcu_t& mcu, uint32_t instr_start, uint8_t byte, I_CachedInstruction instr)
{
    (void)byte;

    instr.op_reg = OpReg;
    if constexpr (Sz == MCU_Operand_Size::BYTE)
    {
        mcu.icache.DoCache(mcu, instr_start, I_AND_B_EAs_Rd<Mode>, instr);
    }
    else if constexpr (Sz == MCU_Operand_Size::WORD)
    {
        mcu.icache.DoCache(mcu, instr_start, I_AND_W_EAs_Rd<Mode>, instr);
    }
}

template <MCU_Operand_Size Sz, uint8_t CR, typename Mode>
void D_ORC_immXX_CR(mcu_t& mcu, uint32_t instr_start, uint8_t byte, I_CachedInstruction instr)
{
    (void)byte;

    instr.op_c = CR;
    if constexpr (Sz == MCU_Operand_Size::BYTE)
    {
        mcu.icache.DoCache(mcu, instr_start, I_ORC_B_imm8_CR<Mode, CR>, instr);
    }
    else if constexpr (Sz == MCU_Operand_Size::WORD)
    {
        mcu.icache.DoCache(mcu, instr_start, I_ORC_W_imm16_CR<Mode, CR>, instr);
    }
}

template <MCU_Operand_Size Sz, uint8_t CR, typename Mode>
void D_ANDC_immXX_CR(mcu_t& mcu, uint32_t instr_start, uint8_t byte, I_CachedInstruction instr)
{
    (void)byte;

    instr.op_c = CR;
    if constexpr (Sz == MCU_Operand_Size::BYTE)
    {
        mcu.icache.DoCache(mcu, instr_start, I_ANDC_B_imm8_CR<Mode, CR>, instr);
    }
    else if constexpr (Sz == MCU_Operand_Size::WORD)
    {
        mcu.icache.DoCache(mcu, instr_start, I_ANDC_W_imm16_CR<Mode, CR>, instr);
    }
}

//=============================================================================
// Special format instructions
//=============================================================================
template <MCU_Operand_Size Sz>
inline void D_Bcc(mcu_t& mcu, uint32_t instr_start, uint8_t opcode)
{
    const uint8_t cond = opcode & 0b1111;
    int16_t       disp;
    switch (Sz)
    {
    case MCU_Operand_Size::BYTE:
        disp = (int8_t)mcu.coder.ReadU8(mcu);
        break;
    case MCU_Operand_Size::WORD:
        disp = (int16_t)mcu.coder.ReadU16(mcu);
        break;
    }
    switch (cond)
    {
    case 0:
        mcu.icache.DoCacheBranch(mcu, instr_start, I_BRA, disp);
        break;
    case 1:
        mcu.icache.DoCacheBranch(mcu, instr_start, I_BRN, disp);
        break;
    case 2:
        mcu.icache.DoCacheBranch(mcu, instr_start, I_BHI, disp);
        break;
    case 3:
        mcu.icache.DoCacheBranch(mcu, instr_start, I_BLS, disp);
        break;
    case 4:
        mcu.icache.DoCacheBranch(mcu, instr_start, I_BCC, disp);
        break;
    case 5:
        mcu.icache.DoCacheBranch(mcu, instr_start, I_BCS, disp);
        break;
    case 6:
        mcu.icache.DoCacheBranch(mcu, instr_start, I_BNE, disp);
        break;
    case 7:
        mcu.icache.DoCacheBranch(mcu, instr_start, I_BEQ, disp);
        break;
    case 8:
        mcu.icache.DoCacheBranch(mcu, instr_start, I_BVC, disp);
        break;
    case 9:
        mcu.icache.DoCacheBranch(mcu, instr_start, I_BVS, disp);
        break;
    case 10:
        mcu.icache.DoCacheBranch(mcu, instr_start, I_BPL, disp);
        break;
    case 11:
        mcu.icache.DoCacheBranch(mcu, instr_start, I_BMI, disp);
        break;
    case 12:
        mcu.icache.DoCacheBranch(mcu, instr_start, I_BGE, disp);
        break;
    case 13:
        mcu.icache.DoCacheBranch(mcu, instr_start, I_BLT, disp);
        break;
    case 14:
        mcu.icache.DoCacheBranch(mcu, instr_start, I_BGT, disp);
        break;
    case 15:
        mcu.icache.DoCacheBranch(mcu, instr_start, I_BLE, disp);
        break;
    default:
        std::unreachable();
    }
}

inline void D_NOP(mcu_t& mcu, uint32_t instr_start, uint8_t byte)
{
    (void)byte;
    mcu.icache.DoCache(mcu, instr_start, I_NOP, {});
}

inline void D_RTE(mcu_t& mcu, uint32_t instr_start, uint8_t byte)
{
    (void)byte;
    mcu.icache.DoCacheBranch(mcu, instr_start, I_RTE, 0);
}

inline void D_SCB(mcu_t& mcu, uint32_t instr_start, uint8_t byte)
{
    const uint8_t regcode = mcu.coder.ReadU8(mcu);
    const int8_t  disp    = (int8_t)mcu.coder.ReadU8(mcu);

    if (byte == 0b00000001)
    {
        switch (regcode)
        {
        case 0b10111000:
            mcu.icache.DoCacheBranch(mcu, instr_start, I_SCB_F<0>, disp);
            break;
        case 0b10111001:
            mcu.icache.DoCacheBranch(mcu, instr_start, I_SCB_F<1>, disp);
            break;
        case 0b10111010:
            mcu.icache.DoCacheBranch(mcu, instr_start, I_SCB_F<2>, disp);
            break;
        case 0b10111011:
            mcu.icache.DoCacheBranch(mcu, instr_start, I_SCB_F<3>, disp);
            break;
        case 0b10111100:
            mcu.icache.DoCacheBranch(mcu, instr_start, I_SCB_F<4>, disp);
            break;
        case 0b10111101:
            mcu.icache.DoCacheBranch(mcu, instr_start, I_SCB_F<5>, disp);
            break;
        case 0b10111110:
            mcu.icache.DoCacheBranch(mcu, instr_start, I_SCB_F<6>, disp);
            break;
        case 0b10111111:
            mcu.icache.DoCacheBranch(mcu, instr_start, I_SCB_F<7>, disp);
            break;
        default:
            D_HardError(mcu, "SCB/F invalid regcode");
        }
    }
    else if (byte == 0b00000110)
    {
        switch (regcode)
        {
        case 0b10111000:
            mcu.icache.DoCacheBranch(mcu, instr_start, I_SCB_NE<0>, disp);
            break;
        case 0b10111001:
            mcu.icache.DoCacheBranch(mcu, instr_start, I_SCB_NE<1>, disp);
            break;
        case 0b10111010:
            mcu.icache.DoCacheBranch(mcu, instr_start, I_SCB_NE<2>, disp);
            break;
        case 0b10111011:
            mcu.icache.DoCacheBranch(mcu, instr_start, I_SCB_NE<3>, disp);
            break;
        case 0b10111100:
            mcu.icache.DoCacheBranch(mcu, instr_start, I_SCB_NE<4>, disp);
            break;
        case 0b10111101:
            mcu.icache.DoCacheBranch(mcu, instr_start, I_SCB_NE<5>, disp);
            break;
        case 0b10111110:
            mcu.icache.DoCacheBranch(mcu, instr_start, I_SCB_NE<6>, disp);
            break;
        case 0b10111111:
            mcu.icache.DoCacheBranch(mcu, instr_start, I_SCB_NE<7>, disp);
            break;
        default:
            D_HardError(mcu, "SCB/NE invalid regcode");
        }
    }
    else if (byte == 0b00000111)
    {
        switch (regcode)
        {
        case 0b10111000:
            mcu.icache.DoCacheBranch(mcu, instr_start, I_SCB_EQ<0>, disp);
            break;
        case 0b10111001:
            mcu.icache.DoCacheBranch(mcu, instr_start, I_SCB_EQ<1>, disp);
            break;
        case 0b10111010:
            mcu.icache.DoCacheBranch(mcu, instr_start, I_SCB_EQ<2>, disp);
            break;
        case 0b10111011:
            mcu.icache.DoCacheBranch(mcu, instr_start, I_SCB_EQ<3>, disp);
            break;
        case 0b10111100:
            mcu.icache.DoCacheBranch(mcu, instr_start, I_SCB_EQ<4>, disp);
            break;
        case 0b10111101:
            mcu.icache.DoCacheBranch(mcu, instr_start, I_SCB_EQ<5>, disp);
            break;
        case 0b10111110:
            mcu.icache.DoCacheBranch(mcu, instr_start, I_SCB_EQ<6>, disp);
            break;
        case 0b10111111:
            mcu.icache.DoCacheBranch(mcu, instr_start, I_SCB_EQ<7>, disp);
            break;
        default:
            D_HardError(mcu, "SCB/EQ invalid regcode");
        }
    }
    else
    {
        D_HardError(mcu, "not implemented");
    }
}

inline void D_RTS(mcu_t& mcu, uint32_t instr_start, uint8_t byte)
{
    (void)byte;
    mcu.icache.DoCache(mcu, instr_start, I_RTS, {});
}

inline void D_PRTS(mcu_t& mcu, uint32_t instr_start, uint8_t byte)
{
    (void)byte;
    mcu.icache.DoCache(mcu, instr_start, I_PRTS, {});
}

inline void D_JMP_ARn(mcu_t& mcu, uint32_t instr_start, uint8_t byte)
{
    I_CachedInstruction instr;
    instr.op_reg = byte & 0b111;
    mcu.icache.DoCache(mcu, instr_start, I_JMP_ARn, instr);
}

inline void D_PJMP_aa24(mcu_t& mcu, uint32_t instr_start, uint8_t byte)
{
    (void)byte;
    I_CachedInstruction instr;
    instr.op_page = mcu.coder.ReadU8(mcu);
    instr.op_data = mcu.coder.ReadU16(mcu);
    mcu.icache.DoCache(mcu, instr_start, I_PJMP_aa24, instr);
}

inline void D_PJSR_aa24(mcu_t& mcu, uint32_t instr_start, uint8_t byte)
{
    (void)byte;
    I_CachedInstruction instr;
    instr.op_page  = mcu.coder.ReadU8(mcu);
    instr.br_true  = mcu.coder.ReadU16(mcu);
    instr.br_false = mcu.coder.GetAddressInPage(mcu);
    mcu.icache.DoCache(mcu, instr_start, I_PJSR_aa24, instr);
}

inline void D_PJMP_ARn(mcu_t& mcu, uint32_t instr_start, uint8_t byte)
{
    I_CachedInstruction instr;
    instr.op_reg = byte & 0b111;
    mcu.icache.DoCache(mcu, instr_start, I_PJMP_ARn, instr);
}

inline void D_JMP_aa16(mcu_t& mcu, uint32_t instr_start, uint8_t byte)
{
    (void)byte;
    I_CachedInstruction instr;
    instr.br_true = mcu.coder.ReadU16(mcu);
    mcu.icache.DoCache(mcu, instr_start, I_JMP_aa16, instr);
}

inline void D_JSR_aa16(mcu_t& mcu, uint32_t instr_start, uint8_t byte)
{
    (void)byte;
    I_CachedInstruction instr;
    instr.br_true  = mcu.coder.ReadU16(mcu);
    instr.br_false = mcu.coder.GetAddressInPage(mcu);
    mcu.icache.DoCache(mcu, instr_start, I_JSR_aa16, instr);
}

inline void D_JSR_ARn(mcu_t& mcu, uint32_t instr_start, uint8_t byte)
{
    I_CachedInstruction instr;
    instr.op_reg   = byte & 0b111;
    instr.br_false = mcu.coder.GetAddressInPage(mcu);
    mcu.icache.DoCache(mcu, instr_start, I_JSR_ARn, instr);
}

inline void D_RTD_imm8(mcu_t& mcu, uint32_t instr_start, uint8_t byte)
{
    (void)byte;
    I_CachedInstruction instr;
    instr.op_data = mcu.coder.ReadU8(mcu);
    mcu.icache.DoCache(mcu, instr_start, I_RTD_immXX, instr);
}

inline void D_RTD_imm16(mcu_t& mcu, uint32_t instr_start, uint8_t byte)
{
    (void)byte;
    I_CachedInstruction instr;
    instr.op_data = mcu.coder.ReadU16(mcu);
    mcu.icache.DoCache(mcu, instr_start, I_RTD_immXX, instr);
}

inline void D_JMP(mcu_t& mcu, uint32_t instr_start, uint8_t byte)
{
    (void)instr_start;
    (void)byte;

    const uint8_t kind = mcu.coder.ReadU8(mcu);

    // TODO - not all kinds implemented
    switch (kind & 0b11111000)
    {
    // JMP @Rn
    case 0b11010000:
        D_JMP_ARn(mcu, instr_start, kind);
        break;

    // PJMP @Rn
    case 0b11000000:
        D_PJMP_ARn(mcu, instr_start, kind);
        break;

    // JSR @Rn
    case 0b11011000:
        D_JSR_ARn(mcu, instr_start, kind);
        break;

    default:
        if (kind == 0b00011001)
        {
            D_PRTS(mcu, instr_start, kind);
        }
        else
        {
            D_HardError(mcu);
        }
    }
}

inline void D_TRAPA(mcu_t& mcu, uint32_t instr_start, uint8_t byte)
{
    (void)byte;

    const uint8_t vec_byte = mcu.coder.ReadU8(mcu);

    I_CachedInstruction instr;
    instr.op_data = vec_byte & 0b1111;
    mcu.icache.DoCache(mcu, instr_start, I_TRAPA_imm4, instr);
}

inline void D_SLEEP(mcu_t& mcu, uint32_t instr_start, uint8_t byte)
{
    (void)byte;
    mcu.icache.DoCache(mcu, instr_start, I_SLEEP, {});
}

inline void D_STM(mcu_t& mcu, uint32_t instr_start, uint8_t byte)
{
    (void)byte;
    const uint8_t reglist = mcu.coder.ReadU8(mcu);
    if (reglist == 127)
    {
        // specialize the most commonly used form of this instruction
        mcu.icache.DoCache(mcu, instr_start, I_STM_Fast<127>, {});
    }
    else
    {
        I_CachedInstruction instr;
        instr.op_data = reglist;
        mcu.icache.DoCache(mcu, instr_start, I_STM, instr);
    }
}

inline void D_LDM(mcu_t& mcu, uint32_t instr_start, uint8_t byte)
{
    (void)byte;
    const uint8_t reglist = mcu.coder.ReadU8(mcu);
    if (reglist == 127)
    {
        // specialize the most commonly used form of this instruction
        mcu.icache.DoCache(mcu, instr_start, I_LDM_Fast<127>, {});
    }
    else
    {
        I_CachedInstruction instr;
        instr.op_data = reglist;
        mcu.icache.DoCache(mcu, instr_start, I_LDM, instr);
    }
}

inline void D_BSR_d8(mcu_t& mcu, uint32_t instr_start, uint8_t opcode)
{
    (void)opcode;
    const int8_t disp = (int8_t)mcu.coder.ReadU8(mcu);
    mcu.icache.DoCacheBranch(mcu, instr_start, I_BSR, disp);
}

inline void D_BSR_d16(mcu_t& mcu, uint32_t instr_start, uint8_t opcode)
{
    (void)opcode;
    const uint16_t disp = mcu.coder.ReadU16(mcu);
    mcu.icache.DoCacheBranch(mcu, instr_start, I_BSR, (int16_t)disp);
}

//=============================================================================
// Short format instructions
//=============================================================================
template <uint8_t Rn>
inline void D_Short_CMP_E_imm8_Rd(mcu_t& mcu, uint32_t instr_start, uint8_t byte)
{
    (void)byte;
    I_CachedInstruction instr;
    instr.op_data = mcu.coder.ReadU8(mcu);
    instr.ea_reg  = Rn;
    mcu.icache.DoCache(mcu, instr_start, I_CMP_E_imm8_Rd<Rn>, instr);
}

template <uint8_t Rn>
inline void D_Short_CMP_I_W_imm16_Rd(mcu_t& mcu, uint32_t instr_start, uint8_t byte)
{
    (void)byte;
    I_CachedInstruction instr;
    instr.op_data = mcu.coder.ReadU16(mcu);
    instr.ea_reg  = Rn;
    mcu.icache.DoCache(mcu, instr_start, I_CMP_I_W_imm16_Rd<Rn>, instr);
}

template <uint8_t Rn>
inline void D_Short_MOV_E_imm8_Rd(mcu_t& mcu, uint32_t instr_start, uint8_t byte)
{
    (void)byte;
    I_CachedInstruction instr;
    instr.op_data = mcu.coder.ReadU8(mcu);
    instr.ea_reg  = Rn;
    mcu.icache.DoCache(mcu, instr_start, I_MOV_E_imm8_Rd<Rn>, instr);
}

template <uint8_t Rn>
inline void D_Short_MOV_I_W_imm16_Rd(mcu_t& mcu, uint32_t instr_start, uint8_t byte)
{
    (void)byte;
    I_CachedInstruction instr;
    instr.op_data = mcu.coder.ReadU16(mcu);
    instr.ea_reg  = Rn;
    mcu.icache.DoCache(mcu, instr_start, I_MOV_I_W_imm16_Rd<Rn>, instr);
}

template <MCU_Operand_Size Sz, uint8_t Rn>
inline void D_Short_MOV_L_aa8_Rd(mcu_t& mcu, uint32_t instr_start, uint8_t byte)
{
    (void)byte;
    I_CachedInstruction instr;
    instr.ea_data = mcu.coder.ReadU8(mcu);
    instr.op_reg  = Rn;

    if constexpr (Sz == MCU_Operand_Size::BYTE)
    {
        mcu.icache.DoCache(mcu, instr_start, I_MOV_L_B_aa8_Rd<Rn>, instr);
    }
    else if constexpr (Sz == MCU_Operand_Size::WORD)
    {
        mcu.icache.DoCache(mcu, instr_start, I_MOV_L_W_aa8_Rd<Rn>, instr);
    }
}

template <MCU_Operand_Size Sz, uint8_t Rn>
inline void D_Short_I_MOV_S_Rs_aa8(mcu_t& mcu, uint32_t instr_start, uint8_t byte)
{
    (void)byte;
    I_CachedInstruction instr;
    instr.op_reg  = Rn;
    instr.ea_data = mcu.coder.ReadU8(mcu);
    if constexpr (Sz == MCU_Operand_Size::BYTE)
    {
        mcu.icache.DoCache(mcu, instr_start, I_MOV_S_B_Rs_aa8<Rn>, instr);
    }
    else if constexpr (Sz == MCU_Operand_Size::WORD)
    {
        mcu.icache.DoCache(mcu, instr_start, I_MOV_S_W_Rs_aa8<Rn>, instr);
    }
}
