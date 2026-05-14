#include "decoder_handlers.h"

#include "disassemble.h"

namespace decoder2
{

void Dis_MOV_G_EAs_Rd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    (void)decoder;
    instr.instr          = InstructionType::MOV;
    instr.op[0].location = OperandLocation::EA;
    instr.op[1].location = OperandLocation::R;
    instr.op[1].reg      = byte & 0b111;
    instr.format         = InstructionFormat::G;
}

void Dis_MOV_G_Rs_EAd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    (void)decoder;
    instr.instr          = InstructionType::MOV;
    instr.op[0].location = OperandLocation::R;
    instr.op[0].imm      = byte & 0b111;
    instr.op[1].location = OperandLocation::EA;
    instr.format         = InstructionFormat::G;
}

void Dis_MOV_G_imm8_EAd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    (void)byte;
    instr.instr          = InstructionType::MOV;
    instr.op[0].location = OperandLocation::imm;
    instr.op[0].imm      = decoder.ReadAdvance();
    instr.op[1].location = OperandLocation::EA;
    instr.format         = InstructionFormat::G;
}

void Dis_MOV_G_imm16_EAd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    (void)byte;
    instr.instr          = InstructionType::MOV;
    instr.op[0].location = OperandLocation::imm;
    instr.op[1].location = OperandLocation::EA;
    instr.op[0].imm      = decoder.ReadU16();
    instr.format         = InstructionFormat::G;
}

void Dis_LDM_SP_Reglist(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    (void)byte;
    instr.instr          = InstructionType::LDM;
    instr.op[0].location = OperandLocation::imm;
    instr.op[0].imm      = decoder.ReadAdvance();
}

void Dis_STM_Reglist_SP(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    (void)byte;
    // TODO revisit this representation
    instr.instr          = InstructionType::STM;
    instr.op[0].location = OperandLocation::imm;
    instr.op[0].imm      = decoder.ReadAdvance();
}

void Dis_ADD_G_EAs_Rd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    (void)decoder;
    instr.instr          = InstructionType::ADD;
    instr.op[0].location = OperandLocation::EA;
    instr.op[1].location = OperandLocation::R;
    instr.op[1].reg      = byte & 0b111;
    instr.format         = InstructionFormat::G;
}

void Dis_XCH_Rs_Rd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    (void)decoder;
    instr.instr          = InstructionType::XCH;
    instr.op[0].location = OperandLocation::EA;
    instr.op[1].location = OperandLocation::R;
    instr.op[1].reg      = byte & 0b111;
}

void Dis_SWAP_Rd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    (void)decoder;
    (void)byte;
    instr.instr          = InstructionType::SWAP;
    instr.op[0].location = OperandLocation::EA;
}

void Dis_MOVTPE_Rs_EAd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    Dis_NotImplemented(decoder, byte, instr);
}

void Dis_MOVTPE_EAS_Rd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    Dis_NotImplemented(decoder, byte, instr);
}

void Dis_ADD_Q_1_EAd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    (void)decoder;
    (void)byte;
    // TODO: maybe we should make this a special ADDQ since the imm is not actually an operand?
    instr.instr          = InstructionType::ADD;
    instr.op[0].location = OperandLocation::imm;
    instr.op[0].imm      = 1;
    instr.op[1].location = OperandLocation::EA;
    instr.format         = InstructionFormat::Q;
}

void Dis_ADD_Q_2_EAd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    (void)decoder;
    (void)byte;
    instr.instr          = InstructionType::ADD;
    instr.op[0].location = OperandLocation::imm;
    instr.op[0].imm      = 2;
    instr.op[1].location = OperandLocation::EA;
    instr.format         = InstructionFormat::Q;
}

void Dis_ADD_Q_M1_EAd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    (void)decoder;
    (void)byte;
    instr.instr          = InstructionType::ADD;
    instr.op[0].location = OperandLocation::imm;
    instr.op[0].imm      = static_cast<uint16_t>(-1);
    instr.op[1].location = OperandLocation::EA;
    instr.format         = InstructionFormat::Q;
}

void Dis_ADD_Q_M2_EAd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    (void)decoder;
    (void)byte;
    instr.instr          = InstructionType::ADD;
    instr.op[0].location = OperandLocation::imm;
    instr.op[0].imm      = static_cast<uint16_t>(-2);
    instr.op[1].location = OperandLocation::EA;
    instr.format         = InstructionFormat::Q;
}

