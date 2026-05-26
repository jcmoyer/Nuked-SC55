#include "disassemble.h"

#include <utility>

#include "address_modes.h"
#include "decoder2/string_util.h"
#include "decoder_handlers.h"
#include "decoders.h"

namespace decoder2
{

static std::string RenderFormatSuffix(InstructionFormat format)
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

static void OperandString(DisassembledInstruction instr, InstructionOperand op, std::string& result)
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

static std::string RenderSizeSuffix(OptionalSize size)
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

bool Disassemble(std::span<const uint8_t> bytes, size_t position, DisassembledInstruction& result)
{
    result = {};

    DisassembleDecoder decoder(bytes, position);

    const size_t  instr_first = decoder.GetPosition();
    const uint8_t byte        = decoder.ReadAdvance();

    if (auto handler = GetDecoderTop(byte); handler)
    {
        handler(decoder, byte, result);

        const size_t instr_last = decoder.GetPosition();
        result.instr_size       = (uint8_t)(instr_last - instr_first);
    }
    else
    {
        decoder.SetError({
            .code     = DisassembleErrorCode::InvalidInstructionFormat,
            .position = instr_first,
            .message  = "No top handler",
        });
    }

    // TODO: propagate error
    return decoder.GetError().code == DisassembleErrorCode{};
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
