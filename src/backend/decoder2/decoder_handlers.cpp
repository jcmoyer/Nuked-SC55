#include "decoder_handlers.h"

#include "disassemble.h"

void Dis_MOV_G_EAs_Rd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    (void)decoder;
    instr.instr  = MOV;
    instr.op_src = EA;
    instr.op_dst = R;
    instr.op_reg = byte & 0b111;
    instr.format = I_Format::G;
}

void Dis_MOV_G_Rs_EAd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    (void)decoder;
    instr.instr  = MOV;
    instr.op_src = R;
    instr.op_dst = EA;
    instr.op_reg = byte & 0b111;
    instr.format = I_Format::G;
}

void Dis_MOV_G_imm8_EAd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    (void)byte;
    instr.instr   = MOV;
    instr.op_src  = imm;
    instr.op_dst  = EA;
    instr.op_data = decoder.ReadAdvance();
    instr.format  = I_Format::G;
}

void Dis_MOV_G_imm16_EAd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    (void)byte;
    instr.instr   = MOV;
    instr.op_src  = imm;
    instr.op_dst  = EA;
    instr.op_data = decoder.ReadU16();
    instr.format  = I_Format::G;
}

void Dis_LDM_SP_Reglist(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    (void)byte;
    // TODO revisit this representation
    instr.instr  = LDM;
    instr.op_src = imm;
    instr.imm    = decoder.ReadAdvance();
}

void Dis_STM_Reglist_SP(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    (void)byte;
    // TODO revisit this representation
    instr.instr  = STM;
    instr.op_src = imm;
    instr.imm    = decoder.ReadAdvance();
}

void Dis_ADD_G_EAs_Rd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    (void)decoder;
    instr.instr  = ADD;
    instr.op_src = EA;
    instr.op_dst = R;
    instr.op_reg = byte & 0b111;
    instr.format = I_Format::G;
}

void Dis_XCH_Rs_Rd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    (void)decoder;
    instr.instr  = XCH;
    instr.op_src = EA;
    instr.op_dst = R;
    instr.op_reg = byte & 0b111;
}

void Dis_SWAP_Rd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    (void)decoder;
    (void)byte;
    instr.instr  = SWAP;
    instr.op_dst = EA;
}

void Dis_MOVTPE_Rs_EAd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    Dis_NotImplemented(decoder, byte, instr);
}

void Dis_MOVTPE_EAS_Rd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    Dis_NotImplemented(decoder, byte, instr);
}

void Dis_ADD_Q_1_EAd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    (void)decoder;
    (void)byte;
    // TODO: maybe we should make this a special ADDQ since the imm is not actually an operand?
    instr.instr  = ADD;
    instr.op_src = imm;
    instr.op_dst = EA;
    instr.imm    = 1;
    instr.format = I_Format::Q;
}

void Dis_ADD_Q_2_EAd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    (void)decoder;
    (void)byte;
    instr.instr  = ADD;
    instr.op_src = imm;
    instr.op_dst = EA;
    instr.imm    = 2;
    instr.format = I_Format::Q;
}

void Dis_ADD_Q_M1_EAd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    (void)decoder;
    (void)byte;
    instr.instr  = ADD;
    instr.op_src = imm;
    instr.op_dst = EA;
    instr.imm    = static_cast<uint16_t>(-1);
    instr.format = I_Format::Q;
}

void Dis_ADD_Q_M2_EAd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    (void)decoder;
    (void)byte;
    instr.instr  = ADD;
    instr.op_src = imm;
    instr.op_dst = EA;
    instr.imm    = static_cast<uint16_t>(-2);
    instr.format = I_Format::Q;
}

void Dis_ADDS_EAs_Rd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    (void)decoder;
    instr.instr  = ADDS;
    instr.op_src = EA;
    instr.op_dst = R;
    instr.op_reg = byte & 0b111;
}

void Dis_ADDX_EAs_Rd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    (void)decoder;
    instr.instr  = ADDX;
    instr.op_src = EA;
    instr.op_dst = R;
    instr.op_reg = byte & 0b111;
}