void Dis_ADDS_EAs_Rd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    (void)decoder;
    instr.instr          = InstructionType::ADDS;
    instr.op[0].location = OperandLocation::EA;
    instr.op[1].location = OperandLocation::R;
    instr.op[1].reg      = byte & 0b111;
}

void Dis_ADDX_EAs_Rd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    (void)decoder;
    instr.instr          = InstructionType::ADDX;
    instr.op[0].location = OperandLocation::EA;
    instr.op[1].location = OperandLocation::R;
    instr.op[1].reg      = byte & 0b111;
}

void Dis_DADD_Rs_Rd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    (void)decoder;
    instr.instr          = InstructionType::DADD;
    instr.op[0].location = OperandLocation::EA;
    instr.op[1].location = OperandLocation::R;
    instr.op[1].reg      = byte & 0b111;
}

void Dis_SUB_EAs_Rd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    (void)decoder;
    instr.instr          = InstructionType::SUB;
    instr.op[0].location = OperandLocation::EA;
    instr.op[1].location = OperandLocation::R;
    instr.op[1].reg      = byte & 0b111;
}

void Dis_SUBS_EAs_Rd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    (void)decoder;
    instr.instr          = InstructionType::SUBS;
    instr.op[0].location = OperandLocation::EA;
    instr.op[1].location = OperandLocation::R;
    instr.op[1].reg      = byte & 0b111;
}

void Dis_SUBX_EAs_Rd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    (void)decoder;
    instr.instr          = InstructionType::SUBX;
    instr.op[0].location = OperandLocation::EA;
    instr.op[1].location = OperandLocation::R;
    instr.op[1].reg      = byte & 0b111;
}

void Dis_DSUB_Rs_Rd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    Dis_NotImplemented(decoder, byte, instr);
}

void Dis_MULXU_EAs_Rd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    (void)decoder;
    instr.instr          = InstructionType::MULXU;
    instr.op[0].location = OperandLocation::EA;
    instr.op[1].location = OperandLocation::R;
    instr.op[1].reg      = byte & 0b111;
}

void Dis_DIVXU_EAs_Rd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    (void)decoder;
    instr.instr          = InstructionType::DIVXU;
    instr.op[0].location = OperandLocation::EA;
    instr.op[1].location = OperandLocation::R;
    instr.op[1].reg      = byte & 0b111;
}

void Dis_CMP_G_EAs_Rd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    (void)decoder;
    instr.instr          = InstructionType::CMP;
    instr.op[0].location = OperandLocation::EA;
    instr.op[1].location = OperandLocation::R;
    instr.op[1].reg      = byte & 0b111;
    instr.format         = InstructionFormat::G;
}

void Dis_CMP_G_imm8_EAd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    (void)byte;
    instr.instr          = InstructionType::CMP;
    instr.op[0].location = OperandLocation::imm;
    instr.op[0].imm      = decoder.ReadAdvance();
    instr.op[1].location = OperandLocation::EA;
    instr.format         = InstructionFormat::G;
}

void Dis_CMP_G_imm16_EAd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    (void)byte;
    instr.instr          = InstructionType::CMP;
    instr.op[0].location = OperandLocation::imm;
    instr.op[0].imm      = decoder.ReadU16();
    instr.op[1].location = OperandLocation::EA;
    instr.format         = InstructionFormat::G;
}

void Dis_EXTS_Rd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    (void)decoder;
    (void)byte;
    instr.instr          = InstructionType::EXTS;
    instr.op[0].location = OperandLocation::EA;
}

void Dis_EXTU_Rd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    (void)decoder;
    (void)byte;
    instr.instr          = InstructionType::EXTU;
    instr.op[0].location = OperandLocation::EA;
}

void Dis_TST_EAd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    (void)decoder;
    (void)byte;
    instr.instr          = InstructionType::TST;
    instr.op[0].location = OperandLocation::EA;
}

void Dis_NEG_EAd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    (void)decoder;
    (void)byte;
    instr.instr          = InstructionType::NEG;
    instr.op[0].location = OperandLocation::EA;
}

void Dis_CLR_EAd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    (void)decoder;
    (void)byte;
    instr.instr          = InstructionType::CLR;
    instr.op[0].location = OperandLocation::EA;
}

void Dis_TAS_EAd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    (void)decoder;
    (void)byte;
    instr.instr          = InstructionType::TAS;
    instr.op[0].location = OperandLocation::EA;
}

// Shift instructions
void Dis_SHAL_EAd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    (void)decoder;
    (void)byte;
    instr.instr          = InstructionType::SHAL;
    instr.op[0].location = OperandLocation::EA;
}

