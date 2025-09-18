#include "dispatch.h"

#include "address_modes.h"
#include "dispatch_aa.h"
#include "dispatch_arn.h"
#include "dispatch_d8d16_rn.h"
#include "dispatch_imm16.h"
#include "dispatch_postinc_rn.h"
#include "dispatch_predec_rn.h"
#include "dispatch_rn.h"
#include "instruction_handlers.h"
#include "mcu.h"

///////////////////////////////////////////////////////////////////////////////
#include <algorithm>
#include <string>
#include <unordered_map>

#include "disassemble.h"

#define INSTRUCTION_HIT_TRACING 0

#if INSTRUCTION_HIT_TRACING
#include <vector>
#endif

void WriteBin(std::string& s, uint8_t x)
{
    for (int mask = 0x80; mask; mask >>= 1)
    {
        s.push_back((x & mask) ? '1' : '0');
    }
}

void WriteHit(mcu_t& mcu, std::string& s, uint32_t addr)
{
    uint8_t  page  = (uint8_t)(addr >> 16);
    uint16_t paddr = (uint16_t)addr;
    uint8_t  bytes[6]{};

    // max instruction length is 6; disassembler will only read as many bytes as necessary
    for (int i = 0; i < 6; ++i)
    {
        bytes[i] = MCU_Read(mcu, MCU_GetAddress(page, (uint16_t)(paddr + i)));
    }

    I_DecodedInstruction instr;
    I_Disassemble(bytes, 0, instr);

    std::string result;
    for (int i = 0; i < instr.instr_size; ++i)
    {
        WriteBin(result, bytes[i]);
        result.push_back(' ');
    }
    result.resize(54, ' ');
    result.push_back('|');
    result.push_back(' ');
    std::string instr_render;
    I_RenderInstruction2(instr, instr_render);
    result += instr_render;
    s      += result;
}

void PrintHitCount(mcu_t& mcu, uint32_t addr, uint64_t count)
{
    std::string buf;
    WriteHit(mcu, buf, addr);
    buf.resize(86, ' ');
    buf += std::to_string(count);
    fprintf(stderr, "%s\n", buf.c_str());
}

std::unordered_map<uint32_t, uint64_t> hitcount;

///////////////////////////////////////////////////////////////////////////////

// Backtrack and re-try using original decoder
void D_Fallback(mcu_t& mcu)
{
    mcu.pc = mcu.restore_pc;
    mcu.cp = mcu.restore_cp;

    // original decoder does not use coder
    const uint8_t byte = MCU_ReadCodeAdvance(mcu);
    MCU_Operand_Table[byte](mcu, byte);
}

void D_HardError(mcu_t& mcu, const char* help_context)
{
    const uint32_t base_addr = MCU_GetAddress(mcu.cp, mcu.pc);

    const uint8_t bytes[6]{
        MCU_Read(mcu, base_addr + 0),
        MCU_Read(mcu, base_addr + 1),
        MCU_Read(mcu, base_addr + 2),
        MCU_Read(mcu, base_addr + 3),
        MCU_Read(mcu, base_addr + 4),
        MCU_Read(mcu, base_addr + 5),
    };

    fprintf(stderr, "Dispatcher: in %s, unrecognized instruction at %d:%x:\n", help_context, mcu.cp, mcu.pc);

    I_DecodedInstruction decoded;
    if (!I_Disassemble(bytes, 0, decoded))
    {
        fprintf(stderr, "    failed to disassemble instruction\n");
        return;
    }

    std::string code;
    I_RenderInstruction2(decoded, code);
    fprintf(stderr, "    %s\n", code.c_str());

    exit(1);
}

void D_NOP(mcu_t& mcu, uint32_t instr_start, uint8_t byte)
{
    (void)byte;
    mcu.icache.DoCache(mcu, instr_start, I_NOP, {});
}

void D_RTE(mcu_t& mcu, uint32_t instr_start, uint8_t byte)
{
    (void)byte;
    mcu.icache.DoCacheBranch(mcu, instr_start, I_RTE, 0);
}

inline uint8_t I_InstrSizeFromPC(const mcu_t& mcu, uint32_t instr_start)
{
    return (uint8_t)(mcu.pc - instr_start);
}

