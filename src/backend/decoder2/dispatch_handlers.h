// This file contains functions that decode an instruction and call into its
// instruction handler. These functions are instantiated by individual
// addressing mode dispatchers and mapped to one of 256 opcodes within that
// addressing mode.
//
// The addressing mode is passed as a template parameter and forwarded to the
// underlying instruction handler so it knows how to extract information from
// the EA field without having to determine the addressing mode at runtime.

#pragma once

#include "decoder2/code_reader.h"
#include "dispatch.h"
#include "instruction_handlers.h"
#include "types.h"

namespace decoder2
{

//=============================================================================
// Helpers
//=============================================================================
inline void MakeBranch(CodeReader& reader, DecodedInstruction& instr, InstructionHandler handler, int16_t disp)
{
    const uint16_t next_ip = reader.GetAddressInPage();
    instr.params.br_true   = (uint16_t)(next_ip + disp);
    instr.params.br_false  = next_ip;
    instr.handler          = handler;
}

//=============================================================================
// Pseudo instructions
//=============================================================================
inline DecodeError D_InvalidInstruction(CodeReader& reader, uint8_t byte, DecodedInstruction& instr)
{
    (void)reader;
    (void)byte;
    (void)instr;
    return DecodeError::UnrecognizedInstruction;
}

//=============================================================================
// General format instructions
//=============================================================================
template <Size Sz, uint8_t OpReg, typename Mode>
DecodeError D_MOV_G_EAs_Rd(CodeReader& reader, uint8_t byte, DecodedInstruction& instr)
{
    (void)reader;
    (void)byte;

    instr.params.op_reg = OpReg;
    instr.handler       = I_MOV_G_EAs_Rd<Sz, Mode>;

    return {};
}

template <Size Sz, uint8_t OpReg, typename Mode>
DecodeError D_MOV_G_Rs_EAd(CodeReader& reader, uint8_t byte, DecodedInstruction& instr)
{
    (void)reader;
    (void)byte;

    instr.params.op_reg = OpReg;
    instr.handler       = I_MOV_G_Rs_EAd<Sz, Mode>;

    return {};
}

template <Size Sz, typename Mode>
DecodeError D_MOV_G_imm8_EAd(CodeReader& reader, uint8_t byte, DecodedInstruction& instr)
{
    (void)byte;

    uint8_t   op_data;
    ReadError err;

    err = reader.ReadU8(op_data);
    if (err != ReadError{})
    {
        return DecodeError::NeedMoreBytes;
    }

    instr.params.op_data = op_data;
    instr.handler        = I_MOV_G_imm8_EAd<Sz, Mode>;

    return {};
}

template <Size Sz, typename Mode>
DecodeError D_MOV_G_imm16_EAd(CodeReader& reader, uint8_t byte, DecodedInstruction& instr)
{
    (void)byte;

    uint16_t  op_data;
    ReadError err;

    err = reader.ReadU16(op_data);
    if (err != ReadError{})
    {
        return DecodeError::NeedMoreBytes;
    }

    instr.params.op_data = op_data;
    instr.handler        = I_MOV_G_imm16_EAd<Sz, Mode>;

    return {};
}

template <Size Sz, typename Mode>
DecodeError D_CMP_G_imm8_EAd(CodeReader& reader, uint8_t byte, DecodedInstruction& instr)
{
    (void)byte;

    uint8_t   op_data;
    ReadError err;

    err = reader.ReadU8(op_data);
    if (err != ReadError{})
    {
        return DecodeError::NeedMoreBytes;
    }

    instr.params.op_data = op_data;
    instr.handler        = I_CMP_G_imm8_EAd<Sz, Mode>;

    return {};
}

template <Size Sz, typename Mode>
DecodeError D_CMP_G_imm16_EAd(CodeReader& reader, uint8_t byte, DecodedInstruction& instr)
{
    (void)byte;

    uint16_t  op_data;
    ReadError err;

    err = reader.ReadU16(op_data);
    if (err != ReadError{})
    {
        return DecodeError::NeedMoreBytes;
    }

    instr.params.op_data = op_data;
    instr.handler        = I_CMP_G_imm16_EAd<Sz, Mode>;

    return {};
}

// Manual lists this as a special format instruction but it allows arbitrary
// addressing modes so we treat it as a general instruction.
template <Size Sz, typename Mode, int8_t N>
DecodeError D_ADD_Q_n_EAd(CodeReader& reader, uint8_t byte, DecodedInstruction& instr)
{
    (void)reader;
    (void)byte;

    instr.handler = I_ADD_Q_n<Sz, Mode, N>;

    return {};
}

template <typename Mode>
DecodeError D_SWAP_Rd(CodeReader& reader, uint8_t byte, DecodedInstruction& instr)
{
    (void)reader;
    (void)byte;

    instr.handler = I_SWAP_B_Rd<Mode>;

    return {};
}

template <uint8_t OpReg, typename Mode>
DecodeError D_XCH_Rs_Rd(CodeReader& reader, uint8_t byte, DecodedInstruction& instr)
{
    (void)reader;
    (void)byte;

    instr.params.op_reg = OpReg;
    instr.handler       = I_XCH_W_Rs_Rd<Mode>;

    return {};
}

template <Size Sz, uint8_t OpReg, typename Mode>
DecodeError D_ADD_G_EAs_Rd(CodeReader& reader, uint8_t byte, DecodedInstruction& instr)
{
    (void)reader;
    (void)byte;

    instr.params.op_reg = OpReg;
    instr.handler       = I_ADD_G_EAs_Rd<Sz, Mode>;

    return {};
}

template <Size Sz, uint8_t OpReg, typename Mode>
DecodeError D_ADDX_EAs_Rd(CodeReader& reader, uint8_t byte, DecodedInstruction& instr)
{
    (void)reader;
    (void)byte;

    instr.params.op_reg = OpReg;
    instr.handler       = I_ADDX_EAs_Rd<Sz, Mode>;

    return {};
}

template <Size Sz, uint8_t OpReg, typename Mode>
DecodeError D_ADDS_EAs_Rd(CodeReader& reader, uint8_t byte, DecodedInstruction& instr)
{
    (void)reader;
    (void)byte;

    instr.params.op_reg = OpReg;
    instr.handler       = I_ADDS_EAs_Rd<Sz, Mode>;

    return {};
}

template <Size Sz, uint8_t OpReg, typename Mode>
DecodeError D_CMP_G_EAs_Rd(CodeReader& reader, uint8_t byte, DecodedInstruction& instr)
{
    (void)reader;
    (void)byte;

    instr.params.op_reg = OpReg;
    instr.handler       = I_CMP_G_EAs_Rd<Sz, Mode>;

    return {};
}

template <Size Sz, typename Mode>
DecodeError D_SHLL_EAd(CodeReader& reader, uint8_t byte, DecodedInstruction& instr)
{
    (void)reader;
    (void)byte;

    instr.handler = I_SHLL_EAd<Sz, Mode>;

    return {};
}

template <Size Sz, typename Mode>
DecodeError D_SHLR_EAd(CodeReader& reader, uint8_t byte, DecodedInstruction& instr)
{
    (void)reader;
    (void)byte;

    instr.handler = I_SHLR_EAd<Sz, Mode>;

    return {};
}

template <Size Sz, typename Mode>
DecodeError D_SHAL_EAd(CodeReader& reader, uint8_t byte, DecodedInstruction& instr)
{
    (void)reader;
    (void)byte;

    instr.handler = I_SHAL_EAd<Sz, Mode>;

    return {};
}

template <Size Sz, typename Mode>
DecodeError D_SHAR_EAd(CodeReader& reader, uint8_t byte, DecodedInstruction& instr)
{
    (void)reader;
    (void)byte;

    instr.handler = I_SHAR_EAd<Sz, Mode>;

    return {};
}

template <Size Sz, typename Mode>
DecodeError D_NEG_EAd(CodeReader& reader, uint8_t byte, DecodedInstruction& instr)
{
    (void)reader;
    (void)byte;

    instr.handler = I_NEG_EAd<Sz, Mode>;

    return {};
}

template <Size Sz, typename Mode>
DecodeError D_CLR_EAd(CodeReader& reader, uint8_t byte, DecodedInstruction& instr)
{
    (void)reader;
    (void)byte;

    instr.handler = I_CLR_EAd<Sz, Mode>;

    return {};
}

template <Size Sz, typename Mode>
DecodeError D_TST_EAd(CodeReader& reader, uint8_t byte, DecodedInstruction& instr)
{
    (void)reader;
    (void)byte;

    instr.handler = I_TST_EAd<Sz, Mode>;

    return {};
}

template <Size Sz, uint8_t OpReg, typename Mode>
DecodeError D_SUB_EAs_Rd(CodeReader& reader, uint8_t byte, DecodedInstruction& instr)
{
    (void)reader;
    (void)byte;

    instr.params.op_reg = OpReg;
    instr.handler       = I_SUB_EAs_Rd<Sz, Mode>;

    return {};
}

template <Size Sz, uint8_t OpReg, typename Mode>
DecodeError D_SUBS_EAs_Rd(CodeReader& reader, uint8_t byte, DecodedInstruction& instr)
{
    (void)reader;
    (void)byte;

    instr.params.op_reg = OpReg;
    instr.handler       = I_SUBS_EAs_Rd<Sz, Mode>;

    return {};
}

template <Size Sz, uint8_t OpReg, typename Mode>
DecodeError D_SUBX_EAs_Rd(CodeReader& reader, uint8_t byte, DecodedInstruction& instr)
{
    (void)reader;
    (void)byte;

    instr.params.op_reg = OpReg;
    instr.handler       = I_SUBX_EAs_Rd<Sz, Mode>;

    return {};
}

template <typename Mode>
DecodeError D_EXTS_Rd(CodeReader& reader, uint8_t byte, DecodedInstruction& instr)
{
    (void)reader;
    (void)byte;

    static_assert(std::is_same_v<Mode, Mode_Rn>);
    instr.handler = I_EXTS_B_Rd<Mode>;

    return {};
}

template <typename Mode>
DecodeError D_EXTU_Rd(CodeReader& reader, uint8_t byte, DecodedInstruction& instr)
{
    (void)reader;
    (void)byte;

    static_assert(std::is_same_v<Mode, Mode_Rn>);
    instr.handler = I_EXTU_B_Rd<Mode>;

    return {};
}

template <Size Sz, typename Mode>
DecodeError D_NOT_EAd(CodeReader& reader, uint8_t byte, DecodedInstruction& instr)
{
    (void)reader;
    (void)byte;

    instr.handler = I_NOT_EAd<Sz, Mode>;

    return {};
}

template <Size Sz, uint8_t OpReg, typename Mode>
DecodeError D_MULXU_EAs_Rd(CodeReader& reader, uint8_t byte, DecodedInstruction& instr)
{
    (void)reader;
    (void)byte;

    instr.params.op_reg = OpReg;
    instr.handler       = I_MULXU_EAs_Rd<Sz, Mode>;

    return {};
}

template <Size Sz, uint8_t OpReg, typename Mode>
DecodeError D_DIVXU_EAs_Rd(CodeReader& reader, uint8_t byte, DecodedInstruction& instr)
{
    (void)reader;
    (void)byte;

    instr.params.op_reg = OpReg;
    instr.handler       = I_DIVXU_EAs_Rd<Sz, Mode>;

    return {};
}

template <Size Sz, uint8_t Imm4, typename Mode>
DecodeError D_BCLR_imm4_EAd(CodeReader& reader, uint8_t byte, DecodedInstruction& instr)
{
    (void)reader;
    (void)byte;

    instr.params.op_data = Imm4 & 0b1111;
    instr.handler        = I_BCLR_imm4_EAd<Sz, Mode>;

    return {};
}

template <Size Sz, uint8_t Rs, typename Mode>
DecodeError D_BCLR_Rs_EAd(CodeReader& reader, uint8_t byte, DecodedInstruction& instr)
{
    (void)reader;
    (void)byte;

    instr.params.op_reg = Rs;
    instr.handler       = I_BCLR_Rs_EAd<Sz, Mode>;

    return {};
}

template <Size Sz, uint8_t Imm4, typename Mode>
DecodeError D_BNOT_imm4_EAd(CodeReader& reader, uint8_t byte, DecodedInstruction& instr)
{
    (void)reader;
    (void)byte;

    instr.params.op_data = Imm4 & 0b1111;
    instr.handler        = I_BNOT_imm4_EAd<Sz, Mode>;

    return {};
}

template <Size Sz, typename Mode>
DecodeError D_ROTL_EAd(CodeReader& reader, uint8_t byte, DecodedInstruction& instr)
{
    (void)reader;
    (void)byte;

    instr.handler = I_ROTL_EAd<Sz, Mode>;

    return {};
}

template <Size Sz, typename Mode>
DecodeError D_ROTR_EAd(CodeReader& reader, uint8_t byte, DecodedInstruction& instr)
{
    (void)reader;
    (void)byte;

    instr.handler = I_ROTR_EAd<Sz, Mode>;

    return {};
}

template <Size Sz, typename Mode>
DecodeError D_ROTXL_EAd(CodeReader& reader, uint8_t byte, DecodedInstruction& instr)
{
    (void)reader;
    (void)byte;

    instr.handler = I_ROTXL_EAd<Sz, Mode>;

    return {};
}

template <Size Sz, typename Mode>
DecodeError D_ROTXR_EAd(CodeReader& reader, uint8_t byte, DecodedInstruction& instr)
{
    (void)reader;
    (void)byte;

    instr.handler = I_ROTXR_EAd<Sz, Mode>;

    return {};
}

template <Size Sz, uint8_t Imm4, typename Mode>
DecodeError D_BSET_imm4_EAd(CodeReader& reader, uint8_t byte, DecodedInstruction& instr)
{
    (void)reader;
    (void)byte;

    instr.params.op_data = Imm4 & 0b1111;
    instr.handler        = I_BSET_imm4_EAd<Sz, Mode>;

    return {};
}

template <Size Sz, uint8_t Rs, typename Mode>
DecodeError D_BSET_Rs_EAd(CodeReader& reader, uint8_t byte, DecodedInstruction& instr)
{
    (void)reader;
    (void)byte;

    instr.params.op_reg = Rs;
    instr.handler       = I_BSET_Rs_EAd<Sz, Mode>;

    return {};
}

template <Size Sz, uint8_t Imm4, typename Mode>
DecodeError D_BTST_imm4_EAd(CodeReader& reader, uint8_t byte, DecodedInstruction& instr)
{
    (void)reader;
    (void)byte;

    instr.params.op_data = Imm4 & 0b1111;
    instr.handler        = I_BTST_imm4_EAd<Sz, Mode>;

    return {};
}

template <Size Sz, uint8_t OpReg, typename Mode>
DecodeError D_BTST_Rs_EAd(CodeReader& reader, uint8_t byte, DecodedInstruction& instr)
{
    (void)reader;
    (void)byte;

    instr.params.op_reg = OpReg;
    instr.handler       = I_BTST_Rs_EAd<Sz, Mode>;

    return {};
}

template <Size Sz, uint8_t CR, typename Mode>
DecodeError D_STC_CR_EAd(CodeReader& reader, uint8_t byte, DecodedInstruction& instr)
{
    (void)reader;
    (void)byte;

    instr.params.op_c = CR;
    instr.handler     = I_STC_CR_EAd<Sz, Mode>;

    return {};
}

template <Size Sz, uint8_t CR, typename Mode>
DecodeError D_LDC_EAs_CR(CodeReader& reader, uint8_t byte, DecodedInstruction& instr)
{
    (void)reader;
    (void)byte;

    instr.params.op_c = CR;
    instr.handler     = I_LDC_EAs_CR<Sz, Mode>;

    return {};
}

template <Size Sz, uint8_t OpReg, typename Mode>
DecodeError D_XOR_EAs_Rd(CodeReader& reader, uint8_t byte, DecodedInstruction& instr)
{
    (void)reader;
    (void)byte;

    instr.params.op_reg = OpReg;
    instr.handler       = I_XOR_EAs_Rd<Sz, Mode>;

    return {};
}

template <Size Sz, uint8_t OpReg, typename Mode>
DecodeError D_OR_EAs_Rd(CodeReader& reader, uint8_t byte, DecodedInstruction& instr)
{
    (void)reader;
    (void)byte;

    instr.params.op_reg = OpReg;
    instr.handler       = I_OR_EAs_Rd<Sz, Mode>;

    return {};
}

template <Size Sz, uint8_t OpReg, typename Mode>
DecodeError D_AND_EAs_Rd(CodeReader& reader, uint8_t byte, DecodedInstruction& instr)
{
    (void)reader;
    (void)byte;

    instr.params.op_reg = OpReg;
    instr.handler       = I_AND_EAs_Rd<Sz, Mode>;

    return {};
}

template <Size Sz, uint8_t CR, typename Mode>
DecodeError D_ORC_immXX_CR(CodeReader& reader, uint8_t byte, DecodedInstruction& instr)
{
    (void)reader;
    (void)byte;

    instr.params.op_c = CR;
    instr.handler     = I_ORC_immXX_CR<Sz, Mode, CR>;

    return {};
}

template <Size Sz, uint8_t CR, typename Mode>
DecodeError D_ANDC_immXX_CR(CodeReader& reader, uint8_t byte, DecodedInstruction& instr)
{
    (void)reader;
    (void)byte;

    instr.params.op_c = CR;
    instr.handler     = I_ANDC_immXX_CR<Sz, Mode, CR>;

    return {};
}

//=============================================================================
// Special format instructions
//=============================================================================
template <Size Sz>
DecodeError D_Bcc(CodeReader& reader, uint8_t byte, DecodedInstruction& instr)
{
    const uint8_t cond = byte & 0b1111;

    ReadError err;
    int16_t   disp;

    switch (Sz)
    {
    case Size::Byte: {
        int8_t byte_disp;
        err = reader.ReadS8(byte_disp);
        if (err != ReadError{})
        {
            return DecodeError::NeedMoreBytes;
        }
        disp = byte_disp;
        break;
    }
    case Size::Word:
        err = reader.ReadS16(disp);
        if (err != ReadError{})
        {
            return DecodeError::NeedMoreBytes;
        }
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

    return {};
}

inline DecodeError D_NOP(CodeReader& reader, uint8_t byte, DecodedInstruction& instr)
{
    (void)reader;
    (void)byte;

    instr.handler = I_NOP;

    return {};
}

inline DecodeError D_RTE(CodeReader& reader, uint8_t byte, DecodedInstruction& instr)
{
    (void)reader;
    (void)byte;

    instr.handler = I_RTE;

    return {};
}

inline DecodeError D_SCB(CodeReader& reader, uint8_t byte, DecodedInstruction& instr)
{
    ReadError err;
    uint8_t   regcode;
    int8_t    disp;

    err = reader.ReadU8(regcode);
    if (err != ReadError{})
    {
        return DecodeError::NeedMoreBytes;
    }

    err = reader.ReadS8(disp);
    if (err != ReadError{})
    {
        return DecodeError::NeedMoreBytes;
    }

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
            // TODO: attach error context "SCB/F invalid regcode"
            return DecodeError::UnrecognizedInstruction;
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
            // TODO: attach error context "SCB/NE invalid regcode"
            return DecodeError::UnrecognizedInstruction;
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
            // TODO: attach error context "SCB/EQ invalid regcode"
            return DecodeError::UnrecognizedInstruction;
        }
    }
    else
    {
        return DecodeError::UnrecognizedInstruction;
    }