void Dis_SHAR_EAd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    (void)decoder;
    (void)byte;
    instr.instr          = InstructionType::SHAR;
    instr.op[0].location = OperandLocation::EA;
}

void Dis_SHLL_EAd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    (void)decoder;
    (void)byte;
    instr.instr          = InstructionType::SHLL;
    instr.op[0].location = OperandLocation::EA;
}

void Dis_SHLR_EAd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    (void)decoder;
    (void)byte;
    instr.instr          = InstructionType::SHLR;
    instr.op[0].location = OperandLocation::EA;
}

void Dis_ROTL_EAd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    (void)decoder;
    (void)byte;
    instr.instr          = InstructionType::ROTL;
    instr.op[0].location = OperandLocation::EA;
}

void Dis_ROTR_EAd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    (void)decoder;
    (void)byte;
    instr.instr          = InstructionType::ROTR;
    instr.op[0].location = OperandLocation::EA;
}

void Dis_ROTXL_EAd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    (void)decoder;
    (void)byte;
    instr.instr          = InstructionType::ROTXL;
    instr.op[0].location = OperandLocation::EA;
}

void Dis_ROTXR_EAd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    (void)decoder;
    (void)byte;
    instr.instr          = InstructionType::ROTXR;
    instr.op[0].location = OperandLocation::EA;
}

// Logic operation instructions
void Dis_AND_EAs_Rd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    (void)decoder;
    instr.instr          = InstructionType::AND;
    instr.op[0].location = OperandLocation::EA;
    instr.op[1].location = OperandLocation::R;
    instr.op[1].reg      = byte & 0b111;
}

void Dis_OR_EAs_Rd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    (void)decoder;
    instr.instr          = InstructionType::OR;
    instr.op[0].location = OperandLocation::EA;
    instr.op[1].location = OperandLocation::R;
    instr.op[1].reg      = byte & 0b111;
}

void Dis_XOR_EAs_Rd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    (void)decoder;
    instr.instr          = InstructionType::XOR;
    instr.op[0].location = OperandLocation::EA;
    instr.op[1].location = OperandLocation::R;
    instr.op[1].reg      = byte & 0b111;
}

void Dis_NOT_EAd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    (void)decoder;
    (void)byte;
    instr.instr          = InstructionType::NOT;
    instr.op[0].location = OperandLocation::EA;
}

// Bit manipulation instructions
void Dis_BSET_imm4_EAd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    (void)decoder;
    instr.instr          = InstructionType::BSET;
    instr.op[0].location = OperandLocation::imm;
    instr.op[0].imm      = byte & 0b1111;
    instr.op[1].location = OperandLocation::EA;
}

void Dis_BSET_Rs_EAd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    (void)decoder;
    instr.instr          = InstructionType::BSET;
    instr.op[0].location = OperandLocation::R;
    instr.op[0].reg      = byte & 0b111;
    instr.op[1].location = OperandLocation::EA;
}

void Dis_BCLR_imm4_EAd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    (void)decoder;
    instr.instr          = InstructionType::BCLR;
    instr.op[0].location = OperandLocation::imm;
    instr.op[0].imm      = byte & 0b1111;
    instr.op[1].location = OperandLocation::EA;
}

void Dis_BCLR_Rs_EAd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    (void)decoder;
    instr.instr          = InstructionType::BCLR;
    instr.op[0].location = OperandLocation::R;
    instr.op[0].reg      = byte & 0b111;
    instr.op[1].location = OperandLocation::EA;
}

void Dis_BTST_imm4_EAd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    (void)decoder;
    instr.instr          = InstructionType::BTST;
    instr.op[0].location = OperandLocation::imm;
    instr.op[0].imm      = byte & 0b1111;
    instr.op[1].location = OperandLocation::EA;
}

void Dis_BTST_Rs_EAd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    (void)decoder;
    instr.instr          = InstructionType::BTST;
    instr.op[0].location = OperandLocation::R;
    instr.op[0].reg      = byte & 0b111;
    instr.op[1].location = OperandLocation::EA;
}

void Dis_BNOT_imm4_EAd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    (void)decoder;
    instr.instr          = InstructionType::BNOT;
    instr.op[0].location = OperandLocation::imm;
    instr.op[0].imm      = byte & 0b1111;
    instr.op[1].location = OperandLocation::EA;
}

void Dis_BNOT_Rs_EAd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    (void)decoder;
    instr.instr          = InstructionType::BNOT;
    instr.op[0].location = OperandLocation::R;
    instr.op[0].reg      = byte & 0b111;
    instr.op[1].location = OperandLocation::EA;
}

