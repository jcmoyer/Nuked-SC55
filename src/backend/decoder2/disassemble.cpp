#include "disassemble.h"

#include <utility>

#include "address_modes.h"
#include "decoder_handlers.h"
#include "decoders.h"

namespace decoder2
{

std::string RenderFormatSuffix(InstructionFormat format)
{
    switch (format)
    {
    case InstructionFormat::NotPresent:
        return "";
    case InstructionFormat::G:
        return ":G";
    case InstructionFormat::E:
        return ":E";
    case InstructionFormat::I:
        return ":I";
    case InstructionFormat::F:
        return ":F";
    case InstructionFormat::L:
        return ":L";
    case InstructionFormat::S:
        return ":S";
    case InstructionFormat::Q:
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

void OperandString(DisassembledInstruction instr, InstructionOperand op, std::string& result)
{
    result.clear();
    switch (op.location)
    {
    case OperandLocation::NotPresent:
        break;
    case OperandLocation::EA:
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
            WriteHexU16(result, static_cast<uint16_t>(instr.ea_disp));
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
            WriteHexU8(result, (uint8_t)instr.ea_addr);
            result += ":8]";
            break;
        case AddressMode::Aaa16:
            result = "[";
            WriteHexU16(result, instr.ea_addr);
            result += ":16]";
            break;
        case AddressMode::imm8:
            result = "#";
            WriteHexU8(result, (uint8_t)instr.ea_imm);
            result += ":8";
            break;
        case AddressMode::imm16:
            result = "#";
            WriteHexU16(result, instr.ea_imm);
            result += ":16";
            break;
        }
        break;
    case OperandLocation::R:
        result  = "R";
        result += std::to_string(op.reg);
        break;
    case OperandLocation::imm:
        result  = "#";
        result += std::to_string(op.imm);
        break;
    case OperandLocation::CR:
        result  = "CR:";
        result += std::to_string(op.cr);
        break;
    }
}

std::string RenderSizeSuffix(OptionalSize size)
{
    switch (size)
    {
    case OptionalSize::Unsized:
        return "";
    case OptionalSize::Byte:
        return ".B";
    case OptionalSize::Word:
        return ".W";
    }
    return ".?";
}

bool DisassembleOpcode(DisassembleDecoder& decoder, uint8_t opcode, DisassembledInstruction& result)
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

bool Disassemble(std::span<const uint8_t> bytes, size_t position, DisassembledInstruction& result)
{
    result = {};

    DisassembleDecoder decoder(bytes, position);

    const size_t  instr_first = decoder.GetPosition();
    const uint8_t byte        = decoder.ReadAdvance();

    if ((byte & 0b11110000) == 0b10100000)
    {
        result.mode       = AddressMode::Rn;
        result.op_size    = (byte & 0b00001000) ? OptionalSize::Word : OptionalSize::Byte;
        result.is_general = true;
        result.ea_reg     = byte & 0b111;
    }
    else if ((byte & 0b11110000) == 0b11010000)
    {
        result.mode       = AddressMode::ARn;
        result.op_size    = (byte & 0b00001000) ? OptionalSize::Word : OptionalSize::Byte;
        result.is_general = true;
        result.ea_reg     = byte & 0b111;
    }
    else if ((byte & 0b11110000) == 0b11100000)
    {
        result.op_size    = (byte & 0b00001000) ? OptionalSize::Word : OptionalSize::Byte;
        result.mode       = AddressMode::Ad8_Rn;
        result.ea_disp    = (int8_t)decoder.ReadAdvance();
        result.is_general = true;
        result.ea_reg     = byte & 0b111;
    }
    else if ((byte & 0b11110000) == 0b11110000)
    {
        result.op_size    = (byte & 0b00001000) ? OptionalSize::Word : OptionalSize::Byte;
        result.mode       = AddressMode::Ad16_Rn;
        result.ea_disp    = (int16_t)decoder.ReadU16();
        result.is_general = true;
        result.ea_reg     = byte & 0b111;
    }
    else if ((byte & 0b11110000) == 0b10110000)
    {
        result.op_size    = (byte & 0b00001000) ? OptionalSize::Word : OptionalSize::Byte;
        result.mode       = AddressMode::APreDecRn;
        result.is_general = true;
        result.ea_reg     = byte & 0b111;
    }
    else if ((byte & 0b11110000) == 0b11000000)
    {
        result.op_size    = (byte & 0b00001000) ? OptionalSize::Word : OptionalSize::Byte;
        result.mode       = AddressMode::APostIncRn;
        result.is_general = true;
        result.ea_reg     = byte & 0b111;
    }
    else if ((byte & 0b11110111) == 0b00000101)
    {
        result.op_size    = (byte & 0b00001000) ? OptionalSize::Word : OptionalSize::Byte;
        result.mode       = AddressMode::Aaa8;
        result.ea_addr    = decoder.ReadAdvance();
        result.is_general = true;
    }
    else if ((byte & 0b11110111) == 0b00010101)
    {
        result.op_size    = (byte & 0b00001000) ? OptionalSize::Word : OptionalSize::Byte;
        result.mode       = AddressMode::Aaa16;
        result.ea_addr    = decoder.ReadU16();
        result.is_general = true;
    }
    else if (byte == 0b00000100)
    {
        result.op_size    = OptionalSize::Byte;
        result.mode       = AddressMode::imm8;
        result.ea_imm     = decoder.ReadAdvance();
        result.is_general = true;
    }
    else if (byte == 0b00001100)
    {
        result.op_size    = OptionalSize::Word;
        result.mode       = AddressMode::imm16;
        result.ea_imm     = decoder.ReadU16();
        result.is_general = true;
    }
    else
    {
        result.is_general = false;
    }

    bool success;
    if (result.is_general)
    {
        success = DisassembleOpcode(decoder, decoder.ReadAdvance(), result);
    }
    else
    {
        success = DisassembleOpcode(decoder, byte, result);
    }

    const size_t instr_last = decoder.GetPosition();
    result.instr_size       = (uint8_t)(instr_last - instr_first);

    return success;
}

const char* ToCString(InstructionType instr)
{
    switch (instr)
    {
    case InstructionType::Unknown:
        return "<unknown>";
    case InstructionType::MOV:
        return "MOV";
    case InstructionType::LDM:
        return "LDM";
    case InstructionType::STM:
        return "STM";
    case InstructionType::XCH:
        return "XCH";
    case InstructionType::SWAP:
        return "SWAP";
    case InstructionType::MOVTPE:
        return "MOVTPE";
    case InstructionType::ADD:
        return "ADD";
    case InstructionType::ADDS:
        return "ADDS";
    case InstructionType::ADDX:
        return "ADDX";
    case InstructionType::DADD:
        return "DADD";
    case InstructionType::SUB:
        return "SUB";
    case InstructionType::SUBS:
        return "SUBS";
    case InstructionType::SUBX:
        return "SUBX";
    case InstructionType::DSUB:
        return "DSUB";
    case InstructionType::MULXU:
        return "MULXU";
    case InstructionType::DIVXU:
        return "DIVXU";
    case InstructionType::CMP:
        return "CMP";
    case InstructionType::EXTS:
        return "EXTS";
    case InstructionType::EXTU:
        return "EXTU";
    case InstructionType::TST:
        return "TST";
    case InstructionType::NEG:
        return "NEG";
    case InstructionType::CLR:
        return "CLR";
    case InstructionType::TAS:
        return "TAS";
    case InstructionType::SHAL:
        return "SHAL";
    case InstructionType::SHAR:
        return "SHAR";
    case InstructionType::SHLL:
        return "SHLL";
    case InstructionType::SHLR:
        return "SHLR";
    case InstructionType::ROTL:
        return "ROTL";
    case InstructionType::ROTR:
        return "ROTR";
    case InstructionType::ROTXL:
        return "ROTXL";
    case InstructionType::ROTXR:
        return "ROTXR";
    case InstructionType::AND:
        return "AND";
    case InstructionType::OR:
        return "OR";
    case InstructionType::XOR:
        return "XOR";
    case InstructionType::NOT:
        return "NOT";
    case InstructionType::BSET:
        return "BSET";
    case InstructionType::BCLR:
        return "BCLR";
    case InstructionType::BTST:
        return "BTST";
    case InstructionType::BNOT:
        return "BNOT";
    case InstructionType::LDC:
        return "LDC";
    case InstructionType::STC:
        return "STC";
    case InstructionType::ANDC:
        return "ANDC";
    case InstructionType::ORC:
        return "ORC";
    case InstructionType::XORC:
        return "XORC";
    case InstructionType::BRA:
        return "BRA";
    case InstructionType::BRN:
        return "BRN";
    case InstructionType::BHI:
        return "BHI";
    case InstructionType::BLS:
        return "BLS";
    case InstructionType::BCC:
        return "BCC";
    case InstructionType::BCS:
        return "BCS";
    case InstructionType::BNE:
        return "BNE";
    case InstructionType::BEQ:
        return "BEQ";
    case InstructionType::BVC:
        return "BVC";
    case InstructionType::BVS:
        return "BVS";
    case InstructionType::BPL:
        return "BPL";
    case InstructionType::BMI:
        return "BMI";
    case InstructionType::BGE:
        return "BGE";
    case InstructionType::BLT:
        return "BLT";
    case InstructionType::BGT:
        return "BGT";
    case InstructionType::BLE:
        return "BLE";
    case InstructionType::JMP:
        return "JMP";
    case InstructionType::BSR:
        return "BSR";
    case InstructionType::JSR:
        return "JSR";
    case InstructionType::RTS:
        return "RTS";
    case InstructionType::RTD:
        return "RTD";
    case InstructionType::SCB_F:
        return "SCB/F";
    case InstructionType::SCB_NE:
        return "SCB/NE";
    case InstructionType::SCB_EQ:
        return "SCB/EQ";
    case InstructionType::PJMP:
        return "PJMP";
    case InstructionType::PJSR:
        return "PJSR";
    case InstructionType::PRTS:
        return "PRTS";
    case InstructionType::PRTD:
        return "PRTD";
    case InstructionType::TRAPA:
        return "TRAPA";
    case InstructionType::TRAP_VS:
        return "TRAP_VS";
    case InstructionType::RTE:
        return "RTE";
    case InstructionType::LINK:
        return "LINK";
    case InstructionType::UNLK:
        return "UNLK";
    case InstructionType::SLEEP:
        return "SLEEP";
    case InstructionType::NOP:
        return "NOP";
    }
    std::unreachable();
}

void RenderInstruction(const DisassembledInstruction& instr, std::string& result)
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
    result += RenderFormatSuffix(instr.format);
    result += RenderSizeSuffix(instr.op_size);

    std::string op_str;

    if (instr.op[0].location != OperandLocation::NotPresent)
    {
        result += " ";
        OperandString(instr, instr.op[0], op_str);
        result += op_str;
    }

    if (instr.op[1].location != OperandLocation::NotPresent)
    {
        if (instr.op[0].location != OperandLocation::NotPresent)
        {
            result += ",";
        }
        result += " ";
        OperandString(instr, instr.op[1], op_str);
        result += op_str;
    }
}

} // namespace decoder2
