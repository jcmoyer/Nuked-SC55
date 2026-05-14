#pragma once

#include "disassemble.h"

namespace decoder2
{

using Disassembler = void (*)(DisassembleDecoder& decoder, uint8_t byte, DisassembledInstruction& instr);

// Rn
Disassembler GetDecoderRn(uint8_t byte);

// @Rn, @(d:8,Rn), @(d:16,Rn), @-Rn, @Rn+, @aa:8, @aa:16
Disassembler GetDecoderGeneric(uint8_t byte);

// #xx:8
Disassembler GetDecoderimm8(uint8_t byte);

// #xx:16
Disassembler GetDecoderimm16(uint8_t byte);

// Short-form instructions
Disassembler GetDecoderTop(uint8_t byte);

} // namespace decoder2