// System control instructions
void Dis_LDC_EAs_CR(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    (void)decoder;
    instr.instr          = InstructionType::LDC;
    instr.op[0].location = OperandLocation::EA;
    instr.op[1].location = OperandLocation::CR;
    instr.op[1].cr       = byte & 0b111;
}

void Dis_STC_CR_EAd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    (void)decoder;
    instr.instr          = InstructionType::STC;
    instr.op[0].location = OperandLocation::CR;
    instr.op[0].cr       = byte & 0b111;
    instr.op[1].location = OperandLocation::EA;
}

void Dis_ANDC_imm_CR(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    (void)decoder;
    instr.instr          = InstructionType::ANDC;
    instr.op[0].location = OperandLocation::EA;
    instr.op[1].location = OperandLocation::CR;
    instr.op[1].cr       = byte & 0b111;
}

void Dis_ORC_imm_CR(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    (void)decoder;
    instr.instr          = InstructionType::ORC;
    instr.op[0].location = OperandLocation::EA;
    instr.op[1].location = OperandLocation::CR;
    instr.op[1].cr       = byte & 0b111;
}

void Dis_XORC_imm_CR(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    (void)decoder;
    instr.instr          = InstructionType::XORC;
    instr.op[0].location = OperandLocation::EA;
    instr.op[1].location = OperandLocation::CR;
    instr.op[1].cr       = byte & 0b111;
}

// Short instructions
void Dis_MOV_E_B_imm8_Rd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    instr.instr          = InstructionType::MOV;
    instr.format         = InstructionFormat::E;
    instr.op_size        = OptionalSize::Byte;
    instr.op[0].location = OperandLocation::imm;
    instr.op[0].imm      = decoder.ReadAdvance();
    instr.op[1].location = OperandLocation::R;
    instr.op[1].reg      = byte & 0b111;
}

void Dis_MOV_I_W_imm16_Rd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    instr.instr          = InstructionType::MOV;
    instr.format         = InstructionFormat::I;
    instr.op_size        = OptionalSize::Word;
    instr.op[0].location = OperandLocation::imm;
    instr.op[0].imm      = decoder.ReadU16();
    instr.op[1].location = OperandLocation::R;
    instr.op[1].reg      = byte & 0b111;
}

void Dis_MOV_F_B_d8_R6_Rd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    instr.instr          = InstructionType::MOV;
    instr.op[0].location = OperandLocation::R;
    instr.op[0].reg      = 6;
    instr.op[0].disp     = decoder.ReadAdvance();
    instr.op[1].location = OperandLocation::R;
    instr.op[1].reg      = byte & 0b111;
    instr.format         = InstructionFormat::F;
}

void Dis_MOV_F_W_d8_R6_Rd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    instr.instr          = InstructionType::MOV;
    instr.op[0].location = OperandLocation::R;
    instr.op[0].reg      = 6;
    instr.op[0].disp     = decoder.ReadAdvance();
    instr.op[1].location = OperandLocation::R;
    instr.op[1].reg      = byte & 0b111;
    instr.format         = InstructionFormat::F;
}

void Dis_MOV_F_B_Rs_d8_R6(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    instr.instr          = InstructionType::MOV;
    instr.op[0].location = OperandLocation::R;
    instr.op[0].reg      = byte & 0b111;
    instr.op[1].location = OperandLocation::R;
    instr.op[1].reg      = 6;
    instr.op[1].disp     = decoder.ReadAdvance();
    instr.format         = InstructionFormat::F;
}

void Dis_MOV_F_W_Rs_d8_R6(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    instr.instr          = InstructionType::MOV;
    instr.op[0].location = OperandLocation::R;
    instr.op[0].reg      = byte & 0b111;
    instr.op[1].location = OperandLocation::R;
    instr.op[1].reg      = 6;
    instr.op[1].disp     = decoder.ReadAdvance();
    instr.format         = InstructionFormat::F;
}

void Dis_CMP_E_B_imm8_Rd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    instr.instr          = InstructionType::CMP;
    instr.op[0].location = OperandLocation::imm;
    instr.op[0].imm      = decoder.ReadAdvance();
    instr.op[1].location = OperandLocation::R;
    instr.op[1].reg      = byte & 0b111;
    instr.format         = InstructionFormat::E;
}

void Dis_CMP_I_W_imm16_Rd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    instr.instr          = InstructionType::CMP;
    instr.op[0].location = OperandLocation::imm;
    instr.op[0].imm      = decoder.ReadU16();
    instr.op[1].location = OperandLocation::R;
    instr.op[1].reg      = byte & 0b111;
    instr.format         = InstructionFormat::I;
}

