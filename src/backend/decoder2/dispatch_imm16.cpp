#include <array>

#include "dispatch.h"
#include "dispatch_handlers.h"

template <MCU_Operand_Size Sz>
struct Size_To_Mode
{
};

template <>
struct Size_To_Mode<MCU_Operand_Size::BYTE>
{
    using Type = Mode_Imm8;
};

template <>
struct Size_To_Mode<MCU_Operand_Size::WORD>
{
    using Type = Mode_Imm16;
};

// In comparison to other addressing modes, the instructions valid for
// immediate modes cannot use EA as a destination. We cannot share
// instructions.
template <MCU_Operand_Size Sz>
constexpr std::array<D_OpcodeHandler, 256> DefineTable()
{
    using Mode = typename Size_To_Mode<Sz>::Type;

    std::array<D_OpcodeHandler, 256> t{};
    t[0b00000000] = nullptr;
    t[0b00000001] = nullptr;
    t[0b00000010] = nullptr;
    t[0b00000011] = nullptr;
    t[0b00000100] = nullptr;
    t[0b00000101] = nullptr;
    t[0b00000110] = nullptr;
    t[0b00000111] = nullptr;
    t[0b00001000] = nullptr;
    t[0b00001001] = nullptr;
    t[0b00001010] = nullptr;
    t[0b00001011] = nullptr;
    t[0b00001100] = nullptr;
    t[0b00001101] = nullptr;
    t[0b00001110] = nullptr;
    t[0b00001111] = nullptr;
    t[0b00010000] = nullptr;
    t[0b00010001] = nullptr;
    t[0b00010010] = nullptr;
    t[0b00010011] = nullptr;
    t[0b00010100] = nullptr;
    t[0b00010101] = nullptr;
    t[0b00010110] = nullptr;
    t[0b00010111] = nullptr;
    t[0b00011000] = nullptr;
    t[0b00011001] = nullptr;
    t[0b00011010] = nullptr;
    t[0b00011011] = nullptr;
    t[0b00011100] = nullptr;
    t[0b00011101] = nullptr;
    t[0b00011110] = nullptr;
    t[0b00011111] = nullptr;
    t[0b00100000] = D_ADD_G_EAs_Rd<Sz, 0, Mode>;
    t[0b00100001] = D_ADD_G_EAs_Rd<Sz, 1, Mode>;
    t[0b00100010] = D_ADD_G_EAs_Rd<Sz, 2, Mode>;
    t[0b00100011] = D_ADD_G_EAs_Rd<Sz, 3, Mode>;
    t[0b00100100] = D_ADD_G_EAs_Rd<Sz, 4, Mode>;
    t[0b00100101] = D_ADD_G_EAs_Rd<Sz, 5, Mode>;
    t[0b00100110] = D_ADD_G_EAs_Rd<Sz, 6, Mode>;
    t[0b00100111] = D_ADD_G_EAs_Rd<Sz, 7, Mode>;
    t[0b00101000] = D_ADDS_EAs_Rd<Sz, 0, Mode>;
    t[0b00101001] = D_ADDS_EAs_Rd<Sz, 1, Mode>;
    t[0b00101010] = D_ADDS_EAs_Rd<Sz, 2, Mode>;
    t[0b00101011] = D_ADDS_EAs_Rd<Sz, 3, Mode>;
    t[0b00101100] = D_ADDS_EAs_Rd<Sz, 4, Mode>;
    t[0b00101101] = D_ADDS_EAs_Rd<Sz, 5, Mode>;
    t[0b00101110] = D_ADDS_EAs_Rd<Sz, 6, Mode>;
    t[0b00101111] = D_ADDS_EAs_Rd<Sz, 7, Mode>;
    t[0b00110000] = D_SUB_EAs_Rd<Sz, 0, Mode>;
    t[0b00110001] = D_SUB_EAs_Rd<Sz, 1, Mode>;
    t[0b00110010] = D_SUB_EAs_Rd<Sz, 2, Mode>;
    t[0b00110011] = D_SUB_EAs_Rd<Sz, 3, Mode>;
    t[0b00110100] = D_SUB_EAs_Rd<Sz, 4, Mode>;
    t[0b00110101] = D_SUB_EAs_Rd<Sz, 5, Mode>;
    t[0b00110110] = D_SUB_EAs_Rd<Sz, 6, Mode>;
    t[0b00110111] = D_SUB_EAs_Rd<Sz, 7, Mode>;
    t[0b00111000] = D_SUBS_EAs_Rd<Sz, 0, Mode>;
    t[0b00111001] = D_SUBS_EAs_Rd<Sz, 1, Mode>;
    t[0b00111010] = D_SUBS_EAs_Rd<Sz, 2, Mode>;
    t[0b00111011] = D_SUBS_EAs_Rd<Sz, 3, Mode>;
    t[0b00111100] = D_SUBS_EAs_Rd<Sz, 4, Mode>;
    t[0b00111101] = D_SUBS_EAs_Rd<Sz, 5, Mode>;
    t[0b00111110] = D_SUBS_EAs_Rd<Sz, 6, Mode>;
    t[0b00111111] = D_SUBS_EAs_Rd<Sz, 7, Mode>;
    t[0b01000000] = D_OR_EAs_Rd<Sz, 0, Mode>;
    t[0b01000001] = D_OR_EAs_Rd<Sz, 1, Mode>;
    t[0b01000010] = D_OR_EAs_Rd<Sz, 2, Mode>;
    t[0b01000011] = D_OR_EAs_Rd<Sz, 3, Mode>;
    t[0b01000100] = D_OR_EAs_Rd<Sz, 4, Mode>;
    t[0b01000101] = D_OR_EAs_Rd<Sz, 5, Mode>;
    t[0b01000110] = D_OR_EAs_Rd<Sz, 6, Mode>;
    t[0b01000111] = D_OR_EAs_Rd<Sz, 7, Mode>;
    t[0b01001000] = D_ORC_immXX_CR<Sz, 0, Mode>;
    t[0b01001001] = D_ORC_immXX_CR<Sz, 1, Mode>;
    t[0b01001010] = D_ORC_immXX_CR<Sz, 2, Mode>;
    t[0b01001011] = D_ORC_immXX_CR<Sz, 3, Mode>;
    t[0b01001100] = D_ORC_immXX_CR<Sz, 4, Mode>;
    t[0b01001101] = D_ORC_immXX_CR<Sz, 5, Mode>;
    t[0b01001110] = D_ORC_immXX_CR<Sz, 6, Mode>;
    t[0b01001111] = D_ORC_immXX_CR<Sz, 7, Mode>;
    t[0b01010000] = D_AND_EAs_Rd<Sz, 0, Mode>;
    t[0b01010001] = D_AND_EAs_Rd<Sz, 1, Mode>;
    t[0b01010010] = D_AND_EAs_Rd<Sz, 2, Mode>;
    t[0b01010011] = D_AND_EAs_Rd<Sz, 3, Mode>;
    t[0b01010100] = D_AND_EAs_Rd<Sz, 4, Mode>;
    t[0b01010101] = D_AND_EAs_Rd<Sz, 5, Mode>;
    t[0b01010110] = D_AND_EAs_Rd<Sz, 6, Mode>;
    t[0b01010111] = D_AND_EAs_Rd<Sz, 7, Mode>;
    t[0b01011000] = D_ANDC_immXX_CR<Sz, 0, Mode>;
    t[0b01011001] = D_ANDC_immXX_CR<Sz, 1, Mode>;
    t[0b01011010] = D_ANDC_immXX_CR<Sz, 2, Mode>;
    t[0b01011011] = D_ANDC_immXX_CR<Sz, 3, Mode>;
    t[0b01011100] = D_ANDC_immXX_CR<Sz, 4, Mode>;
    t[0b01011101] = D_ANDC_immXX_CR<Sz, 5, Mode>;
    t[0b01011110] = D_ANDC_immXX_CR<Sz, 6, Mode>;
    t[0b01011111] = D_ANDC_immXX_CR<Sz, 7, Mode>;
    t[0b01100000] = nullptr;
    t[0b01100001] = nullptr;
    t[0b01100010] = nullptr;
    t[0b01100011] = nullptr;
    t[0b01100100] = nullptr;
    t[0b01100101] = nullptr;
    t[0b01100110] = nullptr;
    t[0b01100111] = nullptr;
    t[0b01101000] = nullptr;
    t[0b01101001] = nullptr;
    t[0b01101010] = nullptr;
    t[0b01101011] = nullptr;
    t[0b01101100] = nullptr;
    t[0b01101101] = nullptr;
    t[0b01101110] = nullptr;
    t[0b01101111] = nullptr;
    t[0b01110000] = nullptr;
    t[0b01110001] = nullptr;
    t[0b01110010] = nullptr;
    t[0b01110011] = nullptr;
    t[0b01110100] = nullptr;
    t[0b01110101] = nullptr;
    t[0b01110110] = nullptr;
    t[0b01110111] = nullptr;
    t[0b01111000] = nullptr;
    t[0b01111001] = nullptr;
    t[0b01111010] = nullptr;
    t[0b01111011] = nullptr;
    t[0b01111100] = nullptr;
    t[0b01111101] = nullptr;
    t[0b01111110] = nullptr;
    t[0b01111111] = nullptr;
    t[0b10000000] = nullptr;
    t[0b10000001] = nullptr;
    t[0b10000010] = nullptr;
    t[0b10000011] = nullptr;
    t[0b10000100] = nullptr;
    t[0b10000101] = nullptr;
    t[0b10000110] = nullptr;
    t[0b10000111] = nullptr;
    t[0b10001000] = D_LDC_EAs_CR<Sz, 0, Mode>;
    t[0b10001001] = D_LDC_EAs_CR<Sz, 1, Mode>;
    t[0b10001010] = D_LDC_EAs_CR<Sz, 2, Mode>;
    t[0b10001011] = D_LDC_EAs_CR<Sz, 3, Mode>;
    t[0b10001100] = D_LDC_EAs_CR<Sz, 4, Mode>;
    t[0b10001101] = D_LDC_EAs_CR<Sz, 5, Mode>;
    t[0b10001110] = D_LDC_EAs_CR<Sz, 6, Mode>;
    t[0b10001111] = D_LDC_EAs_CR<Sz, 7, Mode>;
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
    t[0b10100000] = D_ADDX_EAs_Rd<Sz, 0, Mode>;
    t[0b10100001] = D_ADDX_EAs_Rd<Sz, 1, Mode>;
    t[0b10100010] = D_ADDX_EAs_Rd<Sz, 2, Mode>;
    t[0b10100011] = D_ADDX_EAs_Rd<Sz, 3, Mode>;
    t[0b10100100] = D_ADDX_EAs_Rd<Sz, 4, Mode>;
    t[0b10100101] = D_ADDX_EAs_Rd<Sz, 5, Mode>;
    t[0b10100110] = D_ADDX_EAs_Rd<Sz, 6, Mode>;
    t[0b10100111] = D_ADDX_EAs_Rd<Sz, 7, Mode>;
    t[0b10101000] = D_MULXU_EAs_Rd<Sz, 0, Mode>;
    t[0b10101001] = D_MULXU_EAs_Rd<Sz, 1, Mode>;
    t[0b10101010] = D_MULXU_EAs_Rd<Sz, 2, Mode>;
    t[0b10101011] = D_MULXU_EAs_Rd<Sz, 3, Mode>;
    t[0b10101100] = D_MULXU_EAs_Rd<Sz, 4, Mode>;
    t[0b10101101] = D_MULXU_EAs_Rd<Sz, 5, Mode>;
    t[0b10101110] = D_MULXU_EAs_Rd<Sz, 6, Mode>;
    t[0b10101111] = D_MULXU_EAs_Rd<Sz, 7, Mode>;
    t[0b10110000] = D_SUBX_EAs_Rd<Sz, 0, Mode>;
    t[0b10110001] = D_SUBX_EAs_Rd<Sz, 1, Mode>;
    t[0b10110010] = D_SUBX_EAs_Rd<Sz, 2, Mode>;
    t[0b10110011] = D_SUBX_EAs_Rd<Sz, 3, Mode>;
    t[0b10110100] = D_SUBX_EAs_Rd<Sz, 4, Mode>;
    t[0b10110101] = D_SUBX_EAs_Rd<Sz, 5, Mode>;
    t[0b10110110] = D_SUBX_EAs_Rd<Sz, 6, Mode>;
    t[0b10110111] = D_SUBX_EAs_Rd<Sz, 7, Mode>;
    t[0b10111000] = D_DIVXU_EAs_Rd<Sz, 0, Mode>;
    t[0b10111001] = D_DIVXU_EAs_Rd<Sz, 1, Mode>;
    t[0b10111010] = D_DIVXU_EAs_Rd<Sz, 2, Mode>;
    t[0b10111011] = D_DIVXU_EAs_Rd<Sz, 3, Mode>;
    t[0b10111100] = D_DIVXU_EAs_Rd<Sz, 4, Mode>;
    t[0b10111101] = D_DIVXU_EAs_Rd<Sz, 5, Mode>;
    t[0b10111110] = D_DIVXU_EAs_Rd<Sz, 6, Mode>;
    t[0b10111111] = D_DIVXU_EAs_Rd<Sz, 7, Mode>;
    t[0b11000000] = nullptr;
    t[0b11000001] = nullptr;
    t[0b11000010] = nullptr;
    t[0b11000011] = nullptr;
    t[0b11000100] = nullptr;
    t[0b11000101] = nullptr;
    t[0b11000110] = nullptr;
    t[0b11000111] = nullptr;
    t[0b11001000] = nullptr;
    t[0b11001001] = nullptr;
    t[0b11001010] = nullptr;
    t[0b11001011] = nullptr;
    t[0b11001100] = nullptr;
    t[0b11001101] = nullptr;
    t[0b11001110] = nullptr;
    t[0b11001111] = nullptr;
    t[0b11010000] = nullptr;
    t[0b11010001] = nullptr;
    t[0b11010010] = nullptr;
    t[0b11010011] = nullptr;
    t[0b11010100] = nullptr;
    t[0b11010101] = nullptr;
    t[0b11010110] = nullptr;
    t[0b11010111] = nullptr;
    t[0b11011000] = nullptr;
    t[0b11011001] = nullptr;
    t[0b11011010] = nullptr;
    t[0b11011011] = nullptr;
    t[0b11011100] = nullptr;
    t[0b11011101] = nullptr;
    t[0b11011110] = nullptr;
    t[0b11011111] = nullptr;
    t[0b11100000] = nullptr;
    t[0b11100001] = nullptr;
    t[0b11100010] = nullptr;
    t[0b11100011] = nullptr;
    t[0b11100100] = nullptr;
    t[0b11100101] = nullptr;
    t[0b11100110] = nullptr;
    t[0b11100111] = nullptr;
    t[0b11101000] = nullptr;
    t[0b11101001] = nullptr;
    t[0b11101010] = nullptr;
    t[0b11101011] = nullptr;
    t[0b11101100] = nullptr;
    t[0b11101101] = nullptr;
    t[0b11101110] = nullptr;
    t[0b11101111] = nullptr;
    t[0b11110000] = nullptr;
    t[0b11110001] = nullptr;
    t[0b11110010] = nullptr;
    t[0b11110011] = nullptr;
    t[0b11110100] = nullptr;
    t[0b11110101] = nullptr;
    t[0b11110110] = nullptr;
    t[0b11110111] = nullptr;
    t[0b11111000] = nullptr;
    t[0b11111001] = nullptr;
    t[0b11111010] = nullptr;
    t[0b11111011] = nullptr;
    t[0b11111100] = nullptr;
    t[0b11111101] = nullptr;
    t[0b11111110] = nullptr;
    t[0b11111111] = nullptr;
    return t;
}

constexpr std::array<std::array<D_OpcodeHandler, 256>, 2> DECODE_TABLES{
    {DefineTable<MCU_Operand_Size::BYTE>(), DefineTable<MCU_Operand_Size::WORD>()},
};

D_OpcodeHandler GetDispatcherimm16(uint8_t opcode, MCU_Operand_Size size)
{
    return DECODE_TABLES[(size_t)size][opcode];
}
