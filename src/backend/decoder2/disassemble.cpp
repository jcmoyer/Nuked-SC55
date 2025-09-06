#include "disassemble.h"

#include <utility>

#include "address_modes.h"
#include "decode_address.h"
#include "decode_imm8.h"
#include "decode_rn.h"
#include "decode_short.h"
#include "decoder_handlers.h"

std::string I_RenderFormatSuffix(I_Format format)
{
    switch (format)
    {
    case I_Format::NotPresent:
        return "";
    case I_Format::G:
        return ":G";
    case I_Format::E:
        return ":E";
    case I_Format::I:
        return ":I";
    case I_Format::F:
        return ":F";
    case I_Format::L:
        return ":L";
    case I_Format::S:
        return ":S";
    case I_Format::Q:
        return ":Q";
    }
    std::unreachable();
}

const char* HEX_DIGITS = "0123456789abcdef";

void WriteHexU8(std::string& s, uint8_t val)
{
    uint8_t hi = val >> 4;
    uint8_t lo = val & 0x0f;
    s.push_back(HEX_DIGITS[hi]);
    s.push_back(HEX_DIGITS[lo]);
}

void WriteHexU16(std::string& s, uint16_t val)
{
    WriteHexU8(s, (uint8_t)(val >> 8));
    WriteHexU8(s, (uint8_t)val);
}

void OperandString(I_DecodedInstruction instr, I_OpLocation loc, std::string& result)
{
    result.clear();
    switch (loc)
    {
    case NotPresent:
        break;
    case EA:
        switch (instr.mode)
        {
        case AddressMode::Rn:
            result  = "R";
            result += std::to_string(instr.ea_reg);
            break;
        case AddressMode::ARn:
            result  = "[R";
            result += std::to_string(instr.ea_reg);
            result += "]";
            break;
        case AddressMode::Ad8_Rn:
        case AddressMode::Ad16_Rn:
            result  = "[R";
            result += std::to_string(instr.ea_reg);
            result += "+";
            WriteHexU16(result, static_cast<uint16_t>(instr.disp));
            result += "]";
            break;
        case AddressMode::APreDecRn:
            result  = "[--R";
            result += std::to_string(instr.ea_reg);
            result += "]";
            break;
        case AddressMode::APostIncRn:
            result  = "[R";
            result += std::to_string(instr.ea_reg);
            result += "++]";
            break;
        case AddressMode::Aaa8:
            result = "[";
            WriteHexU8(result, (uint8_t)instr.addr);
            result += ":8]";
            break;
        case AddressMode::Aaa16:
            result = "[";
            WriteHexU16(result, instr.addr);
            result += ":16]";
            break;
        case AddressMode::imm8:
            result = "#";
            WriteHexU8(result, (uint8_t)instr.imm);
            result += ":8";
            break;
        case AddressMode::imm16:
            result = "#";
            WriteHexU16(result, instr.imm);
            result += ":16";
            break;
        }
        break;
    case R:
        result  = "R";
        result += std::to_string(instr.op_reg);
        break;
    case imm:
        result  = "#";
        result += std::to_string(instr.imm);
        break;
    case CR:
        result  = "CR:";
        result += std::to_string(instr.op_cr);
        break;
    }
}

std::string I_RenderSizeSuffix(I_Size size)
{
    switch (size)
    {
    case UNSIZED:
        return "";
    case BYTE:
        return ".B";
    case WORD:
        return ".W";
    }
    return ".?";
}

bool I_DisassembleOpcode(I_Decoder& decoder, uint8_t opcode, I_DecodedInstruction& result)
{
    if (result.is_general)
    {
        switch (result.mode)
        {
        case AddressMode::Rn:
            if (auto handler = GetDecoderRn(opcode); handler)
            {
                handler(decoder, opcode, result);
            }
            break;
        // These addressing modes all share the same instructions
        case AddressMode::Ad8_Rn:
        case AddressMode::Ad16_Rn:
        case AddressMode::Aaa16:
        case AddressMode::Aaa8:
        case AddressMode::ARn:
        case AddressMode::APreDecRn:
        case AddressMode::APostIncRn:
            if (auto handler = GetDecoderGeneric(opcode); handler)
            {
                handler(decoder, opcode, result);
            }
            break;
        // Immediate modes share instructions
        case AddressMode::imm8:
        case AddressMode::imm16:
            if (auto handler = GetDecoderimm8(opcode); handler)
            {
                handler(decoder, opcode, result);
            }
            break;
        }
    }
    else
    {
        if (auto handler = GetDecoderShort(opcode); handler)
        {
            handler(decoder, opcode, result);
        }
    }

    return true;
}

