// This file contains functions that decode an instruction and call into its
// instruction handler. These functions are instantiated by individual
// addressing mode dispatchers and mapped to one of 256 opcodes within that
// addressing mode.
//
// The addressing mode is passed as a template parameter and forwarded to the
// underlying instruction handler so it knows how to extract information from
// the EA field without having to determine the addressing mode at runtime.

#pragma once

#include "decoder2/cache.h"
#include "decoder2/code_reader.h"
#include "dispatch.h"
#include "instruction_handlers.h"
#include "types.h"

namespace decoder2
{

//=============================================================================
// Helpers
//=============================================================================
inline void MakeBranch(CodeReader& reader, CachedInstruction& instr, CachedInstructionHandler handler, int16_t disp)
{
    const uint16_t next_ip = reader.GetAddressInPage();
    instr.params.br_true   = (uint16_t)(next_ip + disp);
    instr.params.br_false  = next_ip;
    instr.handler          = handler;
}

//=============================================================================
// Pseudo instructions
//=============================================================================
inline void D_InvalidInstruction(CodeReader& reader, uint8_t byte, CachedInstruction& instr)
{
    (void)reader;
    (void)byte;
    (void)instr;
    // TODO: do we need to know about the mcu? this should signal failure in the decoder subsystem which might know
    // about the mcu higher up
    FatalError(reader.GetMCU(), "Invalid instruction");
}

//=============================================================================
// General format instructions
//=============================================================================
template <Size Sz, uint8_t OpReg, typename Mode>
void D_MOV_G_EAs_Rd(CodeReader& reader, uint8_t byte, CachedInstruction& instr)
{
    (void)reader;
    (void)byte;

    instr.params.op_reg = OpReg;
    instr.handler       = I_MOV_G_EAs_Rd<Sz, Mode>;
}

template <Size Sz, uint8_t OpReg, typename Mode>
void D_MOV_G_Rs_EAd(CodeReader& reader, uint8_t byte, CachedInstruction& instr)
{
    (void)reader;
    (void)byte;

    instr.params.op_reg = OpReg;
    instr.handler       = I_MOV_G_Rs_EAd<Sz, Mode>;
}

template <Size Sz, typename Mode>
void D_MOV_G_imm8_EAd(CodeReader& reader, uint8_t byte, CachedInstruction& instr)
{
    (void)byte;

    instr.params.op_data = reader.ReadU8();
    instr.handler        = I_MOV_G_imm8_EAd<Sz, Mode>;
}

template <Size Sz, typename Mode>
void D_MOV_G_imm16_EAd(CodeReader& reader, uint8_t byte, CachedInstruction& instr)
{
    (void)byte;

    instr.params.op_data = reader.ReadU16();
    instr.handler        = I_MOV_G_imm16_EAd<Sz, Mode>;
}

template <Size Sz, typename Mode>
void D_CMP_G_imm8_EAd(CodeReader& reader, uint8_t byte, CachedInstruction& instr)
{
    (void)byte;

    instr.params.op_data = reader.ReadU8();
    instr.handler        = I_CMP_G_imm8_EAd<Sz, Mode>;
}

template <Size Sz, typename Mode>
void D_CMP_G_imm16_EAd(CodeReader& reader, uint8_t byte, CachedInstruction& instr)
{
    (void)byte;

    instr.params.op_data = reader.ReadU16();
    instr.handler        = I_CMP_G_imm16_EAd<Sz, Mode>;
}

// Manual lists this as a special format instruction but it allows arbitrary
// addressing modes so we treat it as a general instruction.
template <Size Sz, typename Mode, int8_t N>
void D_ADD_Q_n_EAd(CodeReader& reader, uint8_t byte, CachedInstruction& instr)
{
    (void)reader;
    (void)byte;

    instr.handler = I_ADD_Q_n<Sz, Mode, N>;
}

template <typename Mode>
void D_SWAP_Rd(CodeReader& reader, uint8_t byte, CachedInstruction& instr)
{
    (void)reader;
    (void)byte;

    instr.handler = I_SWAP_B_Rd<Mode>;
}

template <uint8_t OpReg, typename Mode>
void D_XCH_Rs_Rd(CodeReader& reader, uint8_t byte, CachedInstruction& instr)
{
    (void)reader;
    (void)byte;

    instr.params.op_reg = OpReg;
    instr.handler       = I_XCH_W_Rs_Rd<Mode>;
}

template <Size Sz, uint8_t OpReg, typename Mode>
void D_ADD_G_EAs_Rd(CodeReader& reader, uint8_t byte, CachedInstruction& instr)
{
    (void)reader;
    (void)byte;

    instr.params.op_reg = OpReg;
    instr.handler       = I_ADD_G_EAs_Rd<Sz, Mode>;
}

template <Size Sz, uint8_t OpReg, typename Mode>
void D_ADDX_EAs_Rd(CodeReader& reader, uint8_t byte, CachedInstruction& instr)
{
    (void)reader;
    (void)byte;

    instr.params.op_reg = OpReg;
    instr.handler       = I_ADDX_EAs_Rd<Sz, Mode>;
}

template <Size Sz, uint8_t OpReg, typename Mode>
void D_ADDS_EAs_Rd(CodeReader& reader, uint8_t byte, CachedInstruction& instr)
{
    (void)reader;
    (void)byte;

    instr.params.op_reg = OpReg;
    instr.handler       = I_ADDS_EAs_Rd<Sz, Mode>;
}

template <Size Sz, uint8_t OpReg, typename Mode>
void D_CMP_G_EAs_Rd(CodeReader& reader, uint8_t byte, CachedInstruction& instr)
{
    (void)reader;
    (void)byte;

    instr.params.op_reg = OpReg;
    instr.handler       = I_CMP_G_EAs_Rd<Sz, Mode>;
}

template <Size Sz, typename Mode>
void D_SHLL_EAd(CodeReader& reader, uint8_t byte, CachedInstruction& instr)
{
    (void)reader;
    (void)byte;

    instr.handler = I_SHLL_EAd<Sz, Mode>;
}

template <Size Sz, typename Mode>
void D_SHLR_EAd(CodeReader& reader, uint8_t byte, CachedInstruction& instr)
{
    (void)reader;
    (void)byte;

    instr.handler = I_SHLR_EAd<Sz, Mode>;
}

template <Size Sz, typename Mode>
void D_SHAL_EAd(CodeReader& reader, uint8_t byte, CachedInstruction& instr)
{
    (void)reader;
    (void)byte;

    instr.handler = I_SHAL_EAd<Sz, Mode>;
}

template <Size Sz, typename Mode>
void D_SHAR_EAd(CodeReader& reader, uint8_t byte, CachedInstruction& instr)
{
    (void)reader;
    (void)byte;

    instr.handler = I_SHAR_EAd<Sz, Mode>;
}

template <Size Sz, typename Mode>
void D_NEG_EAd(CodeReader& reader, uint8_t byte, CachedInstruction& instr)
{
    (void)reader;
    (void)byte;

    instr.handler = I_NEG_EAd<Sz, Mode>;
}

template <Size Sz, typename Mode>
void D_CLR_EAd(CodeReader& reader, uint8_t byte, CachedInstruction& instr)
{
    (void)reader;
    (void)byte;

    instr.handler = I_CLR_EAd<Sz, Mode>;
}

template <Size Sz, typename Mode>
void D_TST_EAd(CodeReader& reader, uint8_t byte, CachedInstruction& instr)
{
    (void)reader;
    (void)byte;

    instr.handler = I_TST_EAd<Sz, Mode>;
}

template <Size Sz, uint8_t OpReg, typename Mode>
void D_SUB_EAs_Rd(CodeReader& reader, uint8_t byte, CachedInstruction& instr)
{
    (void)reader;
    (void)byte;

    instr.params.op_reg = OpReg;
    instr.handler       = I_SUB_EAs_Rd<Sz, Mode>;
}

template <Size Sz, uint8_t OpReg, typename Mode>
void D_SUBS_EAs_Rd(CodeReader& reader, uint8_t byte, CachedInstruction& instr)
{
    (void)reader;
    (void)byte;

    instr.params.op_reg = OpReg;
    instr.handler       = I_SUBS_EAs_Rd<Sz, Mode>;
}

template <Size Sz, uint8_t OpReg, typename Mode>
void D_SUBX_EAs_Rd(CodeReader& reader, uint8_t byte, CachedInstruction& instr)
{
    (void)reader;
    (void)byte;

    instr.params.op_reg = OpReg;
    instr.handler       = I_SUBX_EAs_Rd<Sz, Mode>;
}

template <typename Mode>
void D_EXTS_Rd(CodeReader& reader, uint8_t byte, CachedInstruction& instr)
{
    (void)reader;
    (void)byte;

    static_assert(std::is_same_v<Mode, Mode_Rn>);
    instr.handler = I_EXTS_B_Rd<Mode>;
}

template <typename Mode>
void D_EXTU_Rd(CodeReader& reader, uint8_t byte, CachedInstruction& instr)
{
    (void)reader;
    (void)byte;

    static_assert(std::is_same_v<Mode, Mode_Rn>);
    instr.handler = I_EXTU_B_Rd<Mode>;
}

template <Size Sz, typename Mode>
void D_NOT_EAd(CodeReader& reader, uint8_t byte, CachedInstruction& instr)
{
    (void)reader;
    (void)byte;

    instr.handler = I_NOT_EAd<Sz, Mode>;
}

template <Size Sz, uint8_t OpReg, typename Mode>
void D_MULXU_EAs_Rd(CodeReader& reader, uint8_t byte, CachedInstruction& instr)
{
    (void)reader;
    (void)byte;

    instr.params.op_reg = OpReg;
    instr.handler       = I_MULXU_EAs_Rd<Sz, Mode>;
}

template <Size Sz, uint8_t OpReg, typename Mode>
void D_DIVXU_EAs_Rd(CodeReader& reader, uint8_t byte, CachedInstruction& instr)
{
    (void)reader;
    (void)byte;

    instr.params.op_reg = OpReg;
    instr.handler       = I_DIVXU_EAs_Rd<Sz, Mode>;
}

template <Size Sz, uint8_t Imm4, typename Mode>
void D_BCLR_imm4_EAd(CodeReader& reader, uint8_t byte, CachedInstruction& instr)
{
    (void)reader;
    (void)byte;

    instr.params.op_data = Imm4 & 0b1111;
    instr.handler        = I_BCLR_imm4_EAd<Sz, Mode>;
}

template <Size Sz, uint8_t Rs, typename Mode>
void D_BCLR_Rs_EAd(CodeReader& reader, uint8_t byte, CachedInstruction& instr)
{
    (void)reader;
    (void)byte;

    instr.params.op_reg = Rs;
    instr.handler       = I_BCLR_Rs_EAd<Sz, Mode>;
}

template <Size Sz, uint8_t Imm4, typename Mode>
void D_BNOT_imm4_EAd(CodeReader& reader, uint8_t byte, CachedInstruction& instr)
{
    (void)reader;
    (void)byte;

    instr.params.op_data = Imm4 & 0b1111;
    instr.handler        = I_BNOT_imm4_EAd<Sz, Mode>;
}

template <Size Sz, typename Mode>
void D_ROTL_EAd(CodeReader& reader, uint8_t byte, CachedInstruction& instr)
{
    (void)reader;
    (void)byte;

    instr.handler = I_ROTL_EAd<Sz, Mode>;
}

template <Size Sz, typename Mode>
void D_ROTR_EAd(CodeReader& reader, uint8_t byte, CachedInstruction& instr)
{
    (void)reader;
    (void)byte;

    instr.handler = I_ROTR_EAd<Sz, Mode>;
}

template <Size Sz, typename Mode>
void D_ROTXL_EAd(CodeReader& reader, uint8_t byte, CachedInstruction& instr)
{
    (void)reader;
    (void)byte;

    instr.handler = I_ROTXL_EAd<Sz, Mode>;
}

template <Size Sz, typename Mode>
void D_ROTXR_EAd(CodeReader& reader, uint8_t byte, CachedInstruction& instr)
{
    (void)reader;
    (void)byte;

    instr.handler = I_ROTXR_EAd<Sz, Mode>;
}

template <Size Sz, uint8_t Imm4, typename Mode>
void D_BSET_imm4_EAd(CodeReader& reader, uint8_t byte, CachedInstruction& instr)
{
    (void)reader;
    (void)byte;

    instr.params.op_data = Imm4 & 0b1111;
    instr.handler        = I_BSET_imm4_EAd<Sz, Mode>;
}

template <Size Sz, uint8_t Rs, typename Mode>
void D_BSET_Rs_EAd(CodeReader& reader, uint8_t byte, CachedInstruction& instr)
{
    (void)reader;
    (void)byte;

    instr.params.op_reg = Rs;
    instr.handler       = I_BSET_Rs_EAd<Sz, Mode>;
}

template <Size Sz, uint8_t Imm4, typename Mode>
void D_BTST_imm4_EAd(CodeReader& reader, uint8_t byte, CachedInstruction& instr)
{
    (void)reader;
    (void)byte;

    instr.params.op_data = Imm4 & 0b1111;
    instr.handler        = I_BTST_imm4_EAd<Sz, Mode>;
}

template <Size Sz, uint8_t OpReg, typename Mode>
void D_BTST_Rs_EAd(CodeReader& reader, uint8_t byte, CachedInstruction& instr)
{
    (void)reader;
    (void)byte;

    instr.params.op_reg = OpReg;
    instr.handler       = I_BTST_Rs_EAd<Sz, Mode>;
}

template <Size Sz, uint8_t CR, typename Mode>
void D_STC_CR_EAd(CodeReader& reader, uint8_t byte, CachedInstruction& instr)
{
    (void)reader;
    (void)byte;

    instr.params.op_c = CR;
    instr.handler     = I_STC_CR_EAd<Sz, Mode>;
}

template <Size Sz, uint8_t CR, typename Mode>
void D_LDC_EAs_CR(CodeReader& reader, uint8_t byte, CachedInstruction& instr)
{
    (void)reader;
    (void)byte;

    instr.params.op_c = CR;
    instr.handler     = I_LDC_EAs_CR<Sz, Mode>;
}

template <Size Sz, uint8_t OpReg, typename Mode>
void D_XOR_EAs_Rd(CodeReader& reader, uint8_t byte, CachedInstruction& instr)
{
    (void)reader;
    (void)byte;

    instr.params.op_reg = OpReg;
    instr.handler       = I_XOR_EAs_Rd<Sz, Mode>;
}

template <Size Sz, uint8_t OpReg, typename Mode>
void D_OR_EAs_Rd(CodeReader& reader, uint8_t byte, CachedInstruction& instr)
{
    (void)reader;
    (void)byte;

    instr.params.op_reg = OpReg;
    instr.handler       = I_OR_EAs_Rd<Sz, Mode>;
}

template <Size Sz, uint8_t OpReg, typename Mode>
void D_AND_EAs_Rd(CodeReader& reader, uint8_t byte, CachedInstruction& instr)
{
    (void)reader;
    (void)byte;

    instr.params.op_reg = OpReg;
    instr.handler       = I_AND_EAs_Rd<Sz, Mode>;
}

template <Size Sz, uint8_t CR, typename Mode>
void D_ORC_immXX_CR(CodeReader& reader, uint8_t byte, CachedInstruction& instr)
{
    (void)reader;
    (void)byte;

    instr.params.op_c = CR;
    instr.handler     = I_ORC_immXX_CR<Sz, Mode, CR>;
}

template <Size Sz, uint8_t CR, typename Mode>
void D_ANDC_immXX_CR(CodeReader& reader, uint8_t byte, CachedInstruction& instr)
{
    (void)reader;
    (void)byte;

    instr.params.op_c = CR;
    instr.handler     = I_ANDC_immXX_CR<Sz, Mode, CR>;
}

//=============================================================================
// Special format instructions
//=============================================================================
template <Size Sz>
inline void D_Bcc(CodeReader& reader, uint8_t byte, CachedInstruction& instr)
{
    const uint8_t cond = byte & 0b1111;
    int16_t       disp;
    switch (Sz)
    {
    case Size::Byte:
        disp = (int8_t)reader.ReadU8();
        break;
    case Size::Word:
        disp = (int16_t)reader.ReadU16();
        break;
    }
    switch (cond)
    {
    case 0:
        MakeBranch(reader, instr, I_BRA, disp);
        break;
    case 1:
        MakeBranch(reader, instr, I_BRN, disp);
        break;
    case 2:
        MakeBranch(reader, instr, I_BHI, disp);
        break;
    case 3:
        MakeBranch(reader, instr, I_BLS, disp);
        break;
    case 4:
        MakeBranch(reader, instr, I_BCC, disp);
        break;
    case 5:
        MakeBranch(reader, instr, I_BCS, disp);
        break;
    case 6:
        MakeBranch(reader, instr, I_BNE, disp);
        break;
    case 7:
        MakeBranch(reader, instr, I_BEQ, disp);
        break;
    case 8:
        MakeBranch(reader, instr, I_BVC, disp);
        break;
    case 9:
        MakeBranch(reader, instr, I_BVS, disp);
        break;
    case 10:
        MakeBranch(reader, instr, I_BPL, disp);
        break;
    case 11:
        MakeBranch(reader, instr, I_BMI, disp);
        break;
    case 12:
        MakeBranch(reader, instr, I_BGE, disp);
        break;
    case 13:
        MakeBranch(reader, instr, I_BLT, disp);
        break;
    case 14:
        MakeBranch(reader, instr, I_BGT, disp);
        break;
    case 15:
        MakeBranch(reader, instr, I_BLE, disp);
        break;
    default:
        std::unreachable();
    }
}

inline void D_NOP(CodeReader& reader, uint8_t byte, CachedInstruction& instr)
{
    (void)reader;
    (void)byte;

    instr.handler = I_NOP;
}

inline void D_RTE(CodeReader& reader, uint8_t byte, CachedInstruction& instr)
{
    (void)reader;
    (void)byte;

    instr.handler = I_RTE;
}

inline void D_SCB(CodeReader& reader, uint8_t byte, CachedInstruction& instr)
{
    const uint8_t regcode = reader.ReadU8();
    const int8_t  disp    = (int8_t)reader.ReadU8();

    if (byte == 0b00000001)
    {
        switch (regcode)
        {
        case 0b10111000:
            MakeBranch(reader, instr, I_SCB_F<0>, disp);
            break;
        case 0b10111001:
            MakeBranch(reader, instr, I_SCB_F<1>, disp);
            break;
        case 0b10111010:
            MakeBranch(reader, instr, I_SCB_F<2>, disp);
            break;
        case 0b10111011:
            MakeBranch(reader, instr, I_SCB_F<3>, disp);
            break;
        case 0b10111100:
            MakeBranch(reader, instr, I_SCB_F<4>, disp);
            break;
        case 0b10111101:
            MakeBranch(reader, instr, I_SCB_F<5>, disp);
            break;
        case 0b10111110:
            MakeBranch(reader, instr, I_SCB_F<6>, disp);
            break;
        case 0b10111111:
            MakeBranch(reader, instr, I_SCB_F<7>, disp);
            break;
        default:
            FatalError(reader.GetMCU(), "SCB/F invalid regcode");
        }
    }
    else if (byte == 0b00000110)
    {
        switch (regcode)
        {
        case 0b10111000:
            MakeBranch(reader, instr, I_SCB_NE<0>, disp);
            break;
        case 0b10111001:
            MakeBranch(reader, instr, I_SCB_NE<1>, disp);
            break;
        case 0b10111010:
            MakeBranch(reader, instr, I_SCB_NE<2>, disp);
            break;
        case 0b10111011:
            MakeBranch(reader, instr, I_SCB_NE<3>, disp);
            break;
        case 0b10111100:
            MakeBranch(reader, instr, I_SCB_NE<4>, disp);
            break;
        case 0b10111101:
            MakeBranch(reader, instr, I_SCB_NE<5>, disp);
            break;
        case 0b10111110:
            MakeBranch(reader, instr, I_SCB_NE<6>, disp);
            break;
        case 0b10111111:
            MakeBranch(reader, instr, I_SCB_NE<7>, disp);
            break;
        default:
            FatalError(reader.GetMCU(), "SCB/NE invalid regcode");
        }
    }
    else if (byte == 0b00000111)
    {
        switch (regcode)
        {
        case 0b10111000:
            MakeBranch(reader, instr, I_SCB_EQ<0>, disp);
            break;
        case 0b10111001:
            MakeBranch(reader, instr, I_SCB_EQ<1>, disp);
            break;
        case 0b10111010:
            MakeBranch(reader, instr, I_SCB_EQ<2>, disp);
            break;
        case 0b10111011:
            MakeBranch(reader, instr, I_SCB_EQ<3>, disp);
            break;
        case 0b10111100:
            MakeBranch(reader, instr, I_SCB_EQ<4>, disp);
            break;
        case 0b10111101:
            MakeBranch(reader, instr, I_SCB_EQ<5>, disp);
            break;
        case 0b10111110:
            MakeBranch(reader, instr, I_SCB_EQ<6>, disp);
            break;
        case 0b10111111:
            MakeBranch(reader, instr, I_SCB_EQ<7>, disp);
            break;
        default:
            FatalError(reader.GetMCU(), "SCB/EQ invalid regcode");
        }
    }
    else
    {
        FatalError(reader.GetMCU(), "not implemented");
    }
}

inline void D_RTS(CodeReader& reader, uint8_t byte, CachedInstruction& instr)
{
    (void)reader;
    (void)byte;

    instr.handler = I_RTS;
}

inline void D_PRTS(CodeReader& reader, uint8_t byte, CachedInstruction& instr)
{
    (void)reader;
    (void)byte;

    instr.handler = I_PRTS;
}

inline void D_JMP_ARn(CodeReader& reader, uint8_t byte, CachedInstruction& instr)
{
    (void)reader;

    instr.params.op_reg = byte & 0b111;
    instr.handler       = I_JMP_ARn;
}

inline void D_PJMP_aa24(CodeReader& reader, uint8_t byte, CachedInstruction& instr)
{
    (void)byte;

    instr.params.op_page = reader.ReadU8();
    instr.params.op_data = reader.ReadU16();
    instr.handler        = I_PJMP_aa24;
}

inline void D_PJSR_aa24(CodeReader& reader, uint8_t byte, CachedInstruction& instr)
{
    (void)byte;

    instr.params.op_page  = reader.ReadU8();
    instr.params.br_true  = reader.ReadU16();
    instr.params.br_false = reader.GetAddressInPage();
    instr.handler         = I_PJSR_aa24;
}

inline void D_PJSR_ARn(CodeReader& reader, uint8_t byte, CachedInstruction& instr)
{
    instr.params.op_reg   = byte & 0b111;
    instr.params.br_false = reader.GetAddressInPage();
    instr.handler         = I_PJSR_ARn;
}

inline void D_PJMP_ARn(CodeReader& reader, uint8_t byte, CachedInstruction& instr)
{
    (void)reader;
    instr.params.op_reg = byte & 0b111;
    instr.handler       = I_PJMP_ARn;
}

inline void D_JMP_aa16(CodeReader& reader, uint8_t byte, CachedInstruction& instr)
{
    (void)byte;

    instr.params.br_true = reader.ReadU16();
    instr.handler        = I_JMP_aa16;
}

inline void D_JSR_aa16(CodeReader& reader, uint8_t byte, CachedInstruction& instr)
{
    (void)byte;
    instr.params.br_true  = reader.ReadU16();
    instr.params.br_false = reader.GetAddressInPage();
    instr.handler         = I_JSR_aa16;
}

inline void D_JSR_ARn(CodeReader& reader, uint8_t byte, CachedInstruction& instr)
{
    instr.params.op_reg   = byte & 0b111;
    instr.params.br_false = reader.GetAddressInPage();
    instr.handler         = I_JSR_ARn;
}

inline void D_RTD_imm8(CodeReader& reader, uint8_t byte, CachedInstruction& instr)
{
    (void)byte;
    instr.params.op_data = reader.ReadU8();
    instr.handler        = I_RTD_immXX;
}

inline void D_RTD_imm16(CodeReader& reader, uint8_t byte, CachedInstruction& instr)
{
    (void)byte;
    instr.params.op_data = reader.ReadU16();
    instr.handler        = I_RTD_immXX;
}

inline void D_JMP(CodeReader& reader, uint8_t byte, CachedInstruction& instr)
{
    (void)byte;

    const uint8_t kind = reader.ReadU8();

    // TODO - not all kinds implemented
    switch (kind & 0b11111000)
    {
    // JMP @Rn
    case 0b11010000:
        D_JMP_ARn(reader, kind, instr);
        break;

    // PJMP @Rn
    case 0b11000000:
        D_PJMP_ARn(reader, kind, instr);
        break;

    // PJSR @Rn
    case 0b11001000:
        D_PJSR_ARn(reader, kind, instr);
        break;

    // JSR @Rn
    case 0b11011000:
        D_JSR_ARn(reader, kind, instr);
        break;

    default:
        if (kind == 0b00011001)
        {
            D_PRTS(reader, kind, instr);
        }
        else
        {
            FatalError(reader.GetMCU());
        }
    }
}

inline void D_TRAPA(CodeReader& reader, uint8_t byte, CachedInstruction& instr)
{
    (void)byte;

    const uint8_t vec_byte = reader.ReadU8();

    instr.params.op_data = vec_byte & 0b1111;
    instr.handler        = I_TRAPA_imm4;
}

inline void D_SLEEP(CodeReader& reader, uint8_t byte, CachedInstruction& instr)
{
    (void)reader;
    (void)byte;

    instr.handler = I_SLEEP;
}

inline void D_STM(CodeReader& reader, uint8_t byte, CachedInstruction& instr)
{
    (void)byte;
    const uint8_t reglist = reader.ReadU8();
    if (reglist == 127)
    {
        // specialize the most commonly used form of this instruction
        instr.handler = I_STM_Fast<127>;
    }
    else
    {
        instr.params.op_data = reglist;
        instr.handler        = I_STM;
    }
}

inline void D_LDM(CodeReader& reader, uint8_t byte, CachedInstruction& instr)
{
    (void)byte;

    const uint8_t reglist = reader.ReadU8();
    if (reglist == 127)
    {
        // specialize the most commonly used form of this instruction
        instr.handler = I_LDM_Fast<127>;
    }
    else
    {
        instr.params.op_data = reglist;
        instr.handler        = I_LDM;
    }
}

inline void D_BSR_d8(CodeReader& reader, uint8_t byte, CachedInstruction& instr)
{
    (void)byte;

    const int8_t disp = (int8_t)reader.ReadU8();
    MakeBranch(reader, instr, I_BSR, disp);
}

inline void D_BSR_d16(CodeReader& reader, uint8_t byte, CachedInstruction& instr)
{
    (void)byte;

    const uint16_t disp = reader.ReadU16();
    MakeBranch(reader, instr, I_BSR, (int16_t)disp);
}

//=============================================================================
// Short format instructions
//=============================================================================
template <uint8_t Rn>
inline void D_Short_CMP_E_imm8_Rd(CodeReader& reader, uint8_t byte, CachedInstruction& instr)
{
    (void)byte;
    instr.params.op_data = reader.ReadU8();
    instr.params.ea_reg  = Rn;
    instr.handler        = I_CMP_E_imm8_Rd<Rn>;
}

template <uint8_t Rn>
inline void D_Short_CMP_I_W_imm16_Rd(CodeReader& reader, uint8_t byte, CachedInstruction& instr)
{
    (void)byte;

    instr.params.op_data = reader.ReadU16();
    instr.params.ea_reg  = Rn;
    instr.handler        = I_CMP_I_W_imm16_Rd<Rn>;
}

template <uint8_t Rn>
inline void D_Short_MOV_E_imm8_Rd(CodeReader& reader, uint8_t byte, CachedInstruction& instr)
{
    (void)byte;
    instr.params.op_data = reader.ReadU8();
    instr.params.ea_reg  = Rn;
    instr.handler        = I_MOV_E_imm8_Rd<Rn>;
}

template <uint8_t Rn>
inline void D_Short_MOV_I_W_imm16_Rd(CodeReader& reader, uint8_t byte, CachedInstruction& instr)
{
    (void)byte;
    instr.params.op_data = reader.ReadU16();
    instr.params.ea_reg  = Rn;
    instr.handler        = I_MOV_I_W_imm16_Rd<Rn>;
}

template <Size Sz, uint8_t Rn>
inline void D_Short_MOV_L_aa8_Rd(CodeReader& reader, uint8_t byte, CachedInstruction& instr)
{
    (void)byte;
    instr.params.ea_data = reader.ReadU8();
    instr.params.op_reg  = Rn;
    instr.handler        = I_MOV_L_aa8_Rd<Sz, Rn>;
}

template <Size Sz, uint8_t Rn>
inline void D_Short_MOV_S_Rs_aa8(CodeReader& reader, uint8_t byte, CachedInstruction& instr)
{
    (void)byte;
    instr.params.op_reg  = Rn;
    instr.params.ea_data = reader.ReadU8();
    instr.handler        = I_MOV_S_Rs_aa8<Sz, Rn>;
}

} // namespace decoder2
