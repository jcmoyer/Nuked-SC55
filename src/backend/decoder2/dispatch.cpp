#include "dispatch.h"

#include "diagnostics.h"
#include "dispatchers.h"
#include "mcu.h"

///////////////////////////////////////////////////////////////////////////////
#include <string>
#include <unordered_map>

#include "disassemble.h"

#define INSTRUCTION_HIT_TRACING 0

#if INSTRUCTION_HIT_TRACING
#include <algorithm>
#include <vector>
#endif

namespace decoder2
{

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
    Diag_Printf(Diag_Category::Debug, "%s\n", buf.c_str());
}

std::unordered_map<uint32_t, uint64_t> hitcount;

///////////////////////////////////////////////////////////////////////////////

// Backtrack and re-try using original decoder
void Fallback(mcu_t& mcu)
{
    // original decoder does not use coder
    const uint8_t byte = MCU_ReadCodeAdvance(mcu);
    MCU_Operand_Table[byte](mcu, byte);
}

void FatalError(mcu_t& mcu, const char* message, const std::source_location& location)
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

    Diag_Printf(
        Diag_Category::Debug, "Dispatcher: in %s, at address %d:%x:\n", location.function_name(), mcu.cp, mcu.pc);
    if (message)
    {
        Diag_Printf(Diag_Category::Debug, "    error: %s\n", message);
    }

    I_DecodedInstruction decoded;
    if (I_Disassemble(bytes, 0, decoded))
    {
        std::string code;
        I_RenderInstruction2(decoded, code);
        Diag_Printf(Diag_Category::Debug, "    %s\n", code.c_str());
    }
    else
    {
        Diag_Printf(Diag_Category::Debug, "    failed to disassemble instruction\n");
    }

    exit(1);
}

void FetchDecodeExecuteNext(mcu_t& mcu)
{
    uint32_t instr_start = MCU_GetAddress(mcu.cp, mcu.pc);

    if (const CachedInstruction& instr = mcu.icache.Lookup(instr_start); instr.handler)
    {
        instr.handler(mcu, instr.params);
        return;
    }

#if INSTRUCTION_HIT_TRACING
    ++hitcount[instr_start];
#endif

    mcu.coder          = CodeReader{};
    const uint8_t byte = mcu.coder.ReadU8(mcu);

    D_Handler handler = GetDispatcherTop(byte);
    if (handler)
    {
        (*handler)(mcu, instr_start, byte);
    }
    else
    {
        FatalError(mcu);
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

        Diag_Printf(Diag_Category::Debug, "total cached: %zu\n", mcu.icache.CountCached());
        exit(0);
    }
#endif
}

void DoCache(mcu_t&                          mcu,
             InstructionCache&               cache,
             uint32_t                        instr_start,
             CachedInstructionHandler        func,
             const DecodedInstructionParams& st)
{
    cache.Write(instr_start, {.handler = func, .params = st});
    func(mcu, st);
}

void DoCacheJump(mcu_t& mcu, InstructionCache& cache, uint32_t instr_start, CachedInstructionHandler func, int16_t disp)
{
    const uint16_t next_ip = mcu.coder.GetAddressInPage(mcu);

    DecodedInstructionParams st;
    st.br_true  = (uint16_t)(next_ip + disp);
    st.br_false = (uint16_t)(next_ip + disp);
    cache.Write(instr_start, {.handler = func, .params = st});
    func(mcu, st);
}

void DoCacheBranch(
    mcu_t& mcu, InstructionCache& cache, uint32_t instr_start, CachedInstructionHandler func, int16_t disp)
{
    const uint16_t next_ip = mcu.coder.GetAddressInPage(mcu);

    DecodedInstructionParams st;
    st.br_true  = (uint16_t)(next_ip + disp);
    st.br_false = next_ip;
    cache.Write(instr_start, {.handler = func, .params = st});
    func(mcu, st);
}

} // namespace decoder2