    return {};
}

inline DecodeError D_RTS(CodeReader& reader, uint8_t byte, DecodedInstruction& instr)
{
    (void)reader;
    (void)byte;

    instr.handler = I_RTS;

    return {};
}

inline DecodeError D_PRTS(CodeReader& reader, uint8_t byte, DecodedInstruction& instr)
{
    (void)reader;
    (void)byte;

    instr.handler = I_PRTS;

    return {};
}

inline DecodeError D_JMP_ARn(CodeReader& reader, uint8_t byte, DecodedInstruction& instr)
{
    (void)reader;

    instr.params.op_reg = byte & 0b111;
    instr.handler       = I_JMP_ARn;

    return {};
}

inline DecodeError D_PJMP_aa24(CodeReader& reader, uint8_t byte, DecodedInstruction& instr)
{
    (void)byte;

    ReadError err;
    uint8_t   op_page;
    uint16_t  op_data;

    err = reader.ReadU8(op_page);
    if (err != ReadError{})
    {
        return DecodeError::NeedMoreBytes;
    }

    err = reader.ReadU16(op_data);
    if (err != ReadError{})
    {
        return DecodeError::NeedMoreBytes;
    }

    instr.params.op_page = op_page;
    instr.params.op_data = op_data;
    instr.handler        = I_PJMP_aa24;

    return {};
}