void Dis_DADD_Rs_Rd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    (void)decoder;
    // TODO: no valid addressing mode???
    instr.instr  = DADD;
    instr.op_src = EA;
    instr.op_dst = R;
    instr.op_reg = byte & 0b111;
}

void Dis_SUB_EAs_Rd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    (void)decoder;
    instr.instr  = SUB;
    instr.op_src = EA;
    instr.op_dst = R;
    instr.op_reg = byte & 0b111;
}

void Dis_SUBS_EAs_Rd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    (void)decoder;
    instr.instr  = SUBS;
    instr.op_src = EA;
    instr.op_dst = R;
    instr.op_reg = byte & 0b111;
}

void Dis_SUBX_EAs_Rd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    (void)decoder;
    instr.instr  = SUBX;
    instr.op_src = EA;
    instr.op_dst = R;
    instr.op_reg = byte & 0b111;
}

void Dis_DSUB_Rs_Rd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    Dis_NotImplemented(decoder, byte, instr);
}

void Dis_MULXU_EAs_Rd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    (void)decoder;
    instr.instr  = MULXU;
    instr.op_src = EA;
    instr.op_dst = R;
    instr.op_reg = byte & 0b111;
}

void Dis_DIVXU_EAs_Rd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    (void)decoder;
    instr.instr  = DIVXU;
    instr.op_src = EA;
    instr.op_dst = R;
    instr.op_reg = byte & 0b111;
}

void Dis_CMP_G_EAs_Rd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    (void)decoder;
    instr.instr  = CMP;
    instr.op_src = EA;
    instr.op_dst = R;
    instr.op_reg = byte & 0b111;
    instr.format = I_Format::G;
}

void Dis_CMP_G_imm8_EAd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    (void)byte;
    instr.instr  = CMP;
    instr.op_src = imm;
    instr.op_dst = EA;
    instr.imm    = decoder.ReadAdvance();
    instr.format = I_Format::G;
}

void Dis_CMP_G_imm16_EAd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    (void)byte;
    instr.instr  = CMP;
    instr.op_src = imm;
    instr.op_dst = EA;
    instr.imm    = decoder.ReadU16();
    instr.format = I_Format::G;
}

void Dis_EXTS_Rd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    (void)decoder;
    (void)byte;
    instr.instr  = EXTS;
    instr.op_dst = EA;
}

void Dis_EXTU_Rd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    (void)decoder;
    instr.instr  = EXTU;
    instr.op_dst = EA;
    instr.op_reg = byte & 0b111;
}

void Dis_TST_EAd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    (void)decoder;
    (void)byte;
    instr.instr  = TST;
    instr.op_dst = EA;
}

void Dis_NEG_EAd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    (void)decoder;
    (void)byte;
    instr.instr  = NEG;
    instr.op_dst = EA;
}

void Dis_CLR_EAd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    (void)decoder;
    (void)byte;
    instr.instr  = CLR;
    instr.op_dst = EA;
}

void Dis_TAS_EAd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    (void)decoder;
    (void)byte;
    instr.instr  = TAS;
    instr.op_dst = EA;
}

// Shift instructions
void Dis_SHAL_EAd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    (void)decoder;
    (void)byte;
    instr.instr  = SHAL;
    instr.op_dst = EA;
}

void Dis_SHAR_EAd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    (void)decoder;
    (void)byte;
    instr.instr  = SHAR;
    instr.op_dst = EA;
}

void Dis_SHLL_EAd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    (void)decoder;
    (void)byte;
    instr.instr  = SHLL;
    instr.op_dst = EA;
}

void Dis_SHLR_EAd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    (void)decoder;
    (void)byte;
    instr.instr  = SHLR;
    instr.op_dst = EA;
}

void Dis_ROTL_EAd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    (void)decoder;
    (void)byte;
    instr.instr  = ROTL;
    instr.op_dst = EA;
}

void Dis_ROTR_EAd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    (void)decoder;
    (void)byte;
    instr.instr  = ROTR;
    instr.op_dst = EA;
}