template <uint8_t Rn>
void D_Short_CMP_E_imm8_Rd(mcu_t& mcu, uint32_t instr_start, uint8_t byte)
{
    (void)byte;
    I_CachedInstruction instr;
    instr.op_data = mcu.coder.ReadU8(mcu);
    instr.ea_reg  = Rn;
    mcu.icache.DoCache(mcu, instr_start, I_CMP_E_imm8_Rd<Rn>, instr);
}

template <uint8_t Rn>
void D_Short_CMP_I_W_imm16_Rd(mcu_t& mcu, uint32_t instr_start, uint8_t byte)
{
    (void)byte;
    I_CachedInstruction instr;
    instr.op_data = mcu.coder.ReadU16(mcu);
    instr.ea_reg  = Rn;
    mcu.icache.DoCache(mcu, instr_start, I_CMP_I_W_imm16_Rd<Rn>, instr);
}

template <uint8_t Rn>
void D_Short_MOV_E_imm8_Rd(mcu_t& mcu, uint32_t instr_start, uint8_t byte)
{
    (void)byte;
    I_CachedInstruction instr;
    instr.op_data = mcu.coder.ReadU8(mcu);
    instr.ea_reg  = Rn;
    mcu.icache.DoCache(mcu, instr_start, I_MOV_E_imm8_Rd<Rn>, instr);
}

template <MCU_Operand_Size Sz, uint8_t Rn>
void D_Short_MOV_L_aa8_Rd(mcu_t& mcu, uint32_t instr_start, uint8_t byte)
{
    (void)byte;
    I_CachedInstruction instr;
    instr.ea_data = mcu.coder.ReadU8(mcu);
    instr.op_reg  = Rn;

    if constexpr (Sz == MCU_Operand_Size::BYTE)
    {
        mcu.icache.DoCache(mcu, instr_start, I_MOV_L_B_aa8_Rd<Rn>, instr);
    }
    else if constexpr (Sz == MCU_Operand_Size::WORD)
    {
        mcu.icache.DoCache(mcu, instr_start, I_MOV_L_W_aa8_Rd<Rn>, instr);
    }
}

template <uint8_t Rn>
void D_Short_MOV_I_W_imm16_Rd(mcu_t& mcu, uint32_t instr_start, uint8_t byte)
{
    (void)byte;
    I_CachedInstruction instr;
    instr.op_data = mcu.coder.ReadU16(mcu);
    instr.ea_reg  = Rn;
    mcu.icache.DoCache(mcu, instr_start, I_MOV_I_W_imm16_Rd<Rn>, instr);
}

template <MCU_Operand_Size Sz, uint8_t Rn>
void D_Short_I_MOV_S_Rs_aa8(mcu_t& mcu, uint32_t instr_start, uint8_t byte)
{
    (void)byte;
    I_CachedInstruction instr;
    instr.op_reg  = Rn;
    instr.ea_data = mcu.coder.ReadU8(mcu);
    if constexpr (Sz == MCU_Operand_Size::BYTE)
    {
        mcu.icache.DoCache(mcu, instr_start, I_MOV_S_B_Rs_aa8<Rn>, instr);
    }
    else if constexpr (Sz == MCU_Operand_Size::WORD)
    {
        mcu.icache.DoCache(mcu, instr_start, I_MOV_S_W_Rs_aa8<Rn>, instr);
    }
}