inline DecodeError D_PJSR_aa24(CodeReader& reader, uint8_t byte, DecodedInstruction& instr)
{
    (void)byte;

    ReadError err;
    uint8_t   op_page;
    uint16_t  br_true;

    err = reader.ReadU8(op_page);
    if (err != ReadError{})
    {
        return DecodeError::NeedMoreBytes;
    }

    err = reader.ReadU16(br_true);
    if (err != ReadError{})
    {
        return DecodeError::NeedMoreBytes;
    }

    instr.params.op_page  = op_page;
    instr.params.br_true  = br_true;
    instr.params.br_false = reader.GetAddressInPage();
    instr.handler         = I_PJSR_aa24;

    return {};
}

inline DecodeError D_PJSR_ARn(CodeReader& reader, uint8_t byte, DecodedInstruction& instr)
{
    instr.params.op_reg   = byte & 0b111;
    instr.params.br_false = reader.GetAddressInPage();
    instr.handler         = I_PJSR_ARn;

    return {};
}

inline DecodeError D_PJMP_ARn(CodeReader& reader, uint8_t byte, DecodedInstruction& instr)
{
    (void)reader;
    instr.params.op_reg = byte & 0b111;
    instr.handler       = I_PJMP_ARn;

    return {};
}

inline DecodeError D_JMP_aa16(CodeReader& reader, uint8_t byte, DecodedInstruction& instr)
{
    (void)byte;

    ReadError err;
    uint16_t  br_true;

    err = reader.ReadU16(br_true);
    if (err != ReadError{})
    {
        return DecodeError::NeedMoreBytes;
    }

    instr.params.br_true = br_true;
    instr.handler        = I_JMP_aa16;

    return {};
}

