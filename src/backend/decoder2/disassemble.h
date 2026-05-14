#pragma once

//=============================================================================
// Disassembly utilities
//=============================================================================

#include <cstdint>
#include <span>
#include <string>

#include "address_modes.h"

namespace decoder2
{

enum class InstructionType
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
    SCB_F,
    SCB_NE,
    SCB_EQ,
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

enum class OptionalSize
{
    Unsized, // e.g. NOP
    Byte,
    Word,
};

using RegisterId        = uint8_t; // range 0..7
using ControlRegisterId = uint8_t; // range 0..7

enum class OperandLocation
{
    NotPresent,
    EA,
    R,
    imm,
    CR,
};

enum class InstructionFormat : uint8_t
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

struct InstructionOperand
{
    OperandLocation location;

    // when location: EA, none of these fields are used
    union {
        RegisterId        reg; // location: R
        ControlRegisterId cr;  // location: CR
        uint16_t          imm; // location: imm
    };

    int16_t disp;
};

struct DisassembledInstruction
{
    AddressMode       mode; // only valid when is_general == true
    InstructionFormat format;
    OptionalSize      op_size;
    InstructionType   instr = InstructionType::Unknown;

    RegisterId ea_reg; // only present for addressing modes that refer to a register

    int16_t  ea_disp; // Ad8_Rn, Ad16_Rn
    uint16_t ea_addr; // Aaa8, Aaa16
    uint16_t ea_imm;  // imm8, imm16
    bool     is_general;
    bool     is_branch; // when true, disp contains the displacement when the branch condition is met

    // These are the operands of the instruction, indexed left-to-right. Usage
    // varies by instruction, but most often 0 is source and 1 is destination.
    InstructionOperand op[2];

    uint8_t instr_size;
};

enum class DisassembleErrorCode : uint8_t
{
    NoMoreBytes = 1,
    InvalidStartingPosition,
    InvalidGenericJump,
    InvalidInstructionFormat,
};

struct DisassembleError
{
    DisassembleErrorCode code;
    size_t               position;
    const char*          message;
};

class DisassembleDecoder
{
public:
    DisassembleDecoder(std::span<const uint8_t> view, size_t position = 0)
        : m_view(view),
          m_pos(position)
    {
        if (m_pos >= m_view.size())
        {
            SetError({
                .code     = DisassembleErrorCode::InvalidStartingPosition,
                .position = m_pos,
                .message  = "Initial position is out of bounds",
            });
        }
    }

    DisassembleDecoder(const DisassembleDecoder&)            = delete;
    DisassembleDecoder& operator=(const DisassembleDecoder&) = delete;
    DisassembleDecoder(DisassembleDecoder&&)                 = delete;
    DisassembleDecoder& operator=(DisassembleDecoder&&)      = delete;

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
                .code     = DisassembleErrorCode::NoMoreBytes,
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

    DisassembleError GetError() const
    {
        return m_err;
    }

    void SetError(DisassembleError error)
    {
        m_err = error;
    }

    [[nodiscard]]
    bool HasError() const
    {
        return m_err.code != DisassembleErrorCode{};
    }

    size_t GetPosition() const
    {
        return m_pos;
    }

private:
    std::span<const uint8_t> m_view;
    size_t                   m_pos = 0;
    DisassembleError         m_err{};
};

using Disassembler = void (*)(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);

bool Disassemble(std::span<const uint8_t> bytes, size_t position, DisassembledInstruction& result);

void RenderInstruction(const DisassembledInstruction& instr, std::string& result);

} // namespace decoder2
