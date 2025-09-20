#include "dispatch.h"

#include "dispatch_top.h"
#include "mcu.h"

///////////////////////////////////////////////////////////////////////////////
#include <string>
#include <unordered_map>

#include "disassemble.h"

#define INSTRUCTION_HIT_TRACING 0

#if INSTRUCTION_HIT_TRACING
#include <vector>
#endif

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
    fprintf(stderr, "%s\n", buf.c_str());
}

std::unordered_map<uint32_t, uint64_t> hitcount;

///////////////////////////////////////////////////////////////////////////////

// Backtrack and re-try using original decoder
void D_Fallback(mcu_t& mcu)
{
    // original decoder does not use coder
    const uint8_t byte = MCU_ReadCodeAdvance(mcu);
    MCU_Operand_Table[byte](mcu, byte);
}

void D_HardError(mcu_t& mcu, const char* help_context)
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

    fprintf(stderr, "Dispatcher: in %s, unrecognized instruction at %d:%x:\n", help_context, mcu.cp, mcu.pc);

    I_DecodedInstruction decoded;
    if (I_Disassemble(bytes, 0, decoded))
    {
        std::string code;
        I_RenderInstruction2(decoded, code);
        fprintf(stderr, "    %s\n", code.c_str());
    }
    else
    {
        fprintf(stderr, "    failed to disassemble instruction\n");
    }

    exit(1);
}

void D_FetchDecodeExecuteNext(mcu_t& mcu)
{
    uint32_t instr_start = MCU_GetAddress(mcu.cp, mcu.pc);
    mcu.coder            = CodeReader{};

    if (const I_Handler& handler = mcu.icache.Lookup(instr_start); handler.F)
    {
        handler.F(mcu, handler.instr);
        return;
    }

#if INSTRUCTION_HIT_TRACING
    ++hitcount[instr_start];
#endif

    const uint8_t byte = mcu.coder.ReadU8(mcu);

    D_Handler handler = GetDispatcherTop(byte);
    if (handler)
    {
        (*handler)(mcu, instr_start, byte);
    }
    else
    {
        D_HardError(mcu, "D_FetchDecodeExecuteNext");
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

        fprintf(stderr, "total cached: %zu\n", mcu.icache.CountCached());
        exit(0);
    }
#endif
}

void D_InvalidInstruction(mcu_t& mcu, uint32_t instr_start, uint8_t byte, I_CachedInstruction instr)
{
    (void)instr_start;
    (void)byte;
    (void)instr;
    D_HardError(mcu, "D_InvalidInstruction");
}