inline DecodeError D_JSR_aa16(CodeReader& reader, uint8_t byte, DecodedInstruction& instr)
{
    (void)byte;

    ReadError err;
    uint16_t  br_true;

    err = reader.ReadU16(br_true);
    if (err != ReadError{})
    {
        return DecodeError::NeedMoreBytes;
    }

    instr.params.br_true  = br_true;
    instr.params.br_false = reader.GetAddressInPage();
    instr.handler         = I_JSR_aa16;

    return {};
}

inline DecodeError D_JSR_ARn(CodeReader& reader, uint8_t byte, DecodedInstruction& instr)
{
    instr.params.op_reg   = byte & 0b111;
    instr.params.br_false = reader.GetAddressInPage();
    instr.handler         = I_JSR_ARn;

    return {};
}

inline DecodeError D_RTD_imm8(CodeReader& reader, uint8_t byte, DecodedInstruction& instr)
{
    (void)byte;

    ReadError err;
    uint8_t   op_data;

    err = reader.ReadU8(op_data);
    if (err != ReadError{})
    {
        return DecodeError::NeedMoreBytes;
    }

    instr.params.op_data = op_data;
    instr.handler        = I_RTD_immXX;

    return {};
}

inline DecodeError D_RTD_imm16(CodeReader& reader, uint8_t byte, DecodedInstruction& instr)
{
    (void)byte;

    ReadError err;
    uint16_t  op_data;

    err = reader.ReadU16(op_data);
    if (err != ReadError{})
    {
        return DecodeError::NeedMoreBytes;
    }

    instr.params.op_data = op_data;
    instr.handler        = I_RTD_immXX;

    return {};
}

