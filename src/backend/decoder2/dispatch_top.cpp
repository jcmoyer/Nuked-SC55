#include <cstdint>

#include "dispatch.h"
#include "dispatch_aa.h"
#include "dispatch_arn.h"
#include "dispatch_d8d16_rn.h"
#include "dispatch_handlers.h"
#include "dispatch_imm16.h"
#include "dispatch_postinc_rn.h"
#include "dispatch_predec_rn.h"
#include "dispatch_rn.h"
#include "instruction_handlers.h"
#include "mcu.h"

template <MCU_Operand_Size Sz, uint8_t Rn>
void D_General_Rn(mcu_t& mcu, uint32_t instr_start, uint8_t byte)
{
    (void)byte;

    I_CachedInstruction instr{};
    instr.ea_reg = Rn;

    const uint8_t   opcode  = mcu.coder.ReadU8(mcu);
    D_OpcodeHandler handler = GetDispatcherRn(opcode, Sz);
    if (handler)
    {
        handler(mcu, instr_start, byte, instr);
    }
    else
    {
        D_HardError(mcu, "D_General_Rn");
    }
}

template <MCU_Operand_Size Sz, uint8_t Rn>
void D_General_PreDecRn(mcu_t& mcu, uint32_t instr_start, uint8_t byte)
{
    (void)byte;

    I_CachedInstruction instr{};
    instr.ea_reg = Rn;

    const uint8_t   opcode  = mcu.coder.ReadU8(mcu);
    D_OpcodeHandler handler = GetDispatcherPreDecRn(opcode, Sz);
    if (handler)
    {
        handler(mcu, instr_start, byte, instr);
    }
    else
    {
        D_HardError(mcu, "D_General_PreDecRn");
    }
}

template <MCU_Operand_Size Sz, uint8_t Rn>
void D_General_PostIncRn(mcu_t& mcu, uint32_t instr_start, uint8_t byte)
{
    (void)byte;

    I_CachedInstruction instr{};
    instr.ea_reg = Rn;

    const uint8_t   opcode  = mcu.coder.ReadU8(mcu);
    D_OpcodeHandler handler = GetDispatcherPostIncRn(opcode, Sz);
    if (handler)
    {
        handler(mcu, instr_start, byte, instr);
    }
    else
    {
        D_HardError(mcu, "D_General_PostIncRn");
    }
}

template <MCU_Operand_Size Sz, uint8_t Rn>
void D_General_ARn(mcu_t& mcu, uint32_t instr_start, uint8_t byte)
{
    (void)byte;

    I_CachedInstruction instr{};
    instr.ea_reg = Rn;

    const uint8_t   opcode  = mcu.coder.ReadU8(mcu);
    D_OpcodeHandler handler = GetDispatcherARn(opcode, Sz);
    if (handler)
    {
        handler(mcu, instr_start, byte, instr);
    }
    else
    {
        D_HardError(mcu, "D_General_ARn");
    }
}

template <MCU_Operand_Size Sz, uint8_t Rn>
void D_General_d8_Rn(mcu_t& mcu, uint32_t instr_start, uint8_t byte)
{
    (void)byte;

    const int16_t disp = (int8_t)mcu.coder.ReadU8(mcu);

    I_CachedInstruction instr{};
    instr.ea_disp = disp;
    instr.ea_reg  = Rn;

    const uint8_t   opcode  = mcu.coder.ReadU8(mcu);
    D_OpcodeHandler handler = GetDispatcherd8Rn(opcode, Sz);
    if (handler)
    {
        handler(mcu, instr_start, byte, instr);
    }
    else
    {
        D_HardError(mcu, "D_General_d8_Rn");
    }
}

