#pragma once

//=============================================================================
// Disassembly utilities
//=============================================================================

#include <cstdint>
#include <span>
#include <string>

enum I_InstructionType
{
    Unknown,
    //-------- general/special form -------------------------------------------
    MOV,
    LDM,
    STM,
    XCH,
    SWAP,
    MOVTPE,
    ADD,
    ADDS,
    ADDX,
    DADD,
    SUB,
    SUBS,
    SUBX,
    DSUB,
    MULXU,
    DIVXU,
    CMP,
    EXTS,
    EXTU,
    TST,
    NEG,
    CLR,
    TAS,
    SHAL,
    SHAR,
    SHLL,
    SHLR,
    ROTL,
    ROTR,
    ROTXL,
    ROTXR,
    AND,
    OR,
    XOR,
    NOT,
    BSET,
    BCLR,
    BTST,
    BNOT,
    LDC,
    STC,
    ANDC,
    ORC,
    XORC,
    //-------- special form only ----------------------------------------------
    BRA,
    BRN,
    BHI,
    BLS,
    BCC,
    BCS,
    BNE,
    BEQ,
    BVC,
    BVS,
    BPL,
    BMI,
    BGE,
    BLT,
    BGT,
    BLE,
    JMP,
    BSR,
    JSR,
    RTS,
    RTD,
    SCB,
    PJMP,
    PJSR,
    PRTS,
    PRTD,
    TRAPA,
    TRAP_VS,
    RTE,
    LINK,
    UNLK,
    SLEEP,
    NOP,
};

enum I_Size
{
    UNSIZED, // e.g. NOP
    BYTE,
    WORD,
};

#include "instructions.h"

using I_RegId        = uint8_t; // range 0..7
using I_ControlRegId = uint8_t; // range 0..7

enum I_OpLocation
{
    NotPresent,
    EA,
    R,
    imm,
    CR,
};

enum class I_Format : uint8_t
{
    NotPresent,
    G,
    E,
    I,
    F,
    L,
    S,
    Q,
};

struct I_DecodedInstruction
{
    I_AddressMode     mode; // only valid when is_general == true
    I_Format          format;
    I_Size            op_size;
    I_InstructionType instr = Unknown;

    I_RegId        ea_reg; // only present for addressing modes that refer to a register
    I_RegId        op_reg; // only present for instructions whose opcodes refer to a register
    I_ControlRegId op_cr;  // only present for instructions whose opcodes refer to a control register
    uint16_t       op_data;

    int16_t  disp; // Ad8_Rn, Ad16_Rn
    uint16_t addr; // Aaa8, Aaa16
    uint16_t imm;  // imm8, imm16
    bool     is_general;
    bool     is_branch; // when true, disp contains the displacement when the branch condition is met

    I_OpLocation op_src; // if present, what the operation sources data from
    I_OpLocation op_dst; // if present, what the operation mutates

    uint8_t instr_size;
};

enum class I_DecoderErrorCode : uint8_t
{
    NoMoreBytes = 1,
    InvalidStartingPosition,
    InvalidGenericJump,
};

struct I_DecoderError
{
    I_DecoderErrorCode code;
    size_t             position;
    const char*        message;
};

class I_Decoder
{
public:
    I_Decoder(std::span<const uint8_t> view, size_t position = 0)
        : m_view(view),
          m_pos(position)
    {
        if (m_pos >= m_view.size())
        {
            SetError({
                .code     = I_DecoderErrorCode::InvalidStartingPosition,
                .position = m_pos,
                .message  = "Initial position is out of bounds",
            });
        }
    }

    I_Decoder(const I_Decoder&)            = delete;
    I_Decoder& operator=(const I_Decoder&) = delete;
    I_Decoder(I_Decoder&&)                 = delete;
    I_Decoder& operator=(I_Decoder&&)      = delete;

    [[nodiscard]]
    uint8_t ReadAdvance()
    {
        if (m_pos < m_view.size())
        {
            const uint8_t byte = m_view[m_pos];
            ++m_pos;
            return byte;
        }
        else
        {
            SetError({
                .code     = I_DecoderErrorCode::NoMoreBytes,
                .position = GetPosition(),
                .message  = "Decoded instruction requires more bytes than were provided",
            });
            return 0xff;
        }
    }

    [[nodiscard]]
    uint16_t ReadU16()
    {
        const uint8_t hi = ReadAdvance();
        const uint8_t lo = ReadAdvance();
        return static_cast<uint16_t>((hi << 8) | lo);
    }

    I_DecoderError GetError() const
    {
        return m_err;
    }

    void SetError(I_DecoderError error)
    {
        m_err = error;
    }

    [[nodiscard]]
    bool HasError() const
    {
        return m_err.code != I_DecoderErrorCode{};
    }

    size_t GetPosition() const
    {
        return m_pos;
    }

private:
    std::span<const uint8_t> m_view;
    size_t                   m_pos = 0;
    I_DecoderError           m_err{};
};

using Decoder_Handler = void (*)(I_Decoder& decoder, uint8_t byte, I_DecodedInstruction& instr);

bool I_Disassemble(std::span<const uint8_t> bytes, size_t position, I_DecodedInstruction& result);

void I_RenderInstruction2(const I_DecodedInstruction& instr, std::string& result);