void Dis_ROTXL_EAd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    (void)decoder;
    (void)byte;
    instr.instr  = ROTXL;
    instr.op_dst = EA;
}

void Dis_ROTXR_EAd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    (void)decoder;
    (void)byte;
    instr.instr  = ROTXR;
    instr.op_dst = EA;
}

// Logic operation instructions
void Dis_AND_EAs_Rd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    (void)decoder;
    instr.instr  = AND;
    instr.op_src = EA;
    instr.op_dst = R;
    instr.op_reg = byte & 0b111;
}

void Dis_OR_EAs_Rd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    (void)decoder;
    instr.instr  = OR;
    instr.op_src = EA;
    instr.op_dst = R;
    instr.op_reg = byte & 0b111;
}

void Dis_XOR_EAs_Rd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    (void)decoder;
    instr.instr  = XOR;
    instr.op_src = EA;
    instr.op_dst = R;
    instr.op_reg = byte & 0b111;
}

void Dis_NOT_EAd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    (void)decoder;
    (void)byte;
    instr.instr  = NOT;
    instr.op_dst = EA;
}

// Bit manipulation instructions
void Dis_BSET_imm4_EAd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    (void)decoder;
    instr.instr  = BSET;
    instr.op_src = imm;
    instr.op_dst = EA;
    instr.imm    = byte & 0b1111;
}

void Dis_BSET_Rs_EAd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    (void)decoder;
    instr.instr  = BSET;
    instr.op_src = R;
    instr.op_dst = EA;
    instr.op_reg = byte & 0b111;
}

void Dis_BCLR_imm4_EAd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    (void)decoder;
    instr.instr  = BCLR;
    instr.op_src = imm;
    instr.op_dst = EA;
    instr.imm    = byte & 0b1111;
}

void Dis_BCLR_Rs_EAd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    (void)decoder;
    instr.instr  = BCLR;
    instr.op_src = R;
    instr.op_dst = EA;
    instr.op_reg = byte & 0b111;
}

void Dis_BTST_imm4_EAd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    (void)decoder;
    instr.instr  = BTST;
    instr.op_src = imm;
    instr.op_dst = EA;
    instr.imm    = byte & 0b1111;
}

void Dis_BTST_Rs_EAd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    (void)decoder;
    instr.instr  = BTST;
    instr.op_src = R;
    instr.op_dst = EA;
    instr.op_reg = byte & 0b111;
}

void Dis_BNOT_imm4_EAd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    (void)decoder;
    instr.instr  = BNOT;
    instr.op_src = imm;
    instr.op_dst = EA;
    instr.imm    = byte & 0b1111;
}

void Dis_BNOT_Rs_EAd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    (void)decoder;
    instr.instr  = BNOT;
    instr.op_src = R;
    instr.op_dst = EA;
    instr.op_reg = byte & 0b111;
}

// System control instructions
void Dis_LDC_EAs_CR(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    (void)decoder;
    instr.instr  = LDC;
    instr.op_src = EA;
    instr.op_dst = CR;
    instr.op_cr  = byte & 0b111;
}

void Dis_STC_CR_EAd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    (void)decoder;
    instr.instr  = STC;
    instr.op_src = CR;
    instr.op_dst = EA;
    instr.op_cr  = byte & 0b111;
}

void Dis_ANDC_imm_CR(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    (void)decoder;
    instr.instr  = ANDC;
    instr.op_src = EA;
    instr.op_dst = CR;
    instr.op_cr  = byte & 0b111;
}

void Dis_ORC_imm_CR(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    (void)decoder;
    instr.instr  = ORC;
    instr.op_src = EA;
    instr.op_dst = CR;
    instr.op_cr  = byte & 0b111;
}

void Dis_XORC_imm_CR(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    (void)decoder;
    instr.instr  = XORC;
    instr.op_src = EA;
    instr.op_dst = CR;
    instr.op_cr  = byte & 0b111;
}