template <MCU_Operand_Size Sz, uint8_t Rn>
void D_General_d16_Rn(mcu_t& mcu, uint32_t instr_start, uint8_t byte)
{
    (void)byte;

    const int16_t disp = (int16_t)mcu.coder.ReadU16(mcu);

    I_CachedInstruction instr{};
    instr.ea_disp = disp;
    instr.ea_reg  = Rn;

    const uint8_t   opcode  = mcu.coder.ReadU8(mcu);
    D_OpcodeHandler handler = GetDispatcherd16Rn(opcode, Sz);
    if (handler)
    {
        handler(mcu, instr_start, byte, instr);
    }
    else
    {
        D_HardError(mcu, "D_General_d16_Rn");
    }
}

void D_General_imm8(mcu_t& mcu, uint32_t instr_start, uint8_t byte)
{
    (void)byte;

    const uint16_t imm = mcu.coder.ReadU8(mcu);

    I_CachedInstruction instr{};
    instr.ea_data = imm;

    const uint8_t   opcode  = mcu.coder.ReadU8(mcu);
    D_OpcodeHandler handler = GetDispatcherimm16(opcode, MCU_Operand_Size::BYTE);
    if (handler)
    {
        handler(mcu, instr_start, byte, instr);
    }
    else
    {
        D_HardError(mcu, "D_General_imm8");
    }
}

void D_General_imm16(mcu_t& mcu, uint32_t instr_start, uint8_t byte)
{
    (void)byte;

    const uint16_t imm = mcu.coder.ReadU16(mcu);

    I_CachedInstruction instr{};
    instr.ea_data = imm;

    const uint8_t   opcode  = mcu.coder.ReadU8(mcu);
    D_OpcodeHandler handler = GetDispatcherimm16(opcode, MCU_Operand_Size::WORD);
    if (handler)
    {
        handler(mcu, instr_start, byte, instr);
    }
    else
    {
        D_HardError(mcu, "D_General_imm16");
    }
}

template <MCU_Operand_Size Sz>
void D_General_Aa8(mcu_t& mcu, uint32_t instr_start, uint8_t byte)
{
    (void)byte;

    const uint8_t imm = mcu.coder.ReadU8(mcu);

    I_CachedInstruction instr{};
    instr.ea_data = imm;

    const uint8_t   opcode  = mcu.coder.ReadU8(mcu);
    D_OpcodeHandler handler = GetDispatcherAa8(opcode, Sz);
    if (handler)
    {
        handler(mcu, instr_start, byte, instr);
    }
    else
    {
        D_HardError(mcu, "D_General_Aa8");
    }
}

template <MCU_Operand_Size Sz>
void D_General_Aa16(mcu_t& mcu, uint32_t instr_start, uint8_t byte)
{
    (void)byte;

    const uint16_t imm = (uint16_t)mcu.coder.ReadU16(mcu);

    I_CachedInstruction instr{};
    instr.ea_data = imm;

    const uint8_t   opcode  = mcu.coder.ReadU8(mcu);
    D_OpcodeHandler handler = GetDispatcherAa16(opcode, Sz);
    if (handler)
    {
        handler(mcu, instr_start, byte, instr);
    }
    else
    {
        D_HardError(mcu, "D_General_Aa16");
    }
}

