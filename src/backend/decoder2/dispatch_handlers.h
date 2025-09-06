// This file contains functions that decode an instruction and call into its
// instruction handler. These functions are instantiated by individual
// addressing mode dispatchers and mapped to one of 256 opcodes within that
// addressing mode.
//
// The addressing mode is passed as a template parameter and forwarded to the
// underlying instruction handler so it knows how to extract information from
// the EA field without having to determine the addressing mode at runtime.

#pragma once

#include "instruction_handlers.h"
#include "mcu.h"

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

    instr.op_data = MCU_ReadCodeAdvance(mcu);
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

    instr.op_data = MCU_ReadCodeAdvance16(mcu);
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

    instr.op_data = MCU_ReadCodeAdvance(mcu);
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

    instr.op_data = MCU_ReadCodeAdvance16(mcu);
    if constexpr (Sz == MCU_Operand_Size::BYTE)
    {
        mcu.icache.DoCache(mcu, instr_start, I_CMP_G_B_imm16_EAd<Mode>, instr);
    }
    else if constexpr (Sz == MCU_Operand_Size::WORD)
    {
        mcu.icache.DoCache(mcu, instr_start, I_CMP_G_W_imm16_EAd<Mode>, instr);
    }
}

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
void D_SWAP_B(mcu_t& mcu, uint32_t instr_start, uint8_t byte, I_CachedInstruction instr)
{
    (void)byte;

    mcu.icache.DoCache(mcu, instr_start, I_SWAP_B<Mode>, instr);
}

template <uint8_t OpReg, typename Mode>
void D_XCH_W_Rs_Rd(mcu_t& mcu, uint32_t instr_start, uint8_t byte, I_CachedInstruction instr)
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
void D_EXTS_B_Rd(mcu_t& mcu, uint32_t instr_start, uint8_t byte, I_CachedInstruction instr)
{
    (void)byte;

    static_assert(std::is_same_v<Mode, Mode_Rn>);
    mcu.icache.DoCache(mcu, instr_start, I_EXTS_B_Rd<Mode>, instr);
}

template <typename Mode>
void D_EXTU_B_Rd(mcu_t& mcu, uint32_t instr_start, uint8_t byte, I_CachedInstruction instr)
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
        mcu.icache.DoCache(mcu, instr_start, I_ORC_B_imm8_CR<CR>, instr);
    }
    else if constexpr (Sz == MCU_Operand_Size::WORD)
    {
        mcu.icache.DoCache(mcu, instr_start, I_ORC_W_imm16_CR<CR>, instr);
    }
}

template <MCU_Operand_Size Sz, uint8_t CR, typename Mode>
void D_ANDC_immXX_CR(mcu_t& mcu, uint32_t instr_start, uint8_t byte, I_CachedInstruction instr)
{
    (void)byte;

    instr.op_c = CR;
    if constexpr (Sz == MCU_Operand_Size::BYTE)
    {
        mcu.icache.DoCache(mcu, instr_start, I_ANDC_B_imm8_CR<CR>, instr);
    }
    else if constexpr (Sz == MCU_Operand_Size::WORD)
    {
        mcu.icache.DoCache(mcu, instr_start, I_ANDC_W_imm16_CR<CR>, instr);
    }
}
