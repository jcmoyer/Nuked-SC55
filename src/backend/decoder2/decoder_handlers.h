#pragma once

#include <cstdint>

class I_Decoder;
struct I_DecodedInstruction;

// Data transfer instructions
void Dis_MOV_G_EAs_Rd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);
void Dis_MOV_G_Rs_EAd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);
void Dis_MOV_G_imm8_EAd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);
void Dis_MOV_G_imm16_EAd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);
void Dis_LDM_SP_Reglist(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);
void Dis_STM_Reglist_SP(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);
void Dis_XCH_Rs_Rd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);
void Dis_SWAP_Rd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);
// TODO: both forms of MOVTPE have same encoding?
void Dis_MOVTPE_Rs_EAd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);
void Dis_MOVTPE_EAS_Rd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);

// Arithmetic operation instructions
void Dis_ADD_G_EAs_Rd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);
void Dis_ADD_Q_1_EAd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);
void Dis_ADD_Q_2_EAd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);
void Dis_ADD_Q_M1_EAd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);
void Dis_ADD_Q_M2_EAd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);
void Dis_ADDS_EAs_Rd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);
void Dis_ADDX_EAs_Rd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);
void Dis_DADD_Rs_Rd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);
void Dis_SUB_EAs_Rd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);
void Dis_SUBS_EAs_Rd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);
void Dis_SUBX_EAs_Rd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);
void Dis_DSUB_Rs_Rd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);
void Dis_MULXU_EAs_Rd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);
void Dis_DIVXU_EAs_Rd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);
void Dis_CMP_G_EAs_Rd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);
void Dis_CMP_G_imm8_EAd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);
void Dis_CMP_G_imm16_EAd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);
void Dis_EXTS_Rd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);
void Dis_EXTU_Rd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);
void Dis_TST_EAd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);
void Dis_NEG_EAd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);
void Dis_CLR_EAd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);
void Dis_TAS_EAd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);

// Shift instructions
void Dis_SHAL_EAd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);
void Dis_SHAR_EAd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);
void Dis_SHLL_EAd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);
void Dis_SHLR_EAd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);
void Dis_ROTL_EAd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);
void Dis_ROTR_EAd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);
void Dis_ROTXL_EAd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);
void Dis_ROTXR_EAd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);

// Logic operation instructions
void Dis_AND_EAs_Rd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);
void Dis_OR_EAs_Rd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);
void Dis_XOR_EAs_Rd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);
void Dis_NOT_EAd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);

// Bit manipulation instructions
void Dis_BSET_imm4_EAd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);
void Dis_BSET_Rs_EAd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);
void Dis_BCLR_imm4_EAd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);
void Dis_BCLR_Rs_EAd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);
void Dis_BTST_imm4_EAd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);
void Dis_BTST_Rs_EAd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);
void Dis_BNOT_imm4_EAd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);
void Dis_BNOT_Rs_EAd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);

// System control instructions
void Dis_LDC_EAs_CR(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);
void Dis_STC_CR_EAd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);
void Dis_ANDC_imm_CR(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);
void Dis_ORC_imm_CR(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);
void Dis_XORC_imm_CR(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);

// Short instructions
void Dis_MOV_E_B_imm8_Rd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);
void Dis_MOV_I_W_imm16_Rd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);
// TODO: MOV:F maybe unused
void Dis_MOV_F_B_d8_R6_Rd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);
void Dis_MOV_F_W_d8_R6_Rd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);
void Dis_MOV_F_B_Rs_d8_R6(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);
void Dis_MOV_F_W_Rs_d8_R6(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);
void Dis_CMP_E_B_imm8_Rd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);
void Dis_CMP_I_W_imm16_Rd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);

void Dis_BRA(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);
void Dis_BRN(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);
void Dis_BHI(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);
void Dis_BLS(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);
void Dis_BCC(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);
void Dis_BCS(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);
void Dis_BNE(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);
void Dis_BEQ(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);
void Dis_BVC(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);
void Dis_BVS(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);
void Dis_BPL(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);
void Dis_BMI(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);
void Dis_BGE(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);
void Dis_BLT(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);
void Dis_BGT(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);
void Dis_BLE(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);

void Dis_BRA16(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);
void Dis_BRN16(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);
void Dis_BHI16(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);
void Dis_BLS16(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);
void Dis_BCC16(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);
void Dis_BCS16(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);
void Dis_BNE16(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);
void Dis_BEQ16(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);
void Dis_BVC16(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);
void Dis_BVS16(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);
void Dis_BPL16(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);
void Dis_BMI16(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);
void Dis_BGE16(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);
void Dis_BLT16(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);
void Dis_BGT16(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);
void Dis_BLE16(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);

// Handles all jumps starting with 00010001
void Dis_GenericJump(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);
void Dis_JMP_aa16(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);
void Dis_BSR_d8(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);
void Dis_BSR_d16(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);
void Dis_JSR_aa16(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);
void Dis_RTS(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);
void Dis_RTD_imm8(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);
void Dis_RTD_imm16(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);

void Dis_NOP(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);

void Dis_NotImplemented(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);
void Dis_InvalidInstruction(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);

void Dis_SCB_F(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);
void Dis_SCB_NE(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);
void Dis_SCB_EQ(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);

void Dis_MOV_L_B_aa8_Rd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);
void Dis_MOV_L_W_aa8_Rd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);
void Dis_MOV_S_B_Rs_aa8(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);
void Dis_MOV_S_W_Rs_aa8(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);
void Dis_MOV_E_imm8_Rd(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);

void Dis_TRAPA_imm4(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);

void Dis_PJMP_aa24(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);
void Dis_SLEEP(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);
