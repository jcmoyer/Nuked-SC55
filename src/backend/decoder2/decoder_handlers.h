#pragma once

#include <cstdint>

namespace decoder2
{

class DisassembleDecoder;
struct DisassembledInstruction;

// Data transfer instructions
void Dis_MOV_G_EAs_Rd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);
void Dis_MOV_G_Rs_EAd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);
void Dis_MOV_G_imm8_EAd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);
void Dis_MOV_G_imm16_EAd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);
void Dis_LDM_SP_Reglist(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);
void Dis_STM_Reglist_SP(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);
void Dis_XCH_Rs_Rd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);
void Dis_SWAP_Rd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);
// TODO: both forms of MOVTPE have same encoding?
void Dis_MOVTPE_Rs_EAd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);
void Dis_MOVTPE_EAS_Rd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);

// Arithmetic operation instructions
void Dis_ADD_G_EAs_Rd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);
void Dis_ADD_Q_1_EAd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);
void Dis_ADD_Q_2_EAd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);
void Dis_ADD_Q_M1_EAd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);
void Dis_ADD_Q_M2_EAd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);
void Dis_ADDS_EAs_Rd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);
void Dis_ADDX_EAs_Rd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);
void Dis_DADD_Rs_Rd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);
void Dis_SUB_EAs_Rd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);
void Dis_SUBS_EAs_Rd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);
void Dis_SUBX_EAs_Rd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);
void Dis_DSUB_Rs_Rd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);
void Dis_MULXU_EAs_Rd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);
void Dis_DIVXU_EAs_Rd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);
void Dis_CMP_G_EAs_Rd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);
void Dis_CMP_G_imm8_EAd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);
void Dis_CMP_G_imm16_EAd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);
void Dis_EXTS_Rd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);
void Dis_EXTU_Rd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);
void Dis_TST_EAd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);
void Dis_NEG_EAd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);
void Dis_CLR_EAd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);
void Dis_TAS_EAd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);

// Shift instructions
void Dis_SHAL_EAd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);
void Dis_SHAR_EAd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);
void Dis_SHLL_EAd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);
void Dis_SHLR_EAd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);
void Dis_ROTL_EAd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);
void Dis_ROTR_EAd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);
void Dis_ROTXL_EAd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);
void Dis_ROTXR_EAd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);

// Logic operation instructions
void Dis_AND_EAs_Rd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);
void Dis_OR_EAs_Rd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);
void Dis_XOR_EAs_Rd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);
void Dis_NOT_EAd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);

// Bit manipulation instructions
void Dis_BSET_imm4_EAd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);
void Dis_BSET_Rs_EAd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);
void Dis_BCLR_imm4_EAd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);
void Dis_BCLR_Rs_EAd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);
void Dis_BTST_imm4_EAd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);
void Dis_BTST_Rs_EAd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);
void Dis_BNOT_imm4_EAd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);
void Dis_BNOT_Rs_EAd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);

// System control instructions
void Dis_LDC_EAs_CR(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);
void Dis_STC_CR_EAd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);
void Dis_ANDC_imm_CR(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);
void Dis_ORC_imm_CR(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);
void Dis_XORC_imm_CR(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);

// Short instructions
void Dis_MOV_E_B_imm8_Rd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);
void Dis_MOV_I_W_imm16_Rd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);
// TODO: MOV:F maybe unused
void Dis_MOV_F_B_d8_R6_Rd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);
void Dis_MOV_F_W_d8_R6_Rd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);
void Dis_MOV_F_B_Rs_d8_R6(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);
void Dis_MOV_F_W_Rs_d8_R6(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);
void Dis_CMP_E_B_imm8_Rd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);
void Dis_CMP_I_W_imm16_Rd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);

void Dis_BRA(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);
void Dis_BRN(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);
void Dis_BHI(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);
void Dis_BLS(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);
void Dis_BCC(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);
void Dis_BCS(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);
void Dis_BNE(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);
void Dis_BEQ(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);
void Dis_BVC(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);
void Dis_BVS(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);
void Dis_BPL(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);
void Dis_BMI(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);
void Dis_BGE(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);
void Dis_BLT(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);
void Dis_BGT(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);
void Dis_BLE(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);

void Dis_BRA16(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);
void Dis_BRN16(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);
void Dis_BHI16(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);
void Dis_BLS16(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);
void Dis_BCC16(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);
void Dis_BCS16(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);
void Dis_BNE16(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);
void Dis_BEQ16(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);
void Dis_BVC16(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);
void Dis_BVS16(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);
void Dis_BPL16(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);
void Dis_BMI16(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);
void Dis_BGE16(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);
void Dis_BLT16(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);
void Dis_BGT16(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);
void Dis_BLE16(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);

// Handles all jumps starting with 00010001
void Dis_GenericJump(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);
void Dis_JMP_aa16(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);
void Dis_BSR_d8(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);
void Dis_BSR_d16(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);
void Dis_JSR_aa16(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);
void Dis_RTS(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);
void Dis_RTD_imm8(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);
void Dis_RTD_imm16(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);

void Dis_NOP(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);

void Dis_NotImplemented(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);
void Dis_InvalidInstruction(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);

void Dis_SCB_F(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);
void Dis_SCB_NE(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);
void Dis_SCB_EQ(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);

void Dis_MOV_L_B_aa8_Rd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);
void Dis_MOV_L_W_aa8_Rd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);
void Dis_MOV_S_B_Rs_aa8(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);
void Dis_MOV_S_W_Rs_aa8(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);
void Dis_MOV_E_imm8_Rd(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);

void Dis_TRAPA_imm4(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);

void Dis_PJMP_aa24(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);
void Dis_PJSR_aa24(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);
void Dis_RTE(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);
void Dis_SLEEP(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);

} // namespace decoder2