bool I_Disassemble(std::span<const uint8_t> bytes, size_t position, I_DecodedInstruction& result)
{
    result = {};

    I_Decoder decoder(bytes, position);

    const size_t  instr_first = decoder.GetPosition();
    const uint8_t byte        = decoder.ReadAdvance();

    if ((byte & 0b11110000) == 0b10100000)
    {
        result.mode       = AddressMode::Rn;
        result.op_size    = (byte & 0b00001000) ? WORD : BYTE;
        result.is_general = true;
        result.ea_reg     = byte & 0b111;
    }
    else if ((byte & 0b11110000) == 0b11010000)
    {
        result.mode       = AddressMode::ARn;
        result.op_size    = (byte & 0b00001000) ? WORD : BYTE;
        result.is_general = true;
        result.ea_reg     = byte & 0b111;
    }
    else if ((byte & 0b11110000) == 0b11100000)
    {
        result.op_size    = (byte & 0b00001000) ? WORD : BYTE;
        result.mode       = AddressMode::Ad8_Rn;
        result.disp       = (int8_t)decoder.ReadAdvance();
        result.is_general = true;
        result.ea_reg     = byte & 0b111;
    }
    else if ((byte & 0b11110000) == 0b11110000)
    {
        result.op_size    = (byte & 0b00001000) ? WORD : BYTE;
        result.mode       = AddressMode::Ad16_Rn;
        result.disp       = (int16_t)decoder.ReadU16();
        result.is_general = true;
        result.ea_reg     = byte & 0b111;
    }
    else if ((byte & 0b11110000) == 0b10110000)
    {
        result.op_size    = (byte & 0b00001000) ? WORD : BYTE;
        result.mode       = AddressMode::APreDecRn;
        result.is_general = true;
        result.ea_reg     = byte & 0b111;
    }
    else if ((byte & 0b11110000) == 0b11000000)
    {
        result.op_size    = (byte & 0b00001000) ? WORD : BYTE;
        result.mode       = AddressMode::APostIncRn;
        result.is_general = true;
        result.ea_reg     = byte & 0b111;
    }
    else if ((byte & 0b11110111) == 0b00000101)
    {
        result.op_size    = (byte & 0b00001000) ? WORD : BYTE;
        result.mode       = AddressMode::Aaa8;
        result.addr       = decoder.ReadAdvance();
        result.is_general = true;
    }
    else if ((byte & 0b11110111) == 0b00010101)
    {
        result.op_size    = (byte & 0b00001000) ? WORD : BYTE;
        result.mode       = AddressMode::Aaa16;
        result.addr       = decoder.ReadU16();
        result.is_general = true;
    }
    else if (byte == 0b00000100)
    {
        result.op_size    = BYTE;
        result.mode       = AddressMode::imm8;
        result.imm        = decoder.ReadAdvance();
        result.is_general = true;
    }
    else if (byte == 0b00001100)
    {
        result.op_size    = WORD;
        result.mode       = AddressMode::imm16;
        result.imm        = decoder.ReadU16();
        result.is_general = true;
    }
    else
    {
        result.is_general = false;
    }

    bool success;
    if (result.is_general)
    {
        success = I_DisassembleOpcode(decoder, decoder.ReadAdvance(), result);
    }
    else
    {
        success = I_DisassembleOpcode(decoder, byte, result);
    }

    const size_t instr_last = decoder.GetPosition();
    result.instr_size       = (uint8_t)(instr_last - instr_first);

    return success;
}

