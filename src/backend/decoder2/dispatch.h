#pragma once

#include <cstdint>

struct mcu_t;

namespace decoder2
{

enum class DecodeError : uint8_t
{
    // there is no decoder for the provided byte pattern
    UnrecognizedInstruction = 1,
    // there isn't enough data to decode an entire instruction, e.g. there is an
    // EA field but no opcode
    NeedMoreBytes,
};

void FetchDecodeExecuteNext(mcu_t& mcu);

// Backtrack and re-try using original decoder
void Fallback(mcu_t& mcu);

} // namespace decoder2
