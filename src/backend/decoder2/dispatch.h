#pragma once

#include <source_location>

#include "cache.h"

struct mcu_t;

namespace decoder2
{

void FetchDecodeExecuteNext(mcu_t& mcu);

// Backtrack and re-try using original decoder
void Fallback(mcu_t& mcu);

// Disassembles the instruction at current IP, prints it, and exits process.
[[noreturn]]
void FatalError(mcu_t&                      mcu,
                const char*                 message  = nullptr,
                const std::source_location& location = std::source_location::current());

} // namespace decoder2