// Short instructions
void Dis_MOV_E_B_imm8_Rd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    instr.instr   = MOV;
    instr.format  = I_Format::E;
    instr.op_size = BYTE;
    instr.op_src  = imm;
    instr.imm     = decoder.ReadAdvance();
    instr.op_dst  = R;
    instr.op_reg  = byte & 0b111;
}

void Dis_MOV_I_W_imm16_Rd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    instr.instr   = MOV;
    instr.format  = I_Format::I;
    instr.op_size = WORD;
    instr.op_src  = imm;
    instr.imm     = decoder.ReadU16();
    instr.op_dst  = R;
    instr.op_reg  = byte & 0b111;
}

void Dis_MOV_F_B_d8_R6_Rd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    // TODO: fix op_src/op_dst
    instr.instr  = MOV;
    instr.op_src = R;
    instr.op_dst = R;
    instr.disp   = decoder.ReadAdvance();
    instr.format = I_Format::F;
    instr.op_reg = byte & 0b111;
}

void Dis_MOV_F_W_d8_R6_Rd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    // TODO: fix op_src/op_dst
    instr.instr  = MOV;
    instr.op_src = R;
    instr.op_dst = R;
    instr.disp   = decoder.ReadAdvance();
    instr.format = I_Format::F;
    instr.op_reg = byte & 0b111;
}

void Dis_MOV_F_B_Rs_d8_R6(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    // TODO: fix op_src/op_dst
    instr.instr  = MOV;
    instr.op_src = R;
    instr.op_dst = R;
    instr.disp   = decoder.ReadAdvance();
    instr.format = I_Format::F;
    instr.op_reg = byte & 0b111;
}

void Dis_MOV_F_W_Rs_d8_R6(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    // TODO: fix op_src/op_dst
    instr.instr  = MOV;
    instr.op_src = R;
    instr.op_dst = R;
    instr.disp   = decoder.ReadAdvance();
    instr.format = I_Format::F;
    instr.op_reg = byte & 0b111;
}

void Dis_CMP_E_B_imm8_Rd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    instr.instr  = CMP;
    instr.op_src = imm;
    instr.op_dst = R;
    instr.imm    = decoder.ReadAdvance();
    instr.format = I_Format::E;
    instr.op_reg = byte & 0b111;
}

void Dis_CMP_I_W_imm16_Rd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    instr.instr  = CMP;
    instr.op_src = imm;
    instr.op_dst = R;
    instr.imm    = decoder.ReadU16();
    instr.format = I_Format::I;
    instr.op_reg = byte & 0b111;
}

void Dis_BRA(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    (void)byte;
    instr.instr  = BRA;
    instr.imm    = decoder.ReadAdvance();
    instr.op_src = imm;
}

void Dis_BRN(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    (void)byte;
    instr.instr  = BRN;
    instr.imm    = decoder.ReadAdvance();
    instr.op_src = imm;
}

void Dis_BHI(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    (void)byte;
    instr.instr  = BHI;
    instr.imm    = decoder.ReadAdvance();
    instr.op_src = imm;
}

void Dis_BLS(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    (void)byte;
    instr.instr  = BLS;
    instr.imm    = decoder.ReadAdvance();
    instr.op_src = imm;
}

void Dis_BCC(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    (void)byte;
    instr.instr  = BCC;
    instr.imm    = decoder.ReadAdvance();
    instr.op_src = imm;
}

void Dis_BCS(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    (void)byte;
    instr.instr  = BCS;
    instr.imm    = decoder.ReadAdvance();
    instr.op_src = imm;
}

void Dis_BNE(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    (void)byte;
    instr.instr  = BNE;
    instr.imm    = decoder.ReadAdvance();
    instr.op_src = imm;
}

void Dis_BEQ(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    (void)byte;
    instr.instr  = BEQ;
    instr.imm    = decoder.ReadAdvance();
    instr.op_src = imm;
}

void Dis_BVC(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    (void)byte;
    instr.instr  = BVC;
    instr.imm    = decoder.ReadAdvance();
    instr.op_src = imm;
}

void Dis_BVS(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    (void)byte;
    instr.instr  = BVS;
    instr.imm    = decoder.ReadAdvance();
    instr.op_src = imm;
}