inline DecodeError D_JMP(CodeReader& reader, uint8_t byte, DecodedInstruction& instr)
{
    (void)byte;

    ReadError err;
    uint8_t   kind;

    err = reader.ReadU8(kind);
    if (err != ReadError{})
    {
        return DecodeError::NeedMoreBytes;
    }

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
            return DecodeError::UnrecognizedInstruction;
        }
    }

    return {};
}

inline DecodeError D_TRAPA(CodeReader& reader, uint8_t byte, DecodedInstruction& instr)
{
    (void)byte;

    ReadError err;
    uint8_t   vec_byte;

    err = reader.ReadU8(vec_byte);
    if (err != ReadError{})
    {
        return DecodeError::NeedMoreBytes;
    }

    instr.params.op_data = vec_byte & 0b1111;
    instr.handler        = I_TRAPA_imm4;

    return {};
}

inline DecodeError D_SLEEP(CodeReader& reader, uint8_t byte, DecodedInstruction& instr)
{
    (void)reader;
    (void)byte;

    instr.handler = I_SLEEP;

    return {};
}

inline DecodeError D_STM(CodeReader& reader, uint8_t byte, DecodedInstruction& instr)
{
    (void)byte;

    ReadError err;
    uint8_t   reglist;

    err = reader.ReadU8(reglist);
    if (err != ReadError{})
    {
        return DecodeError::NeedMoreBytes;
    }

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

    return {};
}