void D_Short_SCB(mcu_t& mcu, uint32_t instr_start, uint8_t byte)
{
    const uint8_t regcode = mcu.coder.ReadU8(mcu);
    const int8_t  disp    = (int8_t)mcu.coder.ReadU8(mcu);

    if (byte == 0b00000001)
    {
        switch (regcode)
        {
        case 0b10111000:
            mcu.icache.DoCacheBranch(mcu, instr_start, I_SCB_F<0>, disp);
            break;
        case 0b10111001:
            mcu.icache.DoCacheBranch(mcu, instr_start, I_SCB_F<1>, disp);
            break;
        case 0b10111010:
            mcu.icache.DoCacheBranch(mcu, instr_start, I_SCB_F<2>, disp);
            break;
        case 0b10111011:
            mcu.icache.DoCacheBranch(mcu, instr_start, I_SCB_F<3>, disp);
            break;
        case 0b10111100:
            mcu.icache.DoCacheBranch(mcu, instr_start, I_SCB_F<4>, disp);
            break;
        case 0b10111101:
            mcu.icache.DoCacheBranch(mcu, instr_start, I_SCB_F<5>, disp);
            break;
        case 0b10111110:
            mcu.icache.DoCacheBranch(mcu, instr_start, I_SCB_F<6>, disp);
            break;
        case 0b10111111:
            mcu.icache.DoCacheBranch(mcu, instr_start, I_SCB_F<7>, disp);
            break;
        default:
            D_HardError(mcu, "SCB/F invalid regcode");
        }
    }
    else if (byte == 0b00000110)
    {
        switch (regcode)
        {
        case 0b10111000:
            mcu.icache.DoCacheBranch(mcu, instr_start, I_SCB_NE<0>, disp);
            break;
        case 0b10111001:
            mcu.icache.DoCacheBranch(mcu, instr_start, I_SCB_NE<1>, disp);
            break;
        case 0b10111010:
            mcu.icache.DoCacheBranch(mcu, instr_start, I_SCB_NE<2>, disp);
            break;
        case 0b10111011:
            mcu.icache.DoCacheBranch(mcu, instr_start, I_SCB_NE<3>, disp);
            break;
        case 0b10111100:
            mcu.icache.DoCacheBranch(mcu, instr_start, I_SCB_NE<4>, disp);
            break;
        case 0b10111101:
            mcu.icache.DoCacheBranch(mcu, instr_start, I_SCB_NE<5>, disp);
            break;
        case 0b10111110:
            mcu.icache.DoCacheBranch(mcu, instr_start, I_SCB_NE<6>, disp);
            break;
        case 0b10111111:
            mcu.icache.DoCacheBranch(mcu, instr_start, I_SCB_NE<7>, disp);
            break;
        default:
            D_HardError(mcu, "SCB/NE invalid regcode");
        }
    }
    else if (byte == 0b00000111)
    {
        switch (regcode)
        {
        case 0b10111000:
            mcu.icache.DoCacheBranch(mcu, instr_start, I_SCB_EQ<0>, disp);
            break;
        case 0b10111001:
            mcu.icache.DoCacheBranch(mcu, instr_start, I_SCB_EQ<1>, disp);
            break;
        case 0b10111010:
            mcu.icache.DoCacheBranch(mcu, instr_start, I_SCB_EQ<2>, disp);
            break;
        case 0b10111011:
            mcu.icache.DoCacheBranch(mcu, instr_start, I_SCB_EQ<3>, disp);
            break;
        case 0b10111100:
            mcu.icache.DoCacheBranch(mcu, instr_start, I_SCB_EQ<4>, disp);
            break;
        case 0b10111101:
            mcu.icache.DoCacheBranch(mcu, instr_start, I_SCB_EQ<5>, disp);
            break;
        case 0b10111110:
            mcu.icache.DoCacheBranch(mcu, instr_start, I_SCB_EQ<6>, disp);
            break;
        case 0b10111111:
            mcu.icache.DoCacheBranch(mcu, instr_start, I_SCB_EQ<7>, disp);
            break;
        default:
            D_HardError(mcu, "SCB/EQ invalid regcode");
        }
    }
    else
    {
        D_HardError(mcu, "D_Short_SCB not implemented");
    }
}

void D_Short_RTS(mcu_t& mcu, uint32_t instr_start, uint8_t byte)
{
    (void)byte;
    mcu.icache.DoCache(mcu, instr_start, I_RTS, {});
}

void D_Short_PRTS(mcu_t& mcu, uint32_t instr_start, uint8_t byte)
{
    (void)byte;
    mcu.icache.DoCache(mcu, instr_start, I_PRTS, {});
}

void D_JMP_ARn(mcu_t& mcu, uint32_t instr_start, uint8_t byte)
{
    I_CachedInstruction instr;
    instr.op_reg = byte & 0b111;
    mcu.icache.DoCache(mcu, instr_start, I_JMP_ARn, instr);
}

void D_Short_PJMP_aa24(mcu_t& mcu, uint32_t instr_start, uint8_t byte)
{
    (void)byte;
    I_CachedInstruction instr;
    instr.op_page = mcu.coder.ReadU8(mcu);
    instr.op_data = mcu.coder.ReadU16(mcu);
    mcu.icache.DoCache(mcu, instr_start, I_PJMP_aa24, instr);
}