void Dis_BPL(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    (void)byte;
    instr.instr  = BPL;
    instr.imm    = decoder.ReadAdvance();
    instr.op_src = imm;
}

void Dis_BMI(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    (void)byte;
    instr.instr  = BMI;
    instr.imm    = decoder.ReadAdvance();
    instr.op_src = imm;
}

void Dis_BGE(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    (void)byte;
    instr.instr  = BGE;
    instr.imm    = decoder.ReadAdvance();
    instr.op_src = imm;
}

void Dis_BLT(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    (void)byte;
    instr.instr  = BLT;
    instr.imm    = decoder.ReadAdvance();
    instr.op_src = imm;
}

void Dis_BGT(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    (void)byte;
    instr.instr  = BGT;
    instr.imm    = decoder.ReadAdvance();
    instr.op_src = imm;
}

void Dis_BLE(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    (void)byte;
    instr.instr  = BLE;
    instr.imm    = decoder.ReadAdvance();
    instr.op_src = imm;
}

void Dis_BRA16(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    (void)byte;
    instr.instr  = BRA;
    instr.imm    = decoder.ReadU16();
    instr.op_src = imm;
}

void Dis_BRN16(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    (void)byte;
    instr.instr  = BRN;
    instr.imm    = decoder.ReadU16();
    instr.op_src = imm;
}

void Dis_BHI16(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    (void)byte;
    instr.instr  = BHI;
    instr.imm    = decoder.ReadU16();
    instr.op_src = imm;
}

void Dis_BLS16(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    (void)byte;
    instr.instr  = BLS;
    instr.imm    = decoder.ReadU16();
    instr.op_src = imm;
}

void Dis_BCC16(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    (void)byte;
    instr.instr  = BCC;
    instr.imm    = decoder.ReadU16();
    instr.op_src = imm;
}

void Dis_BCS16(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    (void)byte;
    instr.instr  = BCS;
    instr.imm    = decoder.ReadU16();
    instr.op_src = imm;
}

void Dis_BNE16(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    (void)byte;
    instr.instr  = BNE;
    instr.imm    = decoder.ReadU16();
    instr.op_src = imm;
}

void Dis_BEQ16(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    (void)byte;
    instr.instr  = BEQ;
    instr.imm    = decoder.ReadU16();
    instr.op_src = imm;
}

void Dis_BVC16(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    (void)byte;
    instr.instr  = BVC;
    instr.imm    = decoder.ReadU16();
    instr.op_src = imm;
}

void Dis_BVS16(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    (void)byte;
    instr.instr  = BVS;
    instr.imm    = decoder.ReadU16();
    instr.op_src = imm;
}

void Dis_BPL16(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    (void)byte;
    instr.instr  = BPL;
    instr.imm    = decoder.ReadU16();
    instr.op_src = imm;
}

void Dis_BMI16(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    (void)byte;
    instr.instr  = BMI;
    instr.imm    = decoder.ReadU16();
    instr.op_src = imm;
}

void Dis_BGE16(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    (void)byte;
    instr.instr  = BGE;
    instr.imm    = decoder.ReadU16();
    instr.op_src = imm;
}

void Dis_BLT16(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    (void)byte;
    instr.instr  = BLT;
    instr.imm    = decoder.ReadU16();
    instr.op_src = imm;
}

void Dis_BGT16(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    (void)byte;
    instr.instr  = BGT;
    instr.imm    = decoder.ReadU16();
    instr.op_src = imm;
}

void Dis_BLE16(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    (void)byte;
    instr.instr  = BLE;
    instr.imm    = decoder.ReadU16();
    instr.op_src = imm;
}