void Dis_BRA(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    (void)byte;
    instr.instr          = InstructionType::BRA;
    instr.op[0].location = OperandLocation::imm;
    instr.op[0].imm      = decoder.ReadAdvance();
}

void Dis_BRN(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    (void)byte;
    instr.instr          = InstructionType::BRN;
    instr.op[0].location = OperandLocation::imm;
    instr.op[0].imm      = decoder.ReadAdvance();
}

void Dis_BHI(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    (void)byte;
    instr.instr          = InstructionType::BHI;
    instr.op[0].location = OperandLocation::imm;
    instr.op[0].imm      = decoder.ReadAdvance();
}

void Dis_BLS(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    (void)byte;
    instr.instr          = InstructionType::BLS;
    instr.op[0].location = OperandLocation::imm;
    instr.op[0].imm      = decoder.ReadAdvance();
}

void Dis_BCC(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    (void)byte;
    instr.instr          = InstructionType::BCC;
    instr.op[0].location = OperandLocation::imm;
    instr.op[0].imm      = decoder.ReadAdvance();
}

void Dis_BCS(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    (void)byte;
    instr.instr          = InstructionType::BCS;
    instr.op[0].location = OperandLocation::imm;
    instr.op[0].imm      = decoder.ReadAdvance();
}

void Dis_BNE(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    (void)byte;
    instr.instr          = InstructionType::BNE;
    instr.op[0].location = OperandLocation::imm;
    instr.op[0].imm      = decoder.ReadAdvance();
}

void Dis_BEQ(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    (void)byte;
    instr.instr          = InstructionType::BEQ;
    instr.op[0].location = OperandLocation::imm;
    instr.op[0].imm      = decoder.ReadAdvance();
}

void Dis_BVC(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    (void)byte;
    instr.instr          = InstructionType::BVC;
    instr.op[0].location = OperandLocation::imm;
    instr.op[0].imm      = decoder.ReadAdvance();
}

void Dis_BVS(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    (void)byte;
    instr.instr          = InstructionType::BVS;
    instr.op[0].location = OperandLocation::imm;
    instr.op[0].imm      = decoder.ReadAdvance();
}

void Dis_BPL(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    (void)byte;
    instr.instr          = InstructionType::BPL;
    instr.op[0].location = OperandLocation::imm;
    instr.op[0].imm      = decoder.ReadAdvance();
}

void Dis_BMI(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    (void)byte;
    instr.instr          = InstructionType::BMI;
    instr.op[0].location = OperandLocation::imm;
    instr.op[0].imm      = decoder.ReadAdvance();
}

void Dis_BGE(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    (void)byte;
    instr.instr          = InstructionType::BGE;
    instr.op[0].location = OperandLocation::imm;
    instr.op[0].imm      = decoder.ReadAdvance();
}

void Dis_BLT(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    (void)byte;
    instr.instr          = InstructionType::BLT;
    instr.op[0].location = OperandLocation::imm;
    instr.op[0].imm      = decoder.ReadAdvance();
}

void Dis_BGT(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    (void)byte;
    instr.instr          = InstructionType::BGT;
    instr.op[0].location = OperandLocation::imm;
    instr.op[0].imm      = decoder.ReadAdvance();
}

void Dis_BLE(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    (void)byte;
    instr.instr          = InstructionType::BLE;
    instr.op[0].location = OperandLocation::imm;
    instr.op[0].imm      = decoder.ReadAdvance();
}

void Dis_BRA16(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    (void)byte;
    instr.instr          = InstructionType::BRA;
    instr.op[0].location = OperandLocation::imm;
    instr.op[0].imm      = decoder.ReadU16();
}

void Dis_BRN16(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    (void)byte;
    instr.instr          = InstructionType::BRN;
    instr.op[0].location = OperandLocation::imm;
    instr.op[0].imm      = decoder.ReadU16();
}

void Dis_BHI16(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    (void)byte;
    instr.instr          = InstructionType::BHI;
    instr.op[0].location = OperandLocation::imm;
    instr.op[0].imm      = decoder.ReadU16();
}

void Dis_BLS16(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    (void)byte;
    instr.instr          = InstructionType::BLS;
    instr.op[0].location = OperandLocation::imm;
    instr.op[0].imm      = decoder.ReadU16();
}