constexpr D_Handler DECODE_TABLE_0[256] = {
    D_NOP,                                             // 00000000
    D_Short_SCB,                                       // 00000001
    D_Short_LDM,                                       // 00000010
    D_Short_PJSR_aa24,                                 // 00000011
    D_General_imm8,                                    // 00000100
    D_General_Aa8<MCU_Operand_Size::BYTE>,             // 00000101
    D_Short_SCB,                                       // 00000110
    D_Short_SCB,                                       // 00000111
    D_Short_TRAPA,                                     // 00001000
    nullptr,                                           // 00001001
    D_RTE,                                             // 00001010
    nullptr,                                           // 00001011
    D_General_imm16,                                   // 00001100
    D_General_Aa8<MCU_Operand_Size::WORD>,             // 00001101
    D_BSR_d8,                                          // 00001110
    nullptr,                                           // 00001111
    D_Short_JMP_aa16,                                  // 00010000
    D_JMP,                                             // 00010001
    D_Short_STM,                                       // 00010010
    D_Short_PJMP_aa24,                                 // 00010011
    nullptr,                                           // 00010100
    D_General_Aa16<MCU_Operand_Size::BYTE>,            // 00010101
    nullptr,                                           // 00010110
    nullptr,                                           // 00010111
    D_Short_JSR_aa16,                                  // 00011000
    D_Short_RTS,                                       // 00011001
    D_Short_SLEEP,                                     // 00011010
    nullptr,                                           // 00011011
    nullptr,                                           // 00011100
    D_General_Aa16<MCU_Operand_Size::WORD>,            // 00011101
    D_BSR_d16,                                         // 00011110
    nullptr,                                           // 00011111
    D_Bcc<MCU_Operand_Size::BYTE>,                     // 00100000
    D_Bcc<MCU_Operand_Size::BYTE>,                     // 00100001
    D_Bcc<MCU_Operand_Size::BYTE>,                     // 00100010
    D_Bcc<MCU_Operand_Size::BYTE>,                     // 00100011
    D_Bcc<MCU_Operand_Size::BYTE>,                     // 00100100
    D_Bcc<MCU_Operand_Size::BYTE>,                     // 00100101
    D_Bcc<MCU_Operand_Size::BYTE>,                     // 00100110
    D_Bcc<MCU_Operand_Size::BYTE>,                     // 00100111
    D_Bcc<MCU_Operand_Size::BYTE>,                     // 00101000
    D_Bcc<MCU_Operand_Size::BYTE>,                     // 00101001
    D_Bcc<MCU_Operand_Size::BYTE>,                     // 00101010
    D_Bcc<MCU_Operand_Size::BYTE>,                     // 00101011
    D_Bcc<MCU_Operand_Size::BYTE>,                     // 00101100
    D_Bcc<MCU_Operand_Size::BYTE>,                     // 00101101
    D_Bcc<MCU_Operand_Size::BYTE>,                     // 00101110
    D_Bcc<MCU_Operand_Size::BYTE>,                     // 00101111
    D_Bcc<MCU_Operand_Size::WORD>,                     // 00110000
    D_Bcc<MCU_Operand_Size::WORD>,                     // 00110001
    D_Bcc<MCU_Operand_Size::WORD>,                     // 00110010
    D_Bcc<MCU_Operand_Size::WORD>,                     // 00110011
    D_Bcc<MCU_Operand_Size::WORD>,                     // 00110100
    D_Bcc<MCU_Operand_Size::WORD>,                     // 00110101
    D_Bcc<MCU_Operand_Size::WORD>,                     // 00110110
    D_Bcc<MCU_Operand_Size::WORD>,                     // 00110111
    D_Bcc<MCU_Operand_Size::WORD>,                     // 00111000
    D_Bcc<MCU_Operand_Size::WORD>,                     // 00111001
    D_Bcc<MCU_Operand_Size::WORD>,                     // 00111010
    D_Bcc<MCU_Operand_Size::WORD>,                     // 00111011
    D_Bcc<MCU_Operand_Size::WORD>,                     // 00111100
    D_Bcc<MCU_Operand_Size::WORD>,                     // 00111101
    D_Bcc<MCU_Operand_Size::WORD>,                     // 00111110
    D_Bcc<MCU_Operand_Size::WORD>,                     // 00111111
    D_Short_CMP_E_imm8_Rd<0>,                          // 01000000
    D_Short_CMP_E_imm8_Rd<1>,                          // 01000001
    D_Short_CMP_E_imm8_Rd<2>,                          // 01000010
    D_Short_CMP_E_imm8_Rd<3>,                          // 01000011
    D_Short_CMP_E_imm8_Rd<4>,                          // 01000100
    D_Short_CMP_E_imm8_Rd<5>,                          // 01000101
    D_Short_CMP_E_imm8_Rd<6>,                          // 01000110
    D_Short_CMP_E_imm8_Rd<7>,                          // 01000111
    D_Short_CMP_I_W_imm16_Rd<0>,                       // 01001000
    D_Short_CMP_I_W_imm16_Rd<1>,                       // 01001001
    D_Short_CMP_I_W_imm16_Rd<2>,                       // 01001010
    D_Short_CMP_I_W_imm16_Rd<3>,                       // 01001011
    D_Short_CMP_I_W_imm16_Rd<4>,                       // 01001100
    D_Short_CMP_I_W_imm16_Rd<5>,                       // 01001101
    D_Short_CMP_I_W_imm16_Rd<6>,                       // 01001110
    D_Short_CMP_I_W_imm16_Rd<7>,                       // 01001111
    D_Short_MOV_E_imm8_Rd<0>,                          // 01010000
    D_Short_MOV_E_imm8_Rd<1>,                          // 01010001
    D_Short_MOV_E_imm8_Rd<2>,                          // 01010010
    D_Short_MOV_E_imm8_Rd<3>,                          // 01010011
    D_Short_MOV_E_imm8_Rd<4>,                          // 01010100
    D_Short_MOV_E_imm8_Rd<5>,                          // 01010101
    D_Short_MOV_E_imm8_Rd<6>,                          // 01010110
    D_Short_MOV_E_imm8_Rd<7>,                          // 01010111
    D_Short_MOV_I_W_imm16_Rd<0>,                       // 01011000
    D_Short_MOV_I_W_imm16_Rd<1>,                       // 01011001
    D_Short_MOV_I_W_imm16_Rd<2>,                       // 01011010
    D_Short_MOV_I_W_imm16_Rd<3>,                       // 01011011
    D_Short_MOV_I_W_imm16_Rd<4>,                       // 01011100
    D_Short_MOV_I_W_imm16_Rd<5>,                       // 01011101
    D_Short_MOV_I_W_imm16_Rd<6>,                       // 01011110
    D_Short_MOV_I_W_imm16_Rd<7>,                       // 01011111
    D_Short_MOV_L_aa8_Rd<MCU_Operand_Size::BYTE, 0>,   // 01100000
    D_Short_MOV_L_aa8_Rd<MCU_Operand_Size::BYTE, 1>,   // 01100001
    D_Short_MOV_L_aa8_Rd<MCU_Operand_Size::BYTE, 2>,   // 01100010
    D_Short_MOV_L_aa8_Rd<MCU_Operand_Size::BYTE, 3>,   // 01100011
    D_Short_MOV_L_aa8_Rd<MCU_Operand_Size::BYTE, 4>,   // 01100100
    D_Short_MOV_L_aa8_Rd<MCU_Operand_Size::BYTE, 5>,   // 01100101
    D_Short_MOV_L_aa8_Rd<MCU_Operand_Size::BYTE, 6>,   // 01100110
    D_Short_MOV_L_aa8_Rd<MCU_Operand_Size::BYTE, 7>,   // 01100111
    D_Short_MOV_L_aa8_Rd<MCU_Operand_Size::WORD, 0>,   // 01101000
    D_Short_MOV_L_aa8_Rd<MCU_Operand_Size::WORD, 1>,   // 01101001
    D_Short_MOV_L_aa8_Rd<MCU_Operand_Size::WORD, 2>,   // 01101010
    D_Short_MOV_L_aa8_Rd<MCU_Operand_Size::WORD, 3>,   // 01101011
    D_Short_MOV_L_aa8_Rd<MCU_Operand_Size::WORD, 4>,   // 01101100
    D_Short_MOV_L_aa8_Rd<MCU_Operand_Size::WORD, 5>,   // 01101101
    D_Short_MOV_L_aa8_Rd<MCU_Operand_Size::WORD, 6>,   // 01101110
    D_Short_MOV_L_aa8_Rd<MCU_Operand_Size::WORD, 7>,   // 01101111
    D_Short_I_MOV_S_Rs_aa8<MCU_Operand_Size::BYTE, 0>, // 01110000
    D_Short_I_MOV_S_Rs_aa8<MCU_Operand_Size::BYTE, 1>, // 01110001
    D_Short_I_MOV_S_Rs_aa8<MCU_Operand_Size::BYTE, 2>, // 01110010
    D_Short_I_MOV_S_Rs_aa8<MCU_Operand_Size::BYTE, 3>, // 01110011
    D_Short_I_MOV_S_Rs_aa8<MCU_Operand_Size::BYTE, 4>, // 01110100
    D_Short_I_MOV_S_Rs_aa8<MCU_Operand_Size::BYTE, 5>, // 01110101
    D_Short_I_MOV_S_Rs_aa8<MCU_Operand_Size::BYTE, 6>, // 01110110
    D_Short_I_MOV_S_Rs_aa8<MCU_Operand_Size::BYTE, 7>, // 01110111
    D_Short_I_MOV_S_Rs_aa8<MCU_Operand_Size::WORD, 0>, // 01111000
    D_Short_I_MOV_S_Rs_aa8<MCU_Operand_Size::WORD, 1>, // 01111001
    D_Short_I_MOV_S_Rs_aa8<MCU_Operand_Size::WORD, 2>, // 01111010
    D_Short_I_MOV_S_Rs_aa8<MCU_Operand_Size::WORD, 3>, // 01111011
    D_Short_I_MOV_S_Rs_aa8<MCU_Operand_Size::WORD, 4>, // 01111100
    D_Short_I_MOV_S_Rs_aa8<MCU_Operand_Size::WORD, 5>, // 01111101
    D_Short_I_MOV_S_Rs_aa8<MCU_Operand_Size::WORD, 6>, // 01111110
    D_Short_I_MOV_S_Rs_aa8<MCU_Operand_Size::WORD, 7>, // 01111111
    nullptr,                                           // 10000000
    nullptr,                                           // 10000001
    nullptr,                                           // 10000010
    nullptr,                                           // 10000011
    nullptr,                                           // 10000100
    nullptr,                                           // 10000101
    nullptr,                                           // 10000110
    nullptr,                                           // 10000111
    nullptr,                                           // 10001000
    nullptr,                                           // 10001001
    nullptr,                                           // 10001010
    nullptr,                                           // 10001011
    nullptr,                                           // 10001100
    nullptr,                                           // 10001101
    nullptr,                                           // 10001110
    nullptr,                                           // 10001111
    nullptr,                                           // 10010000
    nullptr,                                           // 10010001
    nullptr,                                           // 10010010
    nullptr,                                           // 10010011
    nullptr,                                           // 10010100
    nullptr,                                           // 10010101
    nullptr,                                           // 10010110
    nullptr,                                           // 10010111
    nullptr,                                           // 10011000
    nullptr,                                           // 10011001
    nullptr,                                           // 10011010
    nullptr,                                           // 10011011
    nullptr,                                           // 10011100
    nullptr,                                           // 10011101
    nullptr,                                           // 10011110
    nullptr,                                           // 10011111
    D_General_Rn<MCU_Operand_Size::BYTE, 0>,           // 10100000
    D_General_Rn<MCU_Operand_Size::BYTE, 1>,           // 10100001
    D_General_Rn<MCU_Operand_Size::BYTE, 2>,           // 10100010
    D_General_Rn<MCU_Operand_Size::BYTE, 3>,           // 10100011
    D_General_Rn<MCU_Operand_Size::BYTE, 4>,           // 10100100
    D_General_Rn<MCU_Operand_Size::BYTE, 5>,           // 10100101
    D_General_Rn<MCU_Operand_Size::BYTE, 6>,           // 10100110
    D_General_Rn<MCU_Operand_Size::BYTE, 7>,           // 10100111
    D_General_Rn<MCU_Operand_Size::WORD, 0>,           // 10101000
    D_General_Rn<MCU_Operand_Size::WORD, 1>,           // 10101001
    D_General_Rn<MCU_Operand_Size::WORD, 2>,           // 10101010
    D_General_Rn<MCU_Operand_Size::WORD, 3>,           // 10101011
    D_General_Rn<MCU_Operand_Size::WORD, 4>,           // 10101100
    D_General_Rn<MCU_Operand_Size::WORD, 5>,           // 10101101
    D_General_Rn<MCU_Operand_Size::WORD, 6>,           // 10101110
    D_General_Rn<MCU_Operand_Size::WORD, 7>,           // 10101111
    D_General_PreDecRn<MCU_Operand_Size::BYTE, 0>,     // 10110000
    D_General_PreDecRn<MCU_Operand_Size::BYTE, 1>,     // 10110001
    D_General_PreDecRn<MCU_Operand_Size::BYTE, 2>,     // 10110010
    D_General_PreDecRn<MCU_Operand_Size::BYTE, 3>,     // 10110011
    D_General_PreDecRn<MCU_Operand_Size::BYTE, 4>,     // 10110100
    D_General_PreDecRn<MCU_Operand_Size::BYTE, 5>,     // 10110101
    D_General_PreDecRn<MCU_Operand_Size::BYTE, 6>,     // 10110110
    D_General_PreDecRn<MCU_Operand_Size::BYTE, 7>,     // 10110111
    D_General_PreDecRn<MCU_Operand_Size::WORD, 0>,     // 10111000
    D_General_PreDecRn<MCU_Operand_Size::WORD, 1>,     // 10111001
    D_General_PreDecRn<MCU_Operand_Size::WORD, 2>,     // 10111010
    D_General_PreDecRn<MCU_Operand_Size::WORD, 3>,     // 10111011
    D_General_PreDecRn<MCU_Operand_Size::WORD, 4>,     // 10111100
    D_General_PreDecRn<MCU_Operand_Size::WORD, 5>,     // 10111101
    D_General_PreDecRn<MCU_Operand_Size::WORD, 6>,     // 10111110
    D_General_PreDecRn<MCU_Operand_Size::WORD, 7>,     // 10111111
    D_General_PostIncRn<MCU_Operand_Size::BYTE, 0>,    // 11000000
    D_General_PostIncRn<MCU_Operand_Size::BYTE, 1>,    // 11000001
    D_General_PostIncRn<MCU_Operand_Size::BYTE, 2>,    // 11000010
    D_General_PostIncRn<MCU_Operand_Size::BYTE, 3>,    // 11000011
    D_General_PostIncRn<MCU_Operand_Size::BYTE, 4>,    // 11000100
    D_General_PostIncRn<MCU_Operand_Size::BYTE, 5>,    // 11000101
    D_General_PostIncRn<MCU_Operand_Size::BYTE, 6>,    // 11000110
    D_General_PostIncRn<MCU_Operand_Size::BYTE, 7>,    // 11000111
    D_General_PostIncRn<MCU_Operand_Size::WORD, 0>,    // 11001000
    D_General_PostIncRn<MCU_Operand_Size::WORD, 1>,    // 11001001
    D_General_PostIncRn<MCU_Operand_Size::WORD, 2>,    // 11001010
    D_General_PostIncRn<MCU_Operand_Size::WORD, 3>,    // 11001011
    D_General_PostIncRn<MCU_Operand_Size::WORD, 4>,    // 11001100
    D_General_PostIncRn<MCU_Operand_Size::WORD, 5>,    // 11001101
    D_General_PostIncRn<MCU_Operand_Size::WORD, 6>,    // 11001110
    D_General_PostIncRn<MCU_Operand_Size::WORD, 7>,    // 11001111
    D_General_ARn<MCU_Operand_Size::BYTE, 0>,          // 11010000
    D_General_ARn<MCU_Operand_Size::BYTE, 1>,          // 11010001
    D_General_ARn<MCU_Operand_Size::BYTE, 2>,          // 11010010
    D_General_ARn<MCU_Operand_Size::BYTE, 3>,          // 11010011
    D_General_ARn<MCU_Operand_Size::BYTE, 4>,          // 11010100
    D_General_ARn<MCU_Operand_Size::BYTE, 5>,          // 11010101
    D_General_ARn<MCU_Operand_Size::BYTE, 6>,          // 11010110
    D_General_ARn<MCU_Operand_Size::BYTE, 7>,          // 11010111
    D_General_ARn<MCU_Operand_Size::WORD, 0>,          // 11011000
    D_General_ARn<MCU_Operand_Size::WORD, 1>,          // 11011001
    D_General_ARn<MCU_Operand_Size::WORD, 2>,          // 11011010
    D_General_ARn<MCU_Operand_Size::WORD, 3>,          // 11011011
    D_General_ARn<MCU_Operand_Size::WORD, 4>,          // 11011100
    D_General_ARn<MCU_Operand_Size::WORD, 5>,          // 11011101
    D_General_ARn<MCU_Operand_Size::WORD, 6>,          // 11011110
    D_General_ARn<MCU_Operand_Size::WORD, 7>,          // 11011111
    D_General_d8_Rn<MCU_Operand_Size::BYTE, 0>,        // 11100000
    D_General_d8_Rn<MCU_Operand_Size::BYTE, 1>,        // 11100001
    D_General_d8_Rn<MCU_Operand_Size::BYTE, 2>,        // 11100010
    D_General_d8_Rn<MCU_Operand_Size::BYTE, 3>,        // 11100011
    D_General_d8_Rn<MCU_Operand_Size::BYTE, 4>,        // 11100100
    D_General_d8_Rn<MCU_Operand_Size::BYTE, 5>,        // 11100101
    D_General_d8_Rn<MCU_Operand_Size::BYTE, 6>,        // 11100110
    D_General_d8_Rn<MCU_Operand_Size::BYTE, 7>,        // 11100111
    D_General_d8_Rn<MCU_Operand_Size::WORD, 0>,        // 11101000
    D_General_d8_Rn<MCU_Operand_Size::WORD, 1>,        // 11101001
    D_General_d8_Rn<MCU_Operand_Size::WORD, 2>,        // 11101010
    D_General_d8_Rn<MCU_Operand_Size::WORD, 3>,        // 11101011
    D_General_d8_Rn<MCU_Operand_Size::WORD, 4>,        // 11101100
    D_General_d8_Rn<MCU_Operand_Size::WORD, 5>,        // 11101101
    D_General_d8_Rn<MCU_Operand_Size::WORD, 6>,        // 11101110
    D_General_d8_Rn<MCU_Operand_Size::WORD, 7>,        // 11101111
    D_General_d16_Rn<MCU_Operand_Size::BYTE, 0>,       // 11110000
    D_General_d16_Rn<MCU_Operand_Size::BYTE, 1>,       // 11110001
    D_General_d16_Rn<MCU_Operand_Size::BYTE, 2>,       // 11110010
    D_General_d16_Rn<MCU_Operand_Size::BYTE, 3>,       // 11110011
    D_General_d16_Rn<MCU_Operand_Size::BYTE, 4>,       // 11110100
    D_General_d16_Rn<MCU_Operand_Size::BYTE, 5>,       // 11110101
    D_General_d16_Rn<MCU_Operand_Size::BYTE, 6>,       // 11110110
    D_General_d16_Rn<MCU_Operand_Size::BYTE, 7>,       // 11110111
    D_General_d16_Rn<MCU_Operand_Size::WORD, 0>,       // 11111000
    D_General_d16_Rn<MCU_Operand_Size::WORD, 1>,       // 11111001
    D_General_d16_Rn<MCU_Operand_Size::WORD, 2>,       // 11111010
    D_General_d16_Rn<MCU_Operand_Size::WORD, 3>,       // 11111011
    D_General_d16_Rn<MCU_Operand_Size::WORD, 4>,       // 11111100
    D_General_d16_Rn<MCU_Operand_Size::WORD, 5>,       // 11111101
    D_General_d16_Rn<MCU_Operand_Size::WORD, 6>,       // 11111110
    D_General_d16_Rn<MCU_Operand_Size::WORD, 7>,       // 11111111
};

D_Handler GetDispatcherTop(uint8_t byte)
{
    return DECODE_TABLE_0[byte];
}
