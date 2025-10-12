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
        D_HardError(mcu);
    }
}

template <MCU_Operand_Size Sz, uint8_t Rn>
void D_General_APreDecRn(mcu_t& mcu, uint32_t instr_start, uint8_t byte)
{
    (void)byte;

    I_CachedInstruction instr{};
    instr.ea_reg = Rn;

    const uint8_t   opcode  = mcu.coder.ReadU8(mcu);
    D_OpcodeHandler handler = GetDispatcherAPreDecRn(opcode, Sz);
    if (handler)
    {
        handler(mcu, instr_start, byte, instr);
    }
    else
    {
        D_HardError(mcu);
    }
}

template <MCU_Operand_Size Sz, uint8_t Rn>
void D_General_APostIncRn(mcu_t& mcu, uint32_t instr_start, uint8_t byte)
{
    (void)byte;

    I_CachedInstruction instr{};
    instr.ea_reg = Rn;

    const uint8_t   opcode  = mcu.coder.ReadU8(mcu);
    D_OpcodeHandler handler = GetDispatcherAPostIncRn(opcode, Sz);
    if (handler)
    {
        handler(mcu, instr_start, byte, instr);
    }
    else
    {
        D_HardError(mcu);
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
        D_HardError(mcu);
    }
}

template <MCU_Operand_Size Sz, uint8_t Rn>
void D_General_Ad8_Rn(mcu_t& mcu, uint32_t instr_start, uint8_t byte)
{
    (void)byte;

    const int16_t disp = (int8_t)mcu.coder.ReadU8(mcu);

    I_CachedInstruction instr{};
    instr.ea_disp = disp;
    instr.ea_reg  = Rn;

    const uint8_t   opcode  = mcu.coder.ReadU8(mcu);
    D_OpcodeHandler handler = GetDispatcherAd8Rn(opcode, Sz);
    if (handler)
    {
        handler(mcu, instr_start, byte, instr);
    }
    else
    {
        D_HardError(mcu);
    }
}

template <MCU_Operand_Size Sz, uint8_t Rn>
void D_General_Ad16_Rn(mcu_t& mcu, uint32_t instr_start, uint8_t byte)
{
    (void)byte;

    const int16_t disp = (int16_t)mcu.coder.ReadU16(mcu);

    I_CachedInstruction instr{};
    instr.ea_disp = disp;
    instr.ea_reg  = Rn;

    const uint8_t   opcode  = mcu.coder.ReadU8(mcu);
    D_OpcodeHandler handler = GetDispatcherAd16Rn(opcode, Sz);
    if (handler)
    {
        handler(mcu, instr_start, byte, instr);
    }
    else
    {
        D_HardError(mcu);
    }
}

void D_General_imm8(mcu_t& mcu, uint32_t instr_start, uint8_t byte)
{
    (void)byte;

    const uint16_t imm = mcu.coder.ReadU8(mcu);

    I_CachedInstruction instr{};
    instr.ea_data = imm;

    const uint8_t   opcode  = mcu.coder.ReadU8(mcu);
    D_OpcodeHandler handler = GetDispatcherImm8(opcode);
    if (handler)
    {
        handler(mcu, instr_start, byte, instr);
    }
    else
    {
        D_HardError(mcu);
    }
}

void D_General_imm16(mcu_t& mcu, uint32_t instr_start, uint8_t byte)
{
    (void)byte;

    const uint16_t imm = mcu.coder.ReadU16(mcu);

    I_CachedInstruction instr{};
    instr.ea_data = imm;

    const uint8_t   opcode  = mcu.coder.ReadU8(mcu);
    D_OpcodeHandler handler = GetDispatcherImm16(opcode);
    if (handler)
    {
        handler(mcu, instr_start, byte, instr);
    }
    else
    {
        D_HardError(mcu);
    }
}