void Dis_BCC16(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    (void)byte;
    instr.instr          = InstructionType::BCC;
    instr.op[0].location = OperandLocation::imm;
    instr.op[0].imm      = decoder.ReadU16();
}

void Dis_BCS16(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    (void)byte;
    instr.instr          = InstructionType::BCS;
    instr.op[0].location = OperandLocation::imm;
    instr.op[0].imm      = decoder.ReadU16();
}

void Dis_BNE16(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    (void)byte;
    instr.instr          = InstructionType::BNE;
    instr.op[0].location = OperandLocation::imm;
    instr.op[0].imm      = decoder.ReadU16();
}

void Dis_BEQ16(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    (void)byte;
    instr.instr          = InstructionType::BEQ;
    instr.op[0].location = OperandLocation::imm;
    instr.op[0].imm      = decoder.ReadU16();
}

void Dis_BVC16(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    (void)byte;
    instr.instr          = InstructionType::BVC;
    instr.op[0].location = OperandLocation::imm;
    instr.op[0].imm      = decoder.ReadU16();
}

void Dis_BVS16(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    (void)byte;
    instr.instr          = InstructionType::BVS;
    instr.op[0].location = OperandLocation::imm;
    instr.op[0].imm      = decoder.ReadU16();
}

void Dis_BPL16(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    (void)byte;
    instr.instr          = InstructionType::BPL;
    instr.op[0].location = OperandLocation::imm;
    instr.op[0].imm      = decoder.ReadU16();
}

void Dis_BMI16(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    (void)byte;
    instr.instr          = InstructionType::BMI;
    instr.op[0].location = OperandLocation::imm;
    instr.op[0].imm      = decoder.ReadU16();
}

void Dis_BGE16(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    (void)byte;
    instr.instr          = InstructionType::BGE;
    instr.op[0].location = OperandLocation::imm;
    instr.op[0].imm      = decoder.ReadU16();
}

void Dis_BLT16(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    (void)byte;
    instr.instr          = InstructionType::BLT;
    instr.op[0].location = OperandLocation::imm;
    instr.op[0].imm      = decoder.ReadU16();
}

void Dis_BGT16(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    (void)byte;
    instr.instr          = InstructionType::BGT;
    instr.op[0].location = OperandLocation::imm;
    instr.op[0].imm      = decoder.ReadU16();
}

void Dis_BLE16(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    (void)byte;
    instr.instr          = InstructionType::BLE;
    instr.op[0].location = OperandLocation::imm;
    instr.op[0].imm      = decoder.ReadU16();
}

void Dis_GenericJump(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    (void)byte;

    const uint8_t opcode = decoder.ReadAdvance();

    switch (opcode & 0b11111000)
    {
    // JMP @Rn
    case 0b11010000:
        instr.instr          = InstructionType::JMP;
        instr.op[0].location = OperandLocation::R;
        instr.op[0].reg      = opcode & 0b111;
        break;

    // JMP @(d:8,Rn)
    case 0b11100000:
        instr.instr          = InstructionType::JMP;
        instr.op[0].location = OperandLocation::R;
        instr.op[0].reg      = opcode & 0b111;
        instr.op[0].disp     = (int8_t)decoder.ReadAdvance();
        break;

    // JMP @(d:16,Rn)
    case 0b11110000:
        instr.instr          = InstructionType::JMP;
        instr.op[0].location = OperandLocation::R;
        instr.op[0].reg      = opcode & 0b111;
        instr.op[0].disp     = (int16_t)decoder.ReadU16();
        break;

    // JSR @Rn
    case 0b11011000:
        instr.instr          = InstructionType::JSR;
        instr.op[0].location = OperandLocation::R;
        instr.op[0].reg      = opcode & 0b111;
        break;

    // JSR @(d:8,Rn)
    case 0b11101000:
        instr.instr          = InstructionType::JSR;
        instr.op[0].location = OperandLocation::R;
        instr.op[0].reg      = opcode & 0b111;
        instr.op[0].disp     = (int8_t)decoder.ReadAdvance();
        break;

    // JSR @(d:16,Rn)
    case 0b11111000:
        instr.instr          = InstructionType::JSR;
        instr.op[0].location = OperandLocation::R;
        instr.op[0].reg      = opcode & 0b111;
        instr.op[0].disp     = (int16_t)decoder.ReadU16();
        break;

    // PJMP @Rn
    case 0b11000000:
        instr.instr          = InstructionType::PJMP;
        instr.op[0].location = OperandLocation::R;
        instr.op[0].reg      = opcode & 0b111;
        break;

    // PJSR @Rn
    case 0b11001000:
        instr.instr          = InstructionType::PJSR;
        instr.op[0].location = OperandLocation::R;
        instr.op[0].reg      = opcode & 0b111;
        break;

    default:
        // PRTS
        if (opcode == 0b00011001)
        {
            instr.instr = InstructionType::PRTS;
        }
        // PRTD #xx:8
        else if (opcode == 0b00010100)
        {
            instr.instr          = InstructionType::PRTD;
            instr.op[0].location = OperandLocation::imm;
            instr.op[0].imm      = decoder.ReadAdvance();
        }
        // PRTD #xx:16
        else if (opcode == 0b00011100)
        {
            instr.instr          = InstructionType::PRTD;
            instr.op[0].location = OperandLocation::imm;
            instr.op[0].imm      = decoder.ReadU16();
        }
        else
        {
            DisassembleError err;
            err.code     = DisassembleErrorCode::InvalidGenericJump;
            err.position = decoder.GetPosition() - 1;
            err.message  = "Unrecognized generic jump";
            decoder.SetError(err);
        }
    }
}