void Dis_GenericJump(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    (void)byte;

    const uint8_t opcode = decoder.ReadAdvance();

    switch (opcode & 0b11111000)
    {
    // JMP @Rn
    case 0b11010000:
        instr.instr  = JMP;
        instr.op_src = R;
        instr.op_reg = opcode & 0b111;
        break;

    // JMP @(d:8,Rn)
    case 0b11100000:
        instr.instr  = JMP;
        instr.op_src = R;
        instr.op_reg = opcode & 0b111;
        instr.disp   = (int8_t)decoder.ReadAdvance();
        break;

    // JMP @(d:16,Rn)
    case 0b11110000:
        instr.instr  = JMP;
        instr.op_src = R;
        instr.op_reg = opcode & 0b111;
        instr.disp   = (int16_t)decoder.ReadU16();
        break;

    // JSR @Rn
    case 0b11011000:
        instr.instr  = JSR;
        instr.op_src = R;
        instr.op_reg = opcode & 0b111;
        break;

    // JSR @(d:8,Rn)
    case 0b11101000:
        instr.instr  = JSR;
        instr.op_src = R;
        instr.op_reg = opcode & 0b111;
        instr.disp   = (int8_t)decoder.ReadAdvance();
        break;

    // JSR @(d:16,Rn)
    case 0b11111000:
        instr.instr  = JSR;
        instr.op_src = R;
        instr.op_reg = opcode & 0b111;
        instr.disp   = (int16_t)decoder.ReadU16();
        break;

    // PJMP @Rn
    case 0b11000000:
        instr.instr  = PJMP;
        instr.op_src = R;
        instr.op_reg = opcode & 0b111;
        break;

    // PJSR @Rn
    case 0b11001000:
        instr.instr  = PJSR;
        instr.op_src = R;
        instr.op_reg = opcode & 0b111;
        break;

    default:
        // PRTS
        if (opcode == 0b00011001)
        {
            instr.instr = PRTS;
        }
        // PRTD #xx:8
        else if (opcode == 0b00010100)
        {
            instr.instr  = PRTD;
            instr.op_src = imm;
            instr.imm    = decoder.ReadAdvance();
        }
        // PRTD #xx:16
        else if (opcode == 0b00011100)
        {
            instr.instr  = PRTD;
            instr.op_src = imm;
            instr.imm    = decoder.ReadU16();
        }
        else
        {
            I_DecoderError err;
            err.code     = I_DecoderErrorCode::InvalidGenericJump;
            err.position = decoder.GetPosition() - 1;
            err.message  = "Unrecognized generic jump";
            decoder.SetError(err);
        }
    }
}

void Dis_JMP_aa16(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    (void)byte;
    instr.instr  = JMP;
    instr.op_src = imm;
    instr.imm    = decoder.ReadU16();
}

void Dis_BSR_d8(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    (void)byte;
    instr.instr  = BSR;
    instr.imm    = decoder.ReadAdvance();
    instr.op_src = imm;
}

void Dis_BSR_d16(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    (void)byte;
    instr.instr  = BSR;
    instr.imm    = decoder.ReadU16();
    instr.op_src = imm;
}

void Dis_JSR_aa16(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    (void)byte;
    instr.instr  = JSR;
    instr.imm    = decoder.ReadU16();
    instr.op_src = imm;
}

void Dis_RTS(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    (void)decoder;
    (void)byte;
    instr.instr = RTS;
}

void Dis_RTD_imm8(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    (void)byte;
    instr.instr  = RTD;
    instr.op_src = imm;
    instr.imm    = decoder.ReadAdvance();
}

void Dis_RTD_imm16(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    (void)byte;
    instr.instr  = RTD;
    instr.op_src = imm;
    instr.imm    = decoder.ReadU16();
}

void Dis_NOP(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    (void)decoder;
    (void)byte;
    instr.instr = NOP;
}

void Dis_NotImplemented(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    (void)decoder;
    (void)byte;
    (void)instr;
    throw "Instruction not implemented";
}

void Dis_InvalidInstruction(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    (void)decoder;
    (void)byte;
    (void)instr;
    throw "Instruction invalid";
}

void Dis_SCB_F(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    (void)byte;

    const uint8_t reg_byte = decoder.ReadAdvance();

    if ((reg_byte & 0b11111000) != 0b10111000)
    {
        decoder.SetError({
            .code     = I_DecoderErrorCode::InvalidInstructionFormat,
            .position = decoder.GetPosition(),
            .message  = "SCB/F: second byte is not 10111rrr",
        });
        return;
    }

    uint8_t disp = decoder.ReadAdvance();
    instr.instr  = SCB_F;
    instr.op_src = R;
    instr.op_reg = reg_byte & 0b111;
    instr.op_dst = imm;
    instr.imm    = disp;
}