template <MCU_Operand_Size Sz>
void D_General_Aaa8(mcu_t& mcu, uint32_t instr_start, uint8_t byte)
{
    (void)byte;

    const uint8_t imm = mcu.coder.ReadU8(mcu);

    I_CachedInstruction instr{};
    instr.ea_data = imm;

    const uint8_t   opcode  = mcu.coder.ReadU8(mcu);
    D_OpcodeHandler handler = GetDispatcherAaa8(opcode, Sz);
    if (handler)
    {
        handler(mcu, instr_start, byte, instr);
    }
    else
    {
        D_HardError(mcu);
    }
}

template <MCU_Operand_Size Sz>
void D_General_Aaa16(mcu_t& mcu, uint32_t instr_start, uint8_t byte)
{
    (void)byte;

    const uint16_t imm = (uint16_t)mcu.coder.ReadU16(mcu);

    I_CachedInstruction instr{};
    instr.ea_data = imm;

    const uint8_t   opcode  = mcu.coder.ReadU8(mcu);
    D_OpcodeHandler handler = GetDispatcherAaa16(opcode, Sz);
    if (handler)
    {
        handler(mcu, instr_start, byte, instr);
    }
    else
    {
        D_HardError(mcu);
    }
}

constexpr std::array<D_Handler, 256> MakeDecodeTable()
{
    std::array<D_Handler, 256> t{};
    t[0b00000000] = D_NOP;
    t[0b00000001] = D_SCB;
    t[0b00000010] = D_LDM;
    t[0b00000011] = D_PJSR_aa24;
    t[0b00000100] = D_General_imm8;
    t[0b00000101] = D_General_Aaa8<MCU_Operand_Size::BYTE>;
    t[0b00000110] = D_SCB;
    t[0b00000111] = D_SCB;
    t[0b00001000] = D_TRAPA;
    t[0b00001001] = nullptr;
    t[0b00001010] = D_RTE;
    t[0b00001011] = nullptr;
    t[0b00001100] = D_General_imm16;
    t[0b00001101] = D_General_Aaa8<MCU_Operand_Size::WORD>;
    t[0b00001110] = D_BSR_d8;
    t[0b00001111] = nullptr;
    t[0b00010000] = D_JMP_aa16;
    t[0b00010001] = D_JMP;
    t[0b00010010] = D_STM;
    t[0b00010011] = D_PJMP_aa24;
    t[0b00010100] = D_RTD_imm8;
    t[0b00010101] = D_General_Aaa16<MCU_Operand_Size::BYTE>;
    t[0b00010110] = nullptr;
    t[0b00010111] = nullptr;
    t[0b00011000] = D_JSR_aa16;
    t[0b00011001] = D_RTS;
    t[0b00011010] = D_SLEEP;
    t[0b00011011] = nullptr;
    t[0b00011100] = D_RTD_imm16;
    t[0b00011101] = D_General_Aaa16<MCU_Operand_Size::WORD>;
    t[0b00011110] = D_BSR_d16;
    t[0b00011111] = nullptr;
    t[0b00100000] = D_Bcc<MCU_Operand_Size::BYTE>;
    t[0b00100001] = D_Bcc<MCU_Operand_Size::BYTE>;
    t[0b00100010] = D_Bcc<MCU_Operand_Size::BYTE>;
    t[0b00100011] = D_Bcc<MCU_Operand_Size::BYTE>;
    t[0b00100100] = D_Bcc<MCU_Operand_Size::BYTE>;
    t[0b00100101] = D_Bcc<MCU_Operand_Size::BYTE>;
    t[0b00100110] = D_Bcc<MCU_Operand_Size::BYTE>;
    t[0b00100111] = D_Bcc<MCU_Operand_Size::BYTE>;
    t[0b00101000] = D_Bcc<MCU_Operand_Size::BYTE>;
    t[0b00101001] = D_Bcc<MCU_Operand_Size::BYTE>;
    t[0b00101010] = D_Bcc<MCU_Operand_Size::BYTE>;
    t[0b00101011] = D_Bcc<MCU_Operand_Size::BYTE>;
    t[0b00101100] = D_Bcc<MCU_Operand_Size::BYTE>;
    t[0b00101101] = D_Bcc<MCU_Operand_Size::BYTE>;
    t[0b00101110] = D_Bcc<MCU_Operand_Size::BYTE>;
    t[0b00101111] = D_Bcc<MCU_Operand_Size::BYTE>;
    t[0b00110000] = D_Bcc<MCU_Operand_Size::WORD>;
    t[0b00110001] = D_Bcc<MCU_Operand_Size::WORD>;
    t[0b00110010] = D_Bcc<MCU_Operand_Size::WORD>;
    t[0b00110011] = D_Bcc<MCU_Operand_Size::WORD>;
    t[0b00110100] = D_Bcc<MCU_Operand_Size::WORD>;
    t[0b00110101] = D_Bcc<MCU_Operand_Size::WORD>;
    t[0b00110110] = D_Bcc<MCU_Operand_Size::WORD>;
    t[0b00110111] = D_Bcc<MCU_Operand_Size::WORD>;
    t[0b00111000] = D_Bcc<MCU_Operand_Size::WORD>;
    t[0b00111001] = D_Bcc<MCU_Operand_Size::WORD>;
    t[0b00111010] = D_Bcc<MCU_Operand_Size::WORD>;
    t[0b00111011] = D_Bcc<MCU_Operand_Size::WORD>;
    t[0b00111100] = D_Bcc<MCU_Operand_Size::WORD>;
    t[0b00111101] = D_Bcc<MCU_Operand_Size::WORD>;
    t[0b00111110] = D_Bcc<MCU_Operand_Size::WORD>;
    t[0b00111111] = D_Bcc<MCU_Operand_Size::WORD>;
    t[0b01000000] = D_Short_CMP_E_imm8_Rd<0>;
    t[0b01000001] = D_Short_CMP_E_imm8_Rd<1>;
    t[0b01000010] = D_Short_CMP_E_imm8_Rd<2>;
    t[0b01000011] = D_Short_CMP_E_imm8_Rd<3>;
    t[0b01000100] = D_Short_CMP_E_imm8_Rd<4>;
    t[0b01000101] = D_Short_CMP_E_imm8_Rd<5>;
    t[0b01000110] = D_Short_CMP_E_imm8_Rd<6>;
    t[0b01000111] = D_Short_CMP_E_imm8_Rd<7>;
    t[0b01001000] = D_Short_CMP_I_W_imm16_Rd<0>;
    t[0b01001001] = D_Short_CMP_I_W_imm16_Rd<1>;
    t[0b01001010] = D_Short_CMP_I_W_imm16_Rd<2>;
    t[0b01001011] = D_Short_CMP_I_W_imm16_Rd<3>;
    t[0b01001100] = D_Short_CMP_I_W_imm16_Rd<4>;
    t[0b01001101] = D_Short_CMP_I_W_imm16_Rd<5>;
    t[0b01001110] = D_Short_CMP_I_W_imm16_Rd<6>;
    t[0b01001111] = D_Short_CMP_I_W_imm16_Rd<7>;
    t[0b01010000] = D_Short_MOV_E_imm8_Rd<0>;
    t[0b01010001] = D_Short_MOV_E_imm8_Rd<1>;
    t[0b01010010] = D_Short_MOV_E_imm8_Rd<2>;
    t[0b01010011] = D_Short_MOV_E_imm8_Rd<3>;
    t[0b01010100] = D_Short_MOV_E_imm8_Rd<4>;
    t[0b01010101] = D_Short_MOV_E_imm8_Rd<5>;
    t[0b01010110] = D_Short_MOV_E_imm8_Rd<6>;
    t[0b01010111] = D_Short_MOV_E_imm8_Rd<7>;
    t[0b01011000] = D_Short_MOV_I_W_imm16_Rd<0>;
    t[0b01011001] = D_Short_MOV_I_W_imm16_Rd<1>;
    t[0b01011010] = D_Short_MOV_I_W_imm16_Rd<2>;
    t[0b01011011] = D_Short_MOV_I_W_imm16_Rd<3>;
    t[0b01011100] = D_Short_MOV_I_W_imm16_Rd<4>;
    t[0b01011101] = D_Short_MOV_I_W_imm16_Rd<5>;
    t[0b01011110] = D_Short_MOV_I_W_imm16_Rd<6>;
    t[0b01011111] = D_Short_MOV_I_W_imm16_Rd<7>;
    t[0b01100000] = D_Short_MOV_L_aa8_Rd<MCU_Operand_Size::BYTE, 0>;
    t[0b01100001] = D_Short_MOV_L_aa8_Rd<MCU_Operand_Size::BYTE, 1>;
    t[0b01100010] = D_Short_MOV_L_aa8_Rd<MCU_Operand_Size::BYTE, 2>;
    t[0b01100011] = D_Short_MOV_L_aa8_Rd<MCU_Operand_Size::BYTE, 3>;
    t[0b01100100] = D_Short_MOV_L_aa8_Rd<MCU_Operand_Size::BYTE, 4>;
    t[0b01100101] = D_Short_MOV_L_aa8_Rd<MCU_Operand_Size::BYTE, 5>;
    t[0b01100110] = D_Short_MOV_L_aa8_Rd<MCU_Operand_Size::BYTE, 6>;
    t[0b01100111] = D_Short_MOV_L_aa8_Rd<MCU_Operand_Size::BYTE, 7>;
    t[0b01101000] = D_Short_MOV_L_aa8_Rd<MCU_Operand_Size::WORD, 0>;
    t[0b01101001] = D_Short_MOV_L_aa8_Rd<MCU_Operand_Size::WORD, 1>;
    t[0b01101010] = D_Short_MOV_L_aa8_Rd<MCU_Operand_Size::WORD, 2>;
    t[0b01101011] = D_Short_MOV_L_aa8_Rd<MCU_Operand_Size::WORD, 3>;
    t[0b01101100] = D_Short_MOV_L_aa8_Rd<MCU_Operand_Size::WORD, 4>;
    t[0b01101101] = D_Short_MOV_L_aa8_Rd<MCU_Operand_Size::WORD, 5>;
    t[0b01101110] = D_Short_MOV_L_aa8_Rd<MCU_Operand_Size::WORD, 6>;
    t[0b01101111] = D_Short_MOV_L_aa8_Rd<MCU_Operand_Size::WORD, 7>;
    t[0b01110000] = D_Short_I_MOV_S_Rs_aa8<MCU_Operand_Size::BYTE, 0>;
    t[0b01110001] = D_Short_I_MOV_S_Rs_aa8<MCU_Operand_Size::BYTE, 1>;
    t[0b01110010] = D_Short_I_MOV_S_Rs_aa8<MCU_Operand_Size::BYTE, 2>;
    t[0b01110011] = D_Short_I_MOV_S_Rs_aa8<MCU_Operand_Size::BYTE, 3>;
    t[0b01110100] = D_Short_I_MOV_S_Rs_aa8<MCU_Operand_Size::BYTE, 4>;
    t[0b01110101] = D_Short_I_MOV_S_Rs_aa8<MCU_Operand_Size::BYTE, 5>;
    t[0b01110110] = D_Short_I_MOV_S_Rs_aa8<MCU_Operand_Size::BYTE, 6>;
    t[0b01110111] = D_Short_I_MOV_S_Rs_aa8<MCU_Operand_Size::BYTE, 7>;
    t[0b01111000] = D_Short_I_MOV_S_Rs_aa8<MCU_Operand_Size::WORD, 0>;
    t[0b01111001] = D_Short_I_MOV_S_Rs_aa8<MCU_Operand_Size::WORD, 1>;
    t[0b01111010] = D_Short_I_MOV_S_Rs_aa8<MCU_Operand_Size::WORD, 2>;
    t[0b01111011] = D_Short_I_MOV_S_Rs_aa8<MCU_Operand_Size::WORD, 3>;
    t[0b01111100] = D_Short_I_MOV_S_Rs_aa8<MCU_Operand_Size::WORD, 4>;
    t[0b01111101] = D_Short_I_MOV_S_Rs_aa8<MCU_Operand_Size::WORD, 5>;
    t[0b01111110] = D_Short_I_MOV_S_Rs_aa8<MCU_Operand_Size::WORD, 6>;
    t[0b01111111] = D_Short_I_MOV_S_Rs_aa8<MCU_Operand_Size::WORD, 7>;
    t[0b10000000] = nullptr;
    t[0b10000001] = nullptr;
    t[0b10000010] = nullptr;
    t[0b10000011] = nullptr;
    t[0b10000100] = nullptr;
    t[0b10000101] = nullptr;
    t[0b10000110] = nullptr;
    t[0b10000111] = nullptr;
    t[0b10001000] = nullptr;
    t[0b10001001] = nullptr;
    t[0b10001010] = nullptr;
    t[0b10001011] = nullptr;
    t[0b10001100] = nullptr;
    t[0b10001101] = nullptr;
    t[0b10001110] = nullptr;
    t[0b10001111] = nullptr;
    t[0b10010000] = nullptr;
    t[0b10010001] = nullptr;
    t[0b10010010] = nullptr;
    t[0b10010011] = nullptr;
    t[0b10010100] = nullptr;
    t[0b10010101] = nullptr;
    t[0b10010110] = nullptr;
    t[0b10010111] = nullptr;
    t[0b10011000] = nullptr;
    t[0b10011001] = nullptr;
    t[0b10011010] = nullptr;
    t[0b10011011] = nullptr;
    t[0b10011100] = nullptr;
    t[0b10011101] = nullptr;
    t[0b10011110] = nullptr;
    t[0b10011111] = nullptr;
    t[0b10100000] = D_General_Rn<MCU_Operand_Size::BYTE, 0>;
    t[0b10100001] = D_General_Rn<MCU_Operand_Size::BYTE, 1>;
    t[0b10100010] = D_General_Rn<MCU_Operand_Size::BYTE, 2>;
    t[0b10100011] = D_General_Rn<MCU_Operand_Size::BYTE, 3>;
    t[0b10100100] = D_General_Rn<MCU_Operand_Size::BYTE, 4>;
    t[0b10100101] = D_General_Rn<MCU_Operand_Size::BYTE, 5>;
    t[0b10100110] = D_General_Rn<MCU_Operand_Size::BYTE, 6>;
    t[0b10100111] = D_General_Rn<MCU_Operand_Size::BYTE, 7>;
    t[0b10101000] = D_General_Rn<MCU_Operand_Size::WORD, 0>;
    t[0b10101001] = D_General_Rn<MCU_Operand_Size::WORD, 1>;
    t[0b10101010] = D_General_Rn<MCU_Operand_Size::WORD, 2>;
    t[0b10101011] = D_General_Rn<MCU_Operand_Size::WORD, 3>;
    t[0b10101100] = D_General_Rn<MCU_Operand_Size::WORD, 4>;
    t[0b10101101] = D_General_Rn<MCU_Operand_Size::WORD, 5>;
    t[0b10101110] = D_General_Rn<MCU_Operand_Size::WORD, 6>;
    t[0b10101111] = D_General_Rn<MCU_Operand_Size::WORD, 7>;
    t[0b10110000] = D_General_APreDecRn<MCU_Operand_Size::BYTE, 0>;
    t[0b10110001] = D_General_APreDecRn<MCU_Operand_Size::BYTE, 1>;
    t[0b10110010] = D_General_APreDecRn<MCU_Operand_Size::BYTE, 2>;
    t[0b10110011] = D_General_APreDecRn<MCU_Operand_Size::BYTE, 3>;
    t[0b10110100] = D_General_APreDecRn<MCU_Operand_Size::BYTE, 4>;
    t[0b10110101] = D_General_APreDecRn<MCU_Operand_Size::BYTE, 5>;
    t[0b10110110] = D_General_APreDecRn<MCU_Operand_Size::BYTE, 6>;
    t[0b10110111] = D_General_APreDecRn<MCU_Operand_Size::BYTE, 7>;
    t[0b10111000] = D_General_APreDecRn<MCU_Operand_Size::WORD, 0>;
    t[0b10111001] = D_General_APreDecRn<MCU_Operand_Size::WORD, 1>;
    t[0b10111010] = D_General_APreDecRn<MCU_Operand_Size::WORD, 2>;
    t[0b10111011] = D_General_APreDecRn<MCU_Operand_Size::WORD, 3>;
    t[0b10111100] = D_General_APreDecRn<MCU_Operand_Size::WORD, 4>;
    t[0b10111101] = D_General_APreDecRn<MCU_Operand_Size::WORD, 5>;
    t[0b10111110] = D_General_APreDecRn<MCU_Operand_Size::WORD, 6>;
    t[0b10111111] = D_General_APreDecRn<MCU_Operand_Size::WORD, 7>;
    t[0b11000000] = D_General_APostIncRn<MCU_Operand_Size::BYTE, 0>;
    t[0b11000001] = D_General_APostIncRn<MCU_Operand_Size::BYTE, 1>;
    t[0b11000010] = D_General_APostIncRn<MCU_Operand_Size::BYTE, 2>;
    t[0b11000011] = D_General_APostIncRn<MCU_Operand_Size::BYTE, 3>;
    t[0b11000100] = D_General_APostIncRn<MCU_Operand_Size::BYTE, 4>;
    t[0b11000101] = D_General_APostIncRn<MCU_Operand_Size::BYTE, 5>;
    t[0b11000110] = D_General_APostIncRn<MCU_Operand_Size::BYTE, 6>;
    t[0b11000111] = D_General_APostIncRn<MCU_Operand_Size::BYTE, 7>;
    t[0b11001000] = D_General_APostIncRn<MCU_Operand_Size::WORD, 0>;
    t[0b11001001] = D_General_APostIncRn<MCU_Operand_Size::WORD, 1>;
    t[0b11001010] = D_General_APostIncRn<MCU_Operand_Size::WORD, 2>;
    t[0b11001011] = D_General_APostIncRn<MCU_Operand_Size::WORD, 3>;
    t[0b11001100] = D_General_APostIncRn<MCU_Operand_Size::WORD, 4>;
    t[0b11001101] = D_General_APostIncRn<MCU_Operand_Size::WORD, 5>;
    t[0b11001110] = D_General_APostIncRn<MCU_Operand_Size::WORD, 6>;
    t[0b11001111] = D_General_APostIncRn<MCU_Operand_Size::WORD, 7>;
    t[0b11010000] = D_General_ARn<MCU_Operand_Size::BYTE, 0>;
    t[0b11010001] = D_General_ARn<MCU_Operand_Size::BYTE, 1>;
    t[0b11010010] = D_General_ARn<MCU_Operand_Size::BYTE, 2>;
    t[0b11010011] = D_General_ARn<MCU_Operand_Size::BYTE, 3>;
    t[0b11010100] = D_General_ARn<MCU_Operand_Size::BYTE, 4>;
    t[0b11010101] = D_General_ARn<MCU_Operand_Size::BYTE, 5>;
    t[0b11010110] = D_General_ARn<MCU_Operand_Size::BYTE, 6>;
    t[0b11010111] = D_General_ARn<MCU_Operand_Size::BYTE, 7>;
    t[0b11011000] = D_General_ARn<MCU_Operand_Size::WORD, 0>;
    t[0b11011001] = D_General_ARn<MCU_Operand_Size::WORD, 1>;
    t[0b11011010] = D_General_ARn<MCU_Operand_Size::WORD, 2>;
    t[0b11011011] = D_General_ARn<MCU_Operand_Size::WORD, 3>;
    t[0b11011100] = D_General_ARn<MCU_Operand_Size::WORD, 4>;
    t[0b11011101] = D_General_ARn<MCU_Operand_Size::WORD, 5>;
    t[0b11011110] = D_General_ARn<MCU_Operand_Size::WORD, 6>;
    t[0b11011111] = D_General_ARn<MCU_Operand_Size::WORD, 7>;
    t[0b11100000] = D_General_Ad8_Rn<MCU_Operand_Size::BYTE, 0>;
    t[0b11100001] = D_General_Ad8_Rn<MCU_Operand_Size::BYTE, 1>;
    t[0b11100010] = D_General_Ad8_Rn<MCU_Operand_Size::BYTE, 2>;
    t[0b11100011] = D_General_Ad8_Rn<MCU_Operand_Size::BYTE, 3>;
    t[0b11100100] = D_General_Ad8_Rn<MCU_Operand_Size::BYTE, 4>;
    t[0b11100101] = D_General_Ad8_Rn<MCU_Operand_Size::BYTE, 5>;
    t[0b11100110] = D_General_Ad8_Rn<MCU_Operand_Size::BYTE, 6>;
    t[0b11100111] = D_General_Ad8_Rn<MCU_Operand_Size::BYTE, 7>;
    t[0b11101000] = D_General_Ad8_Rn<MCU_Operand_Size::WORD, 0>;
    t[0b11101001] = D_General_Ad8_Rn<MCU_Operand_Size::WORD, 1>;
    t[0b11101010] = D_General_Ad8_Rn<MCU_Operand_Size::WORD, 2>;
    t[0b11101011] = D_General_Ad8_Rn<MCU_Operand_Size::WORD, 3>;
    t[0b11101100] = D_General_Ad8_Rn<MCU_Operand_Size::WORD, 4>;
    t[0b11101101] = D_General_Ad8_Rn<MCU_Operand_Size::WORD, 5>;
    t[0b11101110] = D_General_Ad8_Rn<MCU_Operand_Size::WORD, 6>;
    t[0b11101111] = D_General_Ad8_Rn<MCU_Operand_Size::WORD, 7>;
    t[0b11110000] = D_General_Ad16_Rn<MCU_Operand_Size::BYTE, 0>;
    t[0b11110001] = D_General_Ad16_Rn<MCU_Operand_Size::BYTE, 1>;
    t[0b11110010] = D_General_Ad16_Rn<MCU_Operand_Size::BYTE, 2>;
    t[0b11110011] = D_General_Ad16_Rn<MCU_Operand_Size::BYTE, 3>;
    t[0b11110100] = D_General_Ad16_Rn<MCU_Operand_Size::BYTE, 4>;
    t[0b11110101] = D_General_Ad16_Rn<MCU_Operand_Size::BYTE, 5>;
    t[0b11110110] = D_General_Ad16_Rn<MCU_Operand_Size::BYTE, 6>;
    t[0b11110111] = D_General_Ad16_Rn<MCU_Operand_Size::BYTE, 7>;
    t[0b11111000] = D_General_Ad16_Rn<MCU_Operand_Size::WORD, 0>;
    t[0b11111001] = D_General_Ad16_Rn<MCU_Operand_Size::WORD, 1>;
    t[0b11111010] = D_General_Ad16_Rn<MCU_Operand_Size::WORD, 2>;
    t[0b11111011] = D_General_Ad16_Rn<MCU_Operand_Size::WORD, 3>;
    t[0b11111100] = D_General_Ad16_Rn<MCU_Operand_Size::WORD, 4>;
    t[0b11111101] = D_General_Ad16_Rn<MCU_Operand_Size::WORD, 5>;
    t[0b11111110] = D_General_Ad16_Rn<MCU_Operand_Size::WORD, 6>;
    t[0b11111111] = D_General_Ad16_Rn<MCU_Operand_Size::WORD, 7>;
    return t;
}

constexpr std::array<D_Handler, 256> DECODE_TABLE_0 = MakeDecodeTable();

D_Handler GetDispatcherTop(uint8_t byte)
{
    return DECODE_TABLE_0[byte];
}