const char* ToCString(I_InstructionType instr)
{
    switch (instr)
    {
    case Unknown:
        return "<unknown>";
    case MOV:
        return "MOV";
    case LDM:
        return "LDM";
    case STM:
        return "STM";
    case XCH:
        return "XCH";
    case SWAP:
        return "SWAP";
    case MOVTPE:
        return "MOVTPE";
    case ADD:
        return "ADD";
    case ADDS:
        return "ADDS";
    case ADDX:
        return "ADDX";
    case DADD:
        return "DADD";
    case SUB:
        return "SUB";
    case SUBS:
        return "SUBS";
    case SUBX:
        return "SUBX";
    case DSUB:
        return "DSUB";
    case MULXU:
        return "MULXU";
    case DIVXU:
        return "DIVXU";
    case CMP:
        return "CMP";
    case EXTS:
        return "EXTS";
    case EXTU:
        return "EXTU";
    case TST:
        return "TST";
    case NEG:
        return "NEG";
    case CLR:
        return "CLR";
    case TAS:
        return "TAS";
    case SHAL:
        return "SHAL";
    case SHAR:
        return "SHAR";
    case SHLL:
        return "SHLL";
    case SHLR:
        return "SHLR";
    case ROTL:
        return "ROTL";
    case ROTR:
        return "ROTR";
    case ROTXL:
        return "ROTXL";
    case ROTXR:
        return "ROTXR";
    case AND:
        return "AND";
    case OR:
        return "OR";
    case XOR:
        return "XOR";
    case NOT:
        return "NOT";
    case BSET:
        return "BSET";
    case BCLR:
        return "BCLR";
    case BTST:
        return "BTST";
    case BNOT:
        return "BNOT";
    case LDC:
        return "LDC";
    case STC:
        return "STC";
    case ANDC:
        return "ANDC";
    case ORC:
        return "ORC";
    case XORC:
        return "XORC";
    case BRA:
        return "BRA";
    case BRN:
        return "BRN";
    case BHI:
        return "BHI";
    case BLS:
        return "BLS";
    case BCC:
        return "BCC";
    case BCS:
        return "BCS";
    case BNE:
        return "BNE";
    case BEQ:
        return "BEQ";
    case BVC:
        return "BVC";
    case BVS:
        return "BVS";
    case BPL:
        return "BPL";
    case BMI:
        return "BMI";
    case BGE:
        return "BGE";
    case BLT:
        return "BLT";
    case BGT:
        return "BGT";
    case BLE:
        return "BLE";
    case JMP:
        return "JMP";
    case BSR:
        return "BSR";
    case JSR:
        return "JSR";
    case RTS:
        return "RTS";
    case RTD:
        return "RTD";
    case SCB_F:
        return "SCB/F";
    case SCB_NE:
        return "SCB/NE";
    case SCB_EQ:
        return "SCB/EQ";
    case PJMP:
        return "PJMP";
    case PJSR:
        return "PJSR";
    case PRTS:
        return "PRTS";
    case PRTD:
        return "PRTD";
    case TRAPA:
        return "TRAPA";
    case TRAP_VS:
        return "TRAP_VS";
    case RTE:
        return "RTE";
    case LINK:
        return "LINK";
    case UNLK:
        return "UNLK";
    case SLEEP:
        return "SLEEP";
    case NOP:
        return "NOP";
    }
    std::unreachable();
}

void I_RenderInstruction2(const I_DecodedInstruction& instr, std::string& result)
{
    result.clear();
    // if (instr.is_general)
    // {
    //     result += ToCString(instr.mode);
    //     result += " ";
    // }
    // else
    // {
    //     result += "Sh ";
    // }
    result += ToCString(instr.instr);
    result += I_RenderFormatSuffix(instr.format);
    result += I_RenderSizeSuffix(instr.op_size);

    std::string op_str;

    if (instr.op_src != NotPresent)
    {
        result += " ";
        OperandString(instr, instr.op_src, op_str);
        result += op_str;
    }

    if (instr.op_dst != NotPresent)
    {
        if (instr.op_src != NotPresent)
        {
            result += ",";
        }
        result += " ";
        OperandString(instr, instr.op_dst, op_str);
        result += op_str;
    }
}