inline DecodeError D_LDM(CodeReader& reader, uint8_t byte, DecodedInstruction& instr)
{
    (void)byte;

    ReadError err;
    uint8_t   reglist;

    err = reader.ReadU8(reglist);
    if (err != ReadError{})
    {
        return DecodeError::NeedMoreBytes;
    }

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

    return {};
}

inline DecodeError D_BSR_d8(CodeReader& reader, uint8_t byte, DecodedInstruction& instr)
{
    (void)byte;

    ReadError err;
    int8_t    disp;

    err = reader.ReadS8(disp);
    if (err != ReadError{})
    {
        return DecodeError::NeedMoreBytes;
    }

    MakeBranch(reader, instr, I_BSR, disp);

    return {};
}

inline DecodeError D_BSR_d16(CodeReader& reader, uint8_t byte, DecodedInstruction& instr)
{
    (void)byte;

    ReadError err;
    int16_t   disp;

    err = reader.ReadS16(disp);
    if (err != ReadError{})
    {
        return DecodeError::NeedMoreBytes;
    }

    MakeBranch(reader, instr, I_BSR, disp);

    return {};
}

//=============================================================================
// Short format instructions
//=============================================================================
template <uint8_t Rn>
DecodeError D_Short_CMP_E_imm8_Rd(CodeReader& reader, uint8_t byte, DecodedInstruction& instr)
{
    (void)byte;

    ReadError err;
    uint8_t   op_data;

    err = reader.ReadU8(op_data);
    if (err != ReadError{})
    {
        return DecodeError::NeedMoreBytes;
    }

    instr.params.op_data = op_data;
    instr.params.ea_reg  = Rn;
    instr.handler        = I_CMP_E_imm8_Rd<Rn>;

    return {};
}