void D_Short_PJSR_aa24(mcu_t& mcu, uint32_t instr_start, uint8_t byte)
{
    (void)byte;
    I_CachedInstruction instr;
    instr.op_page = mcu.coder.ReadU8(mcu);
    instr.op_data = mcu.coder.ReadU16(mcu);
    instr.br_true = (uint16_t)(instr_start + mcu.coder.offset); // TODO: awkward use of br_true as return address
    mcu.icache.DoCache(mcu, instr_start, I_PJSR_aa24, instr);
}

void D_Short_PJMP_ARn(mcu_t& mcu, uint32_t instr_start, uint8_t byte)
{
    I_CachedInstruction instr;
    instr.op_reg = byte & 0b111;
    mcu.icache.DoCache(mcu, instr_start, I_PJMP_ARn, instr);
}

void D_Short_JMP_aa16(mcu_t& mcu, uint32_t instr_start, uint8_t byte)
{
    (void)byte;
    I_CachedInstruction instr;
    instr.br_true = mcu.coder.ReadU16(mcu);
    mcu.icache.DoCache(mcu, instr_start, I_JMP_aa16, instr);
}

void D_Short_JSR_aa16(mcu_t& mcu, uint32_t instr_start, uint8_t byte)
{
    (void)byte;
    I_CachedInstruction instr;
    instr.br_true  = mcu.coder.ReadU16(mcu);
    instr.br_false = (uint16_t)(instr_start + mcu.coder.offset);
    mcu.icache.DoCache(mcu, instr_start, I_JSR_aa16, instr);
}

void D_Short_JSR_ARn(mcu_t& mcu, uint32_t instr_start, uint8_t byte)
{
    I_CachedInstruction instr;
    instr.op_reg   = byte & 0b111;
    instr.br_false = (uint16_t)(instr_start + mcu.coder.offset);
    mcu.icache.DoCache(mcu, instr_start, I_JSR_ARn, instr);
}

void D_JMP(mcu_t& mcu, uint32_t instr_start, uint8_t byte)
{
    (void)instr_start;
    (void)byte;

    const uint8_t kind = mcu.coder.ReadU8(mcu);

    // TODO - not all kinds implemented
    switch (kind & 0b11111000)
    {
    // JMP @Rn
    case 0b11010000:
        D_JMP_ARn(mcu, instr_start, kind);
        break;

    // PJMP @Rn
    case 0b11000000:
        D_Short_PJMP_ARn(mcu, instr_start, kind);
        break;

    // JSR @Rn
    case 0b11011000:
        D_Short_JSR_ARn(mcu, instr_start, kind);
        break;

    default:
        if (kind == 0b00011001)
        {
            D_Short_PRTS(mcu, instr_start, kind);
        }
        else
        {
            D_HardError(mcu, "D_JMP");
        }
    }
}

void D_Short_TRAPA(mcu_t& mcu, uint32_t instr_start, uint8_t byte)
{
    (void)byte;

    const uint8_t vec_byte = mcu.coder.ReadU8(mcu);

    I_CachedInstruction instr;
    instr.op_data = vec_byte & 0b1111;
    mcu.icache.DoCache(mcu, instr_start, I_TRAPA_imm4, instr);
}

void D_Short_SLEEP(mcu_t& mcu, uint32_t instr_start, uint8_t byte)
{
    (void)byte;
    mcu.icache.DoCache(mcu, instr_start, I_SLEEP, {});
}

void D_Short_STM(mcu_t& mcu, uint32_t instr_start, uint8_t byte)
{
    (void)byte;
    const uint8_t reglist = mcu.coder.ReadU8(mcu);
    if (reglist == 127)
    {
        // specialize the most commonly used form of this instruction
        mcu.icache.DoCache(mcu, instr_start, I_STM_Fast<127>, {});
    }
    else
    {
        I_CachedInstruction instr;
        instr.op_data = reglist;
        mcu.icache.DoCache(mcu, instr_start, I_STM, instr);
    }
}

