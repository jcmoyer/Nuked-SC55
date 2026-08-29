#include "decoder2/disassemblers.h"
//=============================================================================

#include <array>

#include "decoder2/disassemble.h"
#include "decoder2/disassemble_handlers.h"

namespace decoder2
{

static void Dis_Rn(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    instr.mode       = AddressMode::Rn;
    instr.op_size    = (byte & 0b00001000) ? OptionalSize::Word : OptionalSize::Byte;
    instr.is_general = true;
    instr.ea_reg     = byte & 0b111;

    const uint8_t opcode = decoder.ReadAdvance();

    if (Disassembler dis = GetDecoderRn(opcode); dis)
    {
        dis(decoder, opcode, instr);
    }
    else
    {
        decoder.SetError({
            .code     = DisassembleErrorCode::InvalidInstructionFormat,
            .position = decoder.GetPosition() - 1,
            .message  = "No Rn handler",
        });
    }
}

static void Dis_APreDecRn(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    instr.op_size    = (byte & 0b00001000) ? OptionalSize::Word : OptionalSize::Byte;
    instr.mode       = AddressMode::APreDecRn;
    instr.is_general = true;
    instr.ea_reg     = byte & 0b111;

    const uint8_t opcode = decoder.ReadAdvance();

    if (Disassembler dis = GetDecoderGeneric(opcode); dis)
    {
        dis(decoder, opcode, instr);
    }
    else
    {
        decoder.SetError({
            .code     = DisassembleErrorCode::InvalidInstructionFormat,
            .position = decoder.GetPosition() - 1,
            .message  = "No @-Rn handler",
        });
    }
}

static void Dis_APostIncRn(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    instr.op_size    = (byte & 0b00001000) ? OptionalSize::Word : OptionalSize::Byte;
    instr.mode       = AddressMode::APostIncRn;
    instr.is_general = true;
    instr.ea_reg     = byte & 0b111;

    const uint8_t opcode = decoder.ReadAdvance();

    if (Disassembler dis = GetDecoderGeneric(opcode); dis)
    {
        dis(decoder, opcode, instr);
    }
    else
    {
        decoder.SetError({
            .code     = DisassembleErrorCode::InvalidInstructionFormat,
            .position = decoder.GetPosition() - 1,
            .message  = "No @Rn+ handler",
        });
    }
}

static void Dis_ARn(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    instr.mode       = AddressMode::ARn;
    instr.op_size    = (byte & 0b00001000) ? OptionalSize::Word : OptionalSize::Byte;
    instr.is_general = true;
    instr.ea_reg     = byte & 0b111;

    const uint8_t opcode = decoder.ReadAdvance();

    if (Disassembler dis = GetDecoderGeneric(opcode); dis)
    {
        dis(decoder, opcode, instr);
    }
    else
    {
        decoder.SetError({
            .code     = DisassembleErrorCode::InvalidInstructionFormat,
            .position = decoder.GetPosition() - 1,
            .message  = "No @Rn handler",
        });
    }
}

static void Dis_Ad8_Rn(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    instr.op_size    = (byte & 0b00001000) ? OptionalSize::Word : OptionalSize::Byte;
    instr.mode       = AddressMode::Ad8_Rn;
    instr.ea_disp    = (int8_t)decoder.ReadAdvance();
    instr.is_general = true;
    instr.ea_reg     = byte & 0b111;

    const uint8_t opcode = decoder.ReadAdvance();

    if (Disassembler dis = GetDecoderGeneric(opcode); dis)
    {
        dis(decoder, opcode, instr);
    }
    else
    {
        decoder.SetError({
            .code     = DisassembleErrorCode::InvalidInstructionFormat,
            .position = decoder.GetPosition() - 1,
            .message  = "No @(d:8, Rn) handler",
        });
    }
}

static void Dis_Ad16_Rn(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    instr.op_size    = (byte & 0b00001000) ? OptionalSize::Word : OptionalSize::Byte;
    instr.mode       = AddressMode::Ad16_Rn;
    instr.ea_disp    = (int16_t)decoder.ReadU16();
    instr.is_general = true;
    instr.ea_reg     = byte & 0b111;

    const uint8_t opcode = decoder.ReadAdvance();

    if (Disassembler dis = GetDecoderGeneric(opcode); dis)
    {
        dis(decoder, opcode, instr);
    }
    else
    {
        decoder.SetError({
            .code     = DisassembleErrorCode::InvalidInstructionFormat,
            .position = decoder.GetPosition() - 1,
            .message  = "No @(d:16, Rn) handler",
        });
    }
}

static void Dis_imm8(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    (void)byte;

    instr.op_size    = OptionalSize::Byte;
    instr.mode       = AddressMode::imm8;
    instr.ea_imm     = decoder.ReadAdvance();
    instr.is_general = true;

    const uint8_t opcode = decoder.ReadAdvance();

    if (Disassembler dis = GetDecoderimm8(opcode); dis)
    {
        dis(decoder, opcode, instr);
    }
    else
    {
        decoder.SetError({
            .code     = DisassembleErrorCode::InvalidInstructionFormat,
            .position = decoder.GetPosition() - 1,
            .message  = "No #xx:8 handler",
        });
    }
}

static void Dis_imm16(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    (void)byte;

    instr.op_size    = OptionalSize::Word;
    instr.mode       = AddressMode::imm16;
    instr.ea_imm     = decoder.ReadU16();
    instr.is_general = true;

    const uint8_t opcode = decoder.ReadAdvance();

    if (Disassembler dis = GetDecoderimm16(opcode); dis)
    {
        dis(decoder, opcode, instr);
    }
    else
    {
        decoder.SetError({
            .code     = DisassembleErrorCode::InvalidInstructionFormat,
            .position = decoder.GetPosition() - 1,
            .message  = "No #xx:16 handler",
        });
    }
}

static void Dis_Aaa8(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    instr.op_size    = (byte & 0b00001000) ? OptionalSize::Word : OptionalSize::Byte;
    instr.mode       = AddressMode::Aaa8;
    instr.ea_addr    = decoder.ReadAdvance();
    instr.is_general = true;

    const uint8_t opcode = decoder.ReadAdvance();

    if (Disassembler dis = GetDecoderGeneric(opcode); dis)
    {
        dis(decoder, opcode, instr);
    }
    else
    {
        decoder.SetError({
            .code     = DisassembleErrorCode::InvalidInstructionFormat,
            .position = decoder.GetPosition() - 1,
            .message  = "No @aa:8 handler",
        });
    }
}

static void Dis_Aaa16(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr)
{
    instr.op_size    = (byte & 0b00001000) ? OptionalSize::Word : OptionalSize::Byte;
    instr.mode       = AddressMode::Aaa16;
    instr.ea_addr    = decoder.ReadU16();
    instr.is_general = true;

    const uint8_t opcode = decoder.ReadAdvance();

    if (Disassembler dis = GetDecoderGeneric(opcode); dis)
    {
        dis(decoder, opcode, instr);
    }
    else
    {
        decoder.SetError({
            .code     = DisassembleErrorCode::InvalidInstructionFormat,
            .position = decoder.GetPosition() - 1,
            .message  = "No @aa:16 handler",
        });
    }
}

constexpr std::array<Disassembler, 256> MakeDecodeTable()
{
    std::array<Disassembler, 256> t{};
    t[0b00000000] = Dis_NOP;
    t[0b00000001] = Dis_SCB_F;
    t[0b00000010] = Dis_LDM_SP_Reglist;
    t[0b00000011] = Dis_PJSR_aa24;
    t[0b00000100] = Dis_imm8;
    t[0b00000101] = Dis_Aaa8;
    t[0b00000110] = Dis_SCB_NE;
    t[0b00000111] = Dis_SCB_EQ;
    t[0b00001000] = Dis_TRAPA_imm4;
    t[0b00001001] = nullptr;
    t[0b00001010] = Dis_RTE;
    t[0b00001011] = nullptr;
    t[0b00001100] = Dis_imm16;
    t[0b00001101] = Dis_Aaa8;
    t[0b00001110] = Dis_BSR_d8;
    t[0b00001111] = nullptr;
    t[0b00010000] = Dis_JMP_aa16;
    t[0b00010001] = Dis_GenericJump;
    t[0b00010010] = Dis_STM_Reglist_SP;
    t[0b00010011] = Dis_PJMP_aa24;
    t[0b00010100] = Dis_RTD_imm8;
    t[0b00010101] = Dis_Aaa16;
    t[0b00010110] = nullptr;
    t[0b00010111] = nullptr;
    t[0b00011000] = Dis_JSR_aa16;
    t[0b00011001] = Dis_RTS;
    t[0b00011010] = Dis_SLEEP;
    t[0b00011011] = nullptr;
    t[0b00011100] = Dis_RTD_imm16;
    t[0b00011101] = Dis_Aaa16;
    t[0b00011110] = Dis_BSR_d16;
    t[0b00011111] = nullptr;
    t[0b00100000] = Dis_BRA;
    t[0b00100001] = Dis_BRN;
    t[0b00100010] = Dis_BHI;
    t[0b00100011] = Dis_BLS;
    t[0b00100100] = Dis_BCC;
    t[0b00100101] = Dis_BCS;
    t[0b00100110] = Dis_BNE;
    t[0b00100111] = Dis_BEQ;
    t[0b00101000] = Dis_BVC;
    t[0b00101001] = Dis_BVS;
    t[0b00101010] = Dis_BPL;
    t[0b00101011] = Dis_BMI;
    t[0b00101100] = Dis_BGE;
    t[0b00101101] = Dis_BLT;
    t[0b00101110] = Dis_BGT;
    t[0b00101111] = Dis_BLE;
    t[0b00110000] = Dis_BRA16;
    t[0b00110001] = Dis_BRN16;
    t[0b00110010] = Dis_BHI16;
    t[0b00110011] = Dis_BLS16;
    t[0b00110100] = Dis_BCC16;
    t[0b00110101] = Dis_BCS16;
    t[0b00110110] = Dis_BNE16;
    t[0b00110111] = Dis_BEQ16;
    t[0b00111000] = Dis_BVC16;
    t[0b00111001] = Dis_BVS16;
    t[0b00111010] = Dis_BPL16;
    t[0b00111011] = Dis_BMI16;
    t[0b00111100] = Dis_BGE16;
    t[0b00111101] = Dis_BLT16;
    t[0b00111110] = Dis_BGT16;
    t[0b00111111] = Dis_BLE16;
    t[0b01000000] = Dis_CMP_E_B_imm8_Rd;
    t[0b01000001] = Dis_CMP_E_B_imm8_Rd;
    t[0b01000010] = Dis_CMP_E_B_imm8_Rd;
    t[0b01000011] = Dis_CMP_E_B_imm8_Rd;
    t[0b01000100] = Dis_CMP_E_B_imm8_Rd;
    t[0b01000101] = Dis_CMP_E_B_imm8_Rd;
    t[0b01000110] = Dis_CMP_E_B_imm8_Rd;
    t[0b01000111] = Dis_CMP_E_B_imm8_Rd;
    t[0b01001000] = Dis_CMP_I_W_imm16_Rd;
    t[0b01001001] = Dis_CMP_I_W_imm16_Rd;
    t[0b01001010] = Dis_CMP_I_W_imm16_Rd;
    t[0b01001011] = Dis_CMP_I_W_imm16_Rd;
    t[0b01001100] = Dis_CMP_I_W_imm16_Rd;
    t[0b01001101] = Dis_CMP_I_W_imm16_Rd;
    t[0b01001110] = Dis_CMP_I_W_imm16_Rd;
    t[0b01001111] = Dis_CMP_I_W_imm16_Rd;
    t[0b01010000] = Dis_MOV_E_imm8_Rd;
    t[0b01010001] = Dis_MOV_E_imm8_Rd;
    t[0b01010010] = Dis_MOV_E_imm8_Rd;
    t[0b01010011] = Dis_MOV_E_imm8_Rd;
    t[0b01010100] = Dis_MOV_E_imm8_Rd;
    t[0b01010101] = Dis_MOV_E_imm8_Rd;
    t[0b01010110] = Dis_MOV_E_imm8_Rd;
    t[0b01010111] = Dis_MOV_E_imm8_Rd;
    t[0b01011000] = Dis_MOV_I_W_imm16_Rd;
    t[0b01011001] = Dis_MOV_I_W_imm16_Rd;
    t[0b01011010] = Dis_MOV_I_W_imm16_Rd;
    t[0b01011011] = Dis_MOV_I_W_imm16_Rd;
    t[0b01011100] = Dis_MOV_I_W_imm16_Rd;
    t[0b01011101] = Dis_MOV_I_W_imm16_Rd;
    t[0b01011110] = Dis_MOV_I_W_imm16_Rd;
    t[0b01011111] = Dis_MOV_I_W_imm16_Rd;
    t[0b01100000] = Dis_MOV_L_B_aa8_Rd;
    t[0b01100001] = Dis_MOV_L_B_aa8_Rd;
    t[0b01100010] = Dis_MOV_L_B_aa8_Rd;
    t[0b01100011] = Dis_MOV_L_B_aa8_Rd;
    t[0b01100100] = Dis_MOV_L_B_aa8_Rd;
    t[0b01100101] = Dis_MOV_L_B_aa8_Rd;
    t[0b01100110] = Dis_MOV_L_B_aa8_Rd;
    t[0b01100111] = Dis_MOV_L_B_aa8_Rd;
    t[0b01101000] = Dis_MOV_L_W_aa8_Rd;
    t[0b01101001] = Dis_MOV_L_W_aa8_Rd;
    t[0b01101010] = Dis_MOV_L_W_aa8_Rd;
    t[0b01101011] = Dis_MOV_L_W_aa8_Rd;
    t[0b01101100] = Dis_MOV_L_W_aa8_Rd;
    t[0b01101101] = Dis_MOV_L_W_aa8_Rd;
    t[0b01101110] = Dis_MOV_L_W_aa8_Rd;
    t[0b01101111] = Dis_MOV_L_W_aa8_Rd;
    t[0b01110000] = Dis_MOV_S_B_Rs_aa8;
    t[0b01110001] = Dis_MOV_S_B_Rs_aa8;
    t[0b01110010] = Dis_MOV_S_B_Rs_aa8;
    t[0b01110011] = Dis_MOV_S_B_Rs_aa8;
    t[0b01110100] = Dis_MOV_S_B_Rs_aa8;
    t[0b01110101] = Dis_MOV_S_B_Rs_aa8;
    t[0b01110110] = Dis_MOV_S_B_Rs_aa8;
    t[0b01110111] = Dis_MOV_S_B_Rs_aa8;
    t[0b01111000] = Dis_MOV_S_W_Rs_aa8;
    t[0b01111001] = Dis_MOV_S_W_Rs_aa8;
    t[0b01111010] = Dis_MOV_S_W_Rs_aa8;
    t[0b01111011] = Dis_MOV_S_W_Rs_aa8;
    t[0b01111100] = Dis_MOV_S_W_Rs_aa8;
    t[0b01111101] = Dis_MOV_S_W_Rs_aa8;
    t[0b01111110] = Dis_MOV_S_W_Rs_aa8;
    t[0b01111111] = Dis_MOV_S_W_Rs_aa8;
    t[0b10000000] = Dis_MOV_F_B_d8_R6_Rd;
    t[0b10000010] = Dis_MOV_F_B_d8_R6_Rd;
    t[0b10000001] = Dis_MOV_F_B_d8_R6_Rd;
    t[0b10000011] = Dis_MOV_F_B_d8_R6_Rd;
    t[0b10000100] = Dis_MOV_F_B_d8_R6_Rd;
    t[0b10000101] = Dis_MOV_F_B_d8_R6_Rd;
    t[0b10000110] = Dis_MOV_F_B_d8_R6_Rd;
    t[0b10000111] = Dis_MOV_F_B_d8_R6_Rd;
    t[0b10001000] = Dis_MOV_F_W_d8_R6_Rd;
    t[0b10001001] = Dis_MOV_F_W_d8_R6_Rd;
    t[0b10001010] = Dis_MOV_F_W_d8_R6_Rd;
    t[0b10001011] = Dis_MOV_F_W_d8_R6_Rd;
    t[0b10001100] = Dis_MOV_F_W_d8_R6_Rd;
    t[0b10001101] = Dis_MOV_F_W_d8_R6_Rd;
    t[0b10001110] = Dis_MOV_F_W_d8_R6_Rd;
    t[0b10001111] = Dis_MOV_F_W_d8_R6_Rd;
    t[0b10010000] = Dis_MOV_F_B_Rs_d8_R6;
    t[0b10010001] = Dis_MOV_F_B_Rs_d8_R6;
    t[0b10010010] = Dis_MOV_F_B_Rs_d8_R6;
    t[0b10010011] = Dis_MOV_F_B_Rs_d8_R6;
    t[0b10010100] = Dis_MOV_F_B_Rs_d8_R6;
    t[0b10010101] = Dis_MOV_F_B_Rs_d8_R6;
    t[0b10010110] = Dis_MOV_F_B_Rs_d8_R6;
    t[0b10010111] = Dis_MOV_F_B_Rs_d8_R6;
    t[0b10011000] = Dis_MOV_F_W_Rs_d8_R6;
    t[0b10011001] = Dis_MOV_F_W_Rs_d8_R6;
    t[0b10011010] = Dis_MOV_F_W_Rs_d8_R6;
    t[0b10011011] = Dis_MOV_F_W_Rs_d8_R6;
    t[0b10011100] = Dis_MOV_F_W_Rs_d8_R6;
    t[0b10011101] = Dis_MOV_F_W_Rs_d8_R6;
    t[0b10011110] = Dis_MOV_F_W_Rs_d8_R6;
    t[0b10011111] = Dis_MOV_F_W_Rs_d8_R6;
    t[0b10100000] = Dis_Rn;
    t[0b10100001] = Dis_Rn;
    t[0b10100010] = Dis_Rn;
    t[0b10100011] = Dis_Rn;
    t[0b10100100] = Dis_Rn;
    t[0b10100101] = Dis_Rn;
    t[0b10100110] = Dis_Rn;
    t[0b10100111] = Dis_Rn;
    t[0b10101000] = Dis_Rn;
    t[0b10101001] = Dis_Rn;
    t[0b10101010] = Dis_Rn;
    t[0b10101011] = Dis_Rn;
    t[0b10101100] = Dis_Rn;
    t[0b10101101] = Dis_Rn;
    t[0b10101110] = Dis_Rn;
    t[0b10101111] = Dis_Rn;
    t[0b10110000] = Dis_APreDecRn;
    t[0b10110001] = Dis_APreDecRn;
    t[0b10110010] = Dis_APreDecRn;
    t[0b10110011] = Dis_APreDecRn;
    t[0b10110100] = Dis_APreDecRn;
    t[0b10110101] = Dis_APreDecRn;
    t[0b10110110] = Dis_APreDecRn;
    t[0b10110111] = Dis_APreDecRn;
    t[0b10111000] = Dis_APreDecRn;
    t[0b10111001] = Dis_APreDecRn;
    t[0b10111010] = Dis_APreDecRn;
    t[0b10111011] = Dis_APreDecRn;
    t[0b10111100] = Dis_APreDecRn;
    t[0b10111101] = Dis_APreDecRn;
    t[0b10111110] = Dis_APreDecRn;
    t[0b10111111] = Dis_APreDecRn;
    t[0b11000000] = Dis_APostIncRn;
    t[0b11000001] = Dis_APostIncRn;
    t[0b11000010] = Dis_APostIncRn;
    t[0b11000011] = Dis_APostIncRn;
    t[0b11000100] = Dis_APostIncRn;
    t[0b11000101] = Dis_APostIncRn;
    t[0b11000110] = Dis_APostIncRn;
    t[0b11000111] = Dis_APostIncRn;
    t[0b11001000] = Dis_APostIncRn;
    t[0b11001001] = Dis_APostIncRn;
    t[0b11001010] = Dis_APostIncRn;
    t[0b11001011] = Dis_APostIncRn;
    t[0b11001100] = Dis_APostIncRn;
    t[0b11001101] = Dis_APostIncRn;
    t[0b11001110] = Dis_APostIncRn;
    t[0b11001111] = Dis_APostIncRn;
    t[0b11010000] = Dis_ARn;
    t[0b11010001] = Dis_ARn;
    t[0b11010010] = Dis_ARn;
    t[0b11010011] = Dis_ARn;
    t[0b11010100] = Dis_ARn;
    t[0b11010101] = Dis_ARn;
    t[0b11010110] = Dis_ARn;
    t[0b11010111] = Dis_ARn;
    t[0b11011000] = Dis_ARn;
    t[0b11011001] = Dis_ARn;
    t[0b11011010] = Dis_ARn;
    t[0b11011011] = Dis_ARn;
    t[0b11011100] = Dis_ARn;
    t[0b11011101] = Dis_ARn;
    t[0b11011110] = Dis_ARn;
    t[0b11011111] = Dis_ARn;
    t[0b11100000] = Dis_Ad8_Rn;
    t[0b11100001] = Dis_Ad8_Rn;
    t[0b11100010] = Dis_Ad8_Rn;
    t[0b11100011] = Dis_Ad8_Rn;
    t[0b11100100] = Dis_Ad8_Rn;
    t[0b11100101] = Dis_Ad8_Rn;
    t[0b11100110] = Dis_Ad8_Rn;
    t[0b11100111] = Dis_Ad8_Rn;
    t[0b11101000] = Dis_Ad8_Rn;
    t[0b11101001] = Dis_Ad8_Rn;
    t[0b11101010] = Dis_Ad8_Rn;
    t[0b11101011] = Dis_Ad8_Rn;
    t[0b11101100] = Dis_Ad8_Rn;
    t[0b11101101] = Dis_Ad8_Rn;
    t[0b11101110] = Dis_Ad8_Rn;
    t[0b11101111] = Dis_Ad8_Rn;
    t[0b11110000] = Dis_Ad16_Rn;
    t[0b11110001] = Dis_Ad16_Rn;
    t[0b11110010] = Dis_Ad16_Rn;
    t[0b11110011] = Dis_Ad16_Rn;
    t[0b11110100] = Dis_Ad16_Rn;
    t[0b11110101] = Dis_Ad16_Rn;
    t[0b11110110] = Dis_Ad16_Rn;
    t[0b11110111] = Dis_Ad16_Rn;
    t[0b11111000] = Dis_Ad16_Rn;
    t[0b11111001] = Dis_Ad16_Rn;
    t[0b11111010] = Dis_Ad16_Rn;
    t[0b11111011] = Dis_Ad16_Rn;
    t[0b11111100] = Dis_Ad16_Rn;
    t[0b11111101] = Dis_Ad16_Rn;
    t[0b11111110] = Dis_Ad16_Rn;
    t[0b11111111] = Dis_Ad16_Rn;
    return t;
}

constexpr std::array<Disassembler, 256> DIS_TOP = MakeDecodeTable();

Disassembler GetDecoderTop(uint8_t byte)
{
    return DIS_TOP[byte];
}

} // namespace decoder2