template <uint8_t Rn>
DecodeError D_Short_CMP_I_W_imm16_Rd(CodeReader& reader, uint8_t byte, DecodedInstruction& instr)
{
    (void)byte;

    ReadError err;
    uint16_t  op_data;

    err = reader.ReadU16(op_data);
    if (err != ReadError{})
    {
        return DecodeError::NeedMoreBytes;
    }

    instr.params.op_data = op_data;
    instr.params.ea_reg  = Rn;
    instr.handler        = I_CMP_I_W_imm16_Rd<Rn>;

    return {};
}

template <uint8_t Rn>
DecodeError D_Short_MOV_E_imm8_Rd(CodeReader& reader, uint8_t byte, DecodedInstruction& instr)
{
    (void)byte;

    ReadError err;
    uint8_t   op_data;

    err = reader.ReadU8(op_data);
    if (err != ReadError{})
    {
        return DecodeError::NeedMoreBytes;
    }

    instr.params.op_data = op_data;
    instr.params.ea_reg  = Rn;
    instr.handler        = I_MOV_E_imm8_Rd<Rn>;

    return {};
}

template <uint8_t Rn>
DecodeError D_Short_MOV_I_W_imm16_Rd(CodeReader& reader, uint8_t byte, DecodedInstruction& instr)
{
    (void)byte;

    ReadError err;
    uint16_t  op_data;

    err = reader.ReadU16(op_data);
    if (err != ReadError{})
    {
        return DecodeError::NeedMoreBytes;
    }

    instr.params.op_data = op_data;
    instr.params.ea_reg  = Rn;
    instr.handler        = I_MOV_I_W_imm16_Rd<Rn>;

    return {};
}

template <Size Sz, uint8_t Rn>
DecodeError D_Short_MOV_L_aa8_Rd(CodeReader& reader, uint8_t byte, DecodedInstruction& instr)
{
    (void)byte;

    ReadError err;
    uint8_t   ea_data;

    err = reader.ReadU8(ea_data);
    if (err != ReadError{})
    {
        return DecodeError::NeedMoreBytes;
    }

    instr.params.ea_data = ea_data;
    instr.params.op_reg  = Rn;
    instr.handler        = I_MOV_L_aa8_Rd<Sz, Rn>;

    return {};
}

template <Size Sz, uint8_t Rn>
DecodeError D_Short_MOV_S_Rs_aa8(CodeReader& reader, uint8_t byte, DecodedInstruction& instr)
{
    (void)byte;

    ReadError err;
    uint8_t   ea_data;

    err = reader.ReadU8(ea_data);
    if (err != ReadError{})
    {
        return DecodeError::NeedMoreBytes;
    }

    instr.params.op_reg  = Rn;
    instr.params.ea_data = ea_data;
    instr.handler        = I_MOV_S_Rs_aa8<Sz, Rn>;

    return {};
}

} // namespace decoder2
