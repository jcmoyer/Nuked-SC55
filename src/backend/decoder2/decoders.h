#pragma once

#include "disassemble.h"

namespace decoder2
{

// Rn
Decoder_Handler GetDecoderRn(uint8_t byte);

// @Rn, @(d:8,Rn), @(d:16,Rn), @-Rn, @Rn+, @aa:8, @aa:16
Decoder_Handler GetDecoderGeneric(uint8_t byte);

// #xx:8
Decoder_Handler GetDecoderimm8(uint8_t byte);

// #xx:16
Decoder_Handler GetDecoderimm16(uint8_t byte);

// Short-form instructions
Decoder_Handler GetDecoderShort(uint8_t byte);

} // namespace decoder2