void Dis_SCB_NE(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    (void)byte;

    const uint8_t reg_byte = decoder.ReadAdvance();

    if ((reg_byte & 0b11111000) != 0b10111000)
    {
        decoder.SetError({
            .code     = I_DecoderErrorCode::InvalidInstructionFormat,
            .position = decoder.GetPosition(),
            .message  = "SCB/NE: second byte is not 10111rrr",
        });
        return;
    }

    uint8_t disp = decoder.ReadAdvance();
    instr.instr  = SCB_NE;
    instr.op_src = R;
    instr.op_reg = reg_byte & 0b111;
    instr.op_dst = imm;
    instr.imm    = disp;
}

void Dis_SCB_EQ(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    (void)byte;

    const uint8_t reg_byte = decoder.ReadAdvance();

    if ((reg_byte & 0b11111000) != 0b10111000)
    {
        decoder.SetError({
            .code     = I_DecoderErrorCode::InvalidInstructionFormat,
            .position = decoder.GetPosition(),
            .message  = "SCB/EQ: second byte is not 10111rrr",
        });
        return;
    }

    uint8_t disp = decoder.ReadAdvance();
    instr.instr  = SCB_EQ;
    instr.op_src = R;
    instr.op_reg = reg_byte & 0b111;
    instr.op_dst = imm;
    instr.imm    = disp;
}

void Dis_MOV_L_B_aa8_Rd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    const uint8_t addr = decoder.ReadAdvance();

    instr.instr   = MOV;
    instr.format  = I_Format::L;
    instr.op_size = BYTE;
    instr.op_src  = imm;
    instr.imm     = addr;
    instr.op_dst  = R;
    instr.op_reg  = byte & 0b111;
}

void Dis_MOV_L_W_aa8_Rd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    const uint8_t addr = decoder.ReadAdvance();

    instr.instr   = MOV;
    instr.format  = I_Format::L;
    instr.op_size = WORD;
    instr.op_src  = imm;
    instr.imm     = addr;
    instr.op_dst  = R;
    instr.op_reg  = byte & 0b111;
}

void Dis_MOV_S_B_Rs_aa8(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    const uint8_t addr = decoder.ReadAdvance();

    instr.instr   = MOV;
    instr.format  = I_Format::S;
    instr.op_size = BYTE;
    instr.op_src  = R;
    instr.op_reg  = byte & 0b111;
    instr.op_dst  = imm;
    instr.imm     = addr;
}

void Dis_MOV_S_W_Rs_aa8(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    const uint8_t addr = decoder.ReadAdvance();

    instr.instr   = MOV;
    instr.format  = I_Format::S;
    instr.op_size = WORD;
    instr.op_src  = R;
    instr.op_reg  = byte & 0b111;
    instr.op_dst  = imm;
    instr.imm     = addr;
}

void Dis_MOV_E_imm8_Rd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    const uint8_t data = decoder.ReadAdvance();

    instr.instr   = MOV;
    instr.format  = I_Format::E;
    instr.op_size = BYTE;
    instr.op_src  = imm;
    instr.imm     = data;
    instr.op_dst  = R;
    instr.op_reg  = byte & 0b111;
}

void Dis_TRAPA_imm4(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr)
{
    (void)byte;

    const uint8_t vec_byte = decoder.ReadAdvance();

    if ((vec_byte & 0b11110000) != 0b00010000)
    {
        decoder.SetError({
            .code     = I_DecoderErrorCode::InvalidInstructionFormat,
            .position = decoder.GetPosition(),
            .message  = "TRAPA: second byte is not 0001#VEC",
        });
        return;
    }

    instr.instr  = TRAPA;
    instr.op_src = imm;
    instr.imm    = vec_byte & 0b1111;
}
