#pragma once

#include <cstdint>

#include "cache.h"

struct mcu_t;

using D_Handler       = void (*)(mcu_t& mcu, uint32_t instr_start, uint8_t byte);
using D_OpcodeHandler = void (*)(mcu_t& mcu, uint32_t instr_start, uint8_t byte, I_CachedInstruction instr);

void D_FetchDecodeExecuteNext(mcu_t& mcu);

// void D_InvalidInstruction(mcu_t& mcu, uint32_t instr_start, uint8_t byte);
void D_InvalidInstruction(mcu_t& mcu, uint32_t instr_start, uint8_t byte, I_CachedInstruction instr);

// Backtrack and re-try using original decoder
void D_Fallback(mcu_t& mcu);

// Disassembles the instruction at current IP, prints it, and exits process.
[[noreturn]]
void D_HardError(mcu_t& mcu, const char* help_context);