void D_Short_LDM(mcu_t& mcu, uint32_t instr_start, uint8_t byte)
{
    (void)byte;
    const uint8_t reglist = mcu.coder.ReadU8(mcu);
    if (reglist == 127)
    {
        // specialize the most commonly used form of this instruction
        mcu.icache.DoCache(mcu, instr_start, I_LDM_Fast<127>, {});
    }
    else
    {
        I_CachedInstruction instr;
        instr.op_data = reglist;
        mcu.icache.DoCache(mcu, instr_start, I_LDM, instr);
    }
}

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

// Handles both d:8 and d:16 forms
template <MCU_Operand_Size Sz>
void I_Bcc(mcu_t& mcu, uint32_t instr_start, uint8_t opcode)
{
    const uint8_t cond = opcode & 0b1111;
    int16_t       disp;
    switch (Sz)
    {
    case MCU_Operand_Size::BYTE:
        disp = (int8_t)mcu.coder.ReadU8(mcu);
        break;
    case MCU_Operand_Size::WORD:
        disp = (int16_t)mcu.coder.ReadU16(mcu);
        break;
    }
    switch (cond)
    {
    case 0:
        mcu.icache.DoCacheBranch(mcu, instr_start, I_Bcc_d8_BRA, disp);
        break;
    case 1:
        mcu.icache.DoCacheBranch(mcu, instr_start, I_Bcc_d8_BRN, disp);
        break;
    case 2:
        mcu.icache.DoCacheBranch(mcu, instr_start, I_Bcc_d8_BHI, disp);
        break;
    case 3:
        mcu.icache.DoCacheBranch(mcu, instr_start, I_Bcc_d8_BLS, disp);
        break;
    case 4:
        mcu.icache.DoCacheBranch(mcu, instr_start, I_Bcc_d8_BCC, disp);
        break;
    case 5:
        mcu.icache.DoCacheBranch(mcu, instr_start, I_Bcc_d8_BCS, disp);
        break;
    case 6:
        mcu.icache.DoCacheBranch(mcu, instr_start, I_Bcc_d8_BNE, disp);
        break;
    case 7:
        mcu.icache.DoCacheBranch(mcu, instr_start, I_Bcc_d8_BEQ, disp);
        break;
    case 8:
        mcu.icache.DoCacheBranch(mcu, instr_start, I_Bcc_d8_BVC, disp);
        break;
    case 9:
        mcu.icache.DoCacheBranch(mcu, instr_start, I_Bcc_d8_BVS, disp);
        break;
    case 10:
        mcu.icache.DoCacheBranch(mcu, instr_start, I_Bcc_d8_BPL, disp);
        break;
    case 11:
        mcu.icache.DoCacheBranch(mcu, instr_start, I_Bcc_d8_BMI, disp);
        break;
    case 12:
        mcu.icache.DoCacheBranch(mcu, instr_start, I_Bcc_d8_BGE, disp);
        break;
    case 13:
        mcu.icache.DoCacheBranch(mcu, instr_start, I_Bcc_d8_BLT, disp);
        break;
    case 14:
        mcu.icache.DoCacheBranch(mcu, instr_start, I_Bcc_d8_BGT, disp);
        break;
    case 15:
        mcu.icache.DoCacheBranch(mcu, instr_start, I_Bcc_d8_BLE, disp);
        break;
    default:
        std::unreachable();
    }
}

void I_BSR(mcu_t& mcu, const I_CachedInstruction& instr)
{
    MCU_PushStack(mcu, instr.br_false);
    mcu.pc = instr.br_true;
}

void D_BSR_d8(mcu_t& mcu, uint32_t instr_start, uint8_t opcode)
{
    (void)opcode;
    int8_t disp = (int8_t)mcu.coder.ReadU8(mcu);
    mcu.icache.DoCacheBranch(mcu, instr_start, I_BSR, disp);
}

void D_BSR_d16(mcu_t& mcu, uint32_t instr_start, uint8_t opcode)
{
    (void)opcode;
    const uint16_t disp = mcu.coder.ReadU16(mcu);
    mcu.icache.DoCacheBranch(mcu, instr_start, I_BSR, (int16_t)disp);
}