void Dis_JMP_aa16(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    (void)byte;
    instr.instr          = InstructionType::JMP;
    instr.op[0].location = OperandLocation::imm;
    instr.op[0].imm      = decoder.ReadU16();
}

void Dis_BSR_d8(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    (void)byte;
    instr.instr          = InstructionType::BSR;
    instr.op[0].location = OperandLocation::imm;
    instr.op[0].imm      = decoder.ReadAdvance();
}

void Dis_BSR_d16(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    (void)byte;
    instr.instr          = InstructionType::BSR;
    instr.op[0].location = OperandLocation::imm;
    instr.op[0].imm      = decoder.ReadU16();
}

void Dis_JSR_aa16(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    (void)byte;
    instr.instr          = InstructionType::JSR;
    instr.op[0].location = OperandLocation::imm;
    instr.op[0].imm      = decoder.ReadU16();
}

void Dis_RTS(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    (void)decoder;
    (void)byte;
    instr.instr = InstructionType::RTS;
}

void Dis_RTD_imm8(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    (void)byte;
    instr.instr          = InstructionType::RTD;
    instr.op[0].location = OperandLocation::imm;
    instr.op[0].imm      = decoder.ReadAdvance();
}

void Dis_RTD_imm16(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    (void)byte;
    instr.instr          = InstructionType::RTD;
    instr.op[0].location = OperandLocation::imm;
    instr.op[0].imm      = decoder.ReadU16();
}

void Dis_NOP(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    (void)decoder;
    (void)byte;
    instr.instr = InstructionType::NOP;
}

void Dis_NotImplemented(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    (void)decoder;
    (void)byte;
    (void)instr;
    throw "Instruction not implemented";
}

void Dis_InvalidInstruction(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    (void)decoder;
    (void)byte;
    (void)instr;
    throw "Instruction invalid";
}

void Dis_SCB_F(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    (void)byte;

    const uint8_t reg_byte = decoder.ReadAdvance();

    if ((reg_byte & 0b11111000) != 0b10111000)
    {
        decoder.SetError({
            .code     = DisassembleErrorCode::InvalidInstructionFormat,
            .position = decoder.GetPosition(),
            .message  = "SCB/F: second byte is not 10111rrr",
        });
        return;
    }

    uint8_t disp         = decoder.ReadAdvance();
    instr.instr          = InstructionType::SCB_F;
    instr.op[0].location = OperandLocation::R;
    instr.op[0].reg      = reg_byte & 0b111;
    instr.op[1].location = OperandLocation::imm;
    instr.op[1].imm      = disp;
}

void Dis_SCB_NE(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    (void)byte;

    const uint8_t reg_byte = decoder.ReadAdvance();

    if ((reg_byte & 0b11111000) != 0b10111000)
    {
        decoder.SetError({
            .code     = DisassembleErrorCode::InvalidInstructionFormat,
            .position = decoder.GetPosition(),
            .message  = "SCB/NE: second byte is not 10111rrr",
        });
        return;
    }

    uint8_t disp         = decoder.ReadAdvance();
    instr.instr          = InstructionType::SCB_NE;
    instr.op[0].location = OperandLocation::R;
    instr.op[0].reg      = reg_byte & 0b111;
    instr.op[1].location = OperandLocation::imm;
    instr.op[1].imm      = disp;
}

