#pragma once

#include <cstdint>
#include <source_location>

#include "cache.h"

struct mcu_t;

namespace decoder2
{

using D_Handler       = void (*)(mcu_t& mcu, uint32_t instr_start, uint8_t byte);
using D_OpcodeHandler = void (*)(mcu_t& mcu, uint32_t instr_start, uint8_t byte, I_CachedInstruction instr);

void D_FetchDecodeExecuteNext(mcu_t& mcu);

void DoCache(mcu_t&                     mcu,
             I_InstructionCache&        cache,
             uint32_t                   instr_start,
             I_Handler_Erased_Func      func,
             const I_CachedInstruction& st);

void DoCacheJump(mcu_t& mcu, I_InstructionCache& cache, uint32_t instr_start, I_Handler_Erased_Func func, int16_t disp);

void DoCacheBranch(
    mcu_t& mcu, I_InstructionCache& cache, uint32_t instr_start, I_Handler_Erased_Func func, int16_t disp);

// void D_InvalidInstruction(mcu_t& mcu, uint32_t instr_start, uint8_t byte);
void D_InvalidInstruction(mcu_t& mcu, uint32_t instr_start, uint8_t byte, I_CachedInstruction instr);

// Backtrack and re-try using original decoder
void D_Fallback(mcu_t& mcu);

// Disassembles the instruction at current IP, prints it, and exits process.
[[noreturn]]
void D_HardError(mcu_t&                      mcu,
                 const char*                 message  = nullptr,
                 const std::source_location& location = std::source_location::current());

} // namespace decoder2