//=============================================================================
//
//=============================================================================
// Top level table for decoder. Here we can determine addressing mode or short
// form instruction.
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
    I_Bcc<MCU_Operand_Size::BYTE>,                     // 00100000
    I_Bcc<MCU_Operand_Size::BYTE>,                     // 00100001
    I_Bcc<MCU_Operand_Size::BYTE>,                     // 00100010
    I_Bcc<MCU_Operand_Size::BYTE>,                     // 00100011
    I_Bcc<MCU_Operand_Size::BYTE>,                     // 00100100
    I_Bcc<MCU_Operand_Size::BYTE>,                     // 00100101
    I_Bcc<MCU_Operand_Size::BYTE>,                     // 00100110
    I_Bcc<MCU_Operand_Size::BYTE>,                     // 00100111
    I_Bcc<MCU_Operand_Size::BYTE>,                     // 00101000
    I_Bcc<MCU_Operand_Size::BYTE>,                     // 00101001
    I_Bcc<MCU_Operand_Size::BYTE>,                     // 00101010
    I_Bcc<MCU_Operand_Size::BYTE>,                     // 00101011
    I_Bcc<MCU_Operand_Size::BYTE>,                     // 00101100
    I_Bcc<MCU_Operand_Size::BYTE>,                     // 00101101
    I_Bcc<MCU_Operand_Size::BYTE>,                     // 00101110
    I_Bcc<MCU_Operand_Size::BYTE>,                     // 00101111
    I_Bcc<MCU_Operand_Size::WORD>,                     // 00110000
    I_Bcc<MCU_Operand_Size::WORD>,                     // 00110001
    I_Bcc<MCU_Operand_Size::WORD>,                     // 00110010
    I_Bcc<MCU_Operand_Size::WORD>,                     // 00110011
    I_Bcc<MCU_Operand_Size::WORD>,                     // 00110100
    I_Bcc<MCU_Operand_Size::WORD>,                     // 00110101
    I_Bcc<MCU_Operand_Size::WORD>,                     // 00110110
    I_Bcc<MCU_Operand_Size::WORD>,                     // 00110111
    I_Bcc<MCU_Operand_Size::WORD>,                     // 00111000
    I_Bcc<MCU_Operand_Size::WORD>,                     // 00111001
    I_Bcc<MCU_Operand_Size::WORD>,                     // 00111010
    I_Bcc<MCU_Operand_Size::WORD>,                     // 00111011
    I_Bcc<MCU_Operand_Size::WORD>,                     // 00111100
    I_Bcc<MCU_Operand_Size::WORD>,                     // 00111101
    I_Bcc<MCU_Operand_Size::WORD>,                     // 00111110
    I_Bcc<MCU_Operand_Size::WORD>,                     // 00111111
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

void D_FetchDecodeExecuteNext(mcu_t& mcu)
{
    mcu.restore_pc = mcu.pc;
    mcu.restore_cp = mcu.cp;

    uint32_t instr_start = MCU_GetAddress(mcu.cp, mcu.pc);
    mcu.coder            = CodeReader{};

    if (const I_Handler& handler = mcu.icache.Lookup(instr_start); handler.F)
    {
        handler.F(mcu, handler.instr);
        return;
    }

#if INSTRUCTION_HIT_TRACING
    ++hitcount[instr_start];
#endif

    const uint8_t byte = mcu.coder.ReadU8(mcu);

    D_Handler handler = DECODE_TABLE_0[byte];
    if (handler)
    {
        (*handler)(mcu, instr_start, byte);
    }
    else
    {
        D_HardError(mcu, "D_FetchDecodeExecuteNext");
    }

#if INSTRUCTION_HIT_TRACING
    if (mcu.cycles >= 1000000000)
    {
        std::vector<std::pair<uint32_t, uint64_t>> hits(hitcount.begin(), hitcount.end());
        std::sort(hits.begin(), hits.end(), [](auto& a, auto& b) { return a.second < b.second; });

        for (auto& kvp : hits)
        {
            if (!mcu.icache.Contains(kvp.first))
            {
                PrintHitCount(mcu, kvp.first, kvp.second);
            }
        }

        fprintf(stderr, "total cached: %zu\n", mcu.icache.CountCached());
        exit(0);
    }
#endif
}

void D_InvalidInstruction(mcu_t& mcu, uint32_t instr_start, uint8_t byte, I_CachedInstruction instr)
{
    (void)instr_start;
    (void)byte;
    (void)instr;
    D_HardError(mcu, "D_InvalidInstruction");
}