void Dis_SCB_EQ(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    (void)byte;

    const uint8_t reg_byte = decoder.ReadAdvance();

    if ((reg_byte & 0b11111000) != 0b10111000)
    {
        decoder.SetError({
            .code     = DisassembleErrorCode::InvalidInstructionFormat,
            .position = decoder.GetPosition(),
            .message  = "SCB/EQ: second byte is not 10111rrr",
        });
        return;
    }

    uint8_t disp         = decoder.ReadAdvance();
    instr.instr          = InstructionType::SCB_EQ;
    instr.op[0].location = OperandLocation::R;
    instr.op[0].reg      = reg_byte & 0b111;
    instr.op[1].location = OperandLocation::imm;
    instr.op[1].imm      = disp;
}

void Dis_MOV_L_B_aa8_Rd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    const uint8_t addr = decoder.ReadAdvance();

    instr.instr          = InstructionType::MOV;
    instr.format         = InstructionFormat::L;
    instr.op_size        = OptionalSize::Byte;
    instr.op[0].location = OperandLocation::imm;
    instr.op[0].imm      = addr;
    instr.op[1].location = OperandLocation::R;
    instr.op[1].reg      = byte & 0b111;
}

void Dis_MOV_L_W_aa8_Rd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    const uint8_t addr = decoder.ReadAdvance();

    instr.instr          = InstructionType::MOV;
    instr.format         = InstructionFormat::L;
    instr.op_size        = OptionalSize::Word;
    instr.op[0].location = OperandLocation::imm;
    instr.op[0].imm      = addr;
    instr.op[1].location = OperandLocation::R;
    instr.op[1].reg      = byte & 0b111;
}

void Dis_MOV_S_B_Rs_aa8(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    const uint8_t addr = decoder.ReadAdvance();

    instr.instr          = InstructionType::MOV;
    instr.format         = InstructionFormat::S;
    instr.op_size        = OptionalSize::Byte;
    instr.op[0].location = OperandLocation::R;
    instr.op[0].reg      = byte & 0b111;
    instr.op[1].location = OperandLocation::imm;
    instr.op[1].imm      = addr;
}

void Dis_MOV_S_W_Rs_aa8(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    const uint8_t addr = decoder.ReadAdvance();

    instr.instr          = InstructionType::MOV;
    instr.format         = InstructionFormat::S;
    instr.op_size        = OptionalSize::Word;
    instr.op[0].location = OperandLocation::R;
    instr.op[0].reg      = byte & 0b111;
    instr.op[1].location = OperandLocation::imm;
    instr.op[1].imm      = addr;
}

void Dis_MOV_E_imm8_Rd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    const uint8_t data = decoder.ReadAdvance();

    instr.instr          = InstructionType::MOV;
    instr.format         = InstructionFormat::E;
    instr.op_size        = OptionalSize::Byte;
    instr.op[0].location = OperandLocation::imm;
    instr.op[0].imm      = data;
    instr.op[1].location = OperandLocation::R;
    instr.op[1].reg      = byte & 0b111;
}

void Dis_TRAPA_imm4(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    (void)byte;

    const uint8_t vec_byte = decoder.ReadAdvance();

    if ((vec_byte & 0b11110000) != 0b00010000)
    {
        decoder.SetError({
            .code     = DisassembleErrorCode::InvalidInstructionFormat,
            .position = decoder.GetPosition(),
            .message  = "TRAPA: second byte is not 0001#VEC",
        });
        return;
    }

    instr.instr          = InstructionType::TRAPA;
    instr.op[0].location = OperandLocation::imm;
    instr.op[0].imm      = vec_byte & 0b1111;
}

void Dis_PJMP_aa24(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    (void)byte;

    const uint8_t  page = decoder.ReadAdvance();
    const uint16_t addr = decoder.ReadU16();

    instr.instr          = InstructionType::PJMP;
    instr.op[0].location = OperandLocation::imm;
    instr.op[0].imm      = page;
    instr.op[1].location = OperandLocation::imm;
    instr.op[1].imm      = addr;
}

void Dis_PJSR_aa24(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    (void)byte;

    const uint8_t  page = decoder.ReadAdvance();
    const uint16_t addr = decoder.ReadU16();

    instr.instr          = InstructionType::PJSR;
    instr.op[0].location = OperandLocation::imm;
    instr.op[0].imm      = page;
    instr.op[1].location = OperandLocation::imm;
    instr.op[1].imm      = addr;
}

void Dis_RTE(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    (void)decoder;
    (void)byte;
    instr.instr = InstructionType::RTE;
}

void Dis_SLEEP(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    (void)decoder;
    (void)byte;
    instr.instr = InstructionType::SLEEP;
}

} // namespace decoder2
