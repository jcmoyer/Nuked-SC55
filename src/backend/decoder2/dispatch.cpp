#include "dispatch.h"

#include <string>

#include "decoder2/cache.h"
#include "decoder2/code_reader.h"
#include "decoder2/disassemble.h"
#include "decoder2/string_util.h"
#include "diagnostics.h"
#include "disassemble.h"
#include "dispatchers.h"
#include "mcu.h"

namespace decoder2
{

struct DecodeResult
{
    // address where decoding started
    uint32_t decode_first;
    // address one past the end of the decoded data
    uint32_t decode_last;

    // decoded instruction, only valid if decoding succeeded
    CachedInstruction instruction;
};

const char* ToCString(DecodeError err)
{
    switch (err)
    {
    case DecodeError::UnrecognizedInstruction:
        return "unrecognized instruction";
    case DecodeError::NeedMoreBytes:
        return "need more bytes";
    }
    return "unknown";
}

// Backtrack and retry using original decoder
void Fallback(mcu_t& mcu)
{
    const uint8_t byte = MCU_ReadCodeAdvance(mcu);
    MCU_Operand_Table[byte](mcu, byte);
}

static void PrintDecodeError(mcu_t& mcu, DecodeError error, DecodeResult& result)
{
    constexpr size_t MAX_INSTR_SIZE = 6;

    std::string msg;
    msg  = "Dispatcher: at address range %02x:%04x ~ %02x:%04x:\n";
    msg += "    Reason: %s\n";
    msg += "    Bytes:  ";

    uint8_t bytes[MAX_INSTR_SIZE]{};
    uint8_t bytei = 0;

    const uint8_t first_page = static_cast<uint8_t>(result.decode_first >> 16);

    uint8_t prev_page = first_page;

    for (uint32_t addr = result.decode_first; addr < result.decode_first + MAX_INSTR_SIZE; ++addr)
    {
        const uint8_t addr_page = static_cast<uint8_t>(addr >> 16);
        if (addr_page < 16)
        {
            const uint8_t byte = MCU_Read(mcu, addr);
            bytes[bytei]       = byte;

            WriteHexU8(msg, byte);
            if (prev_page == addr_page)
            {
                msg += ' ';
            }
            else
            {
                msg += '|';
            }

            ++bytei;
        }
        else
        {
            msg += "xx ";
        }

        prev_page = addr_page;
    }
    msg += "\n";
    msg.append(12 + 3 * (result.decode_last - result.decode_first) - 1, ' ');
    msg += "^~ stopped decoding here\n";

    DisassembledInstruction dis;
    if (Disassemble(bytes, 0, dis))
    {
        std::string rendered_instr;
        RenderInstruction(dis, rendered_instr);
        msg += "    Disassembly: " + rendered_instr + " (may be incorrect)";
    }
    else
    {
        msg += "    no disassembly";
    }

    Diag_Printf(Diag_Category::Error,
                msg.c_str(),
                result.decode_first >> 16,
                (uint16_t)result.decode_first,
                result.decode_last >> 16,
                (uint16_t)result.decode_last,
                ToCString(error));
}

static DecodeError FetchDecode(mcu_t& mcu, DecodeResult& out_result)
{
    CodeReader reader(mcu);
    ReadError  read_err;
    uint8_t    byte;

    reader.SetReadAddress(mcu.cp, mcu.pc);
    out_result.decode_first = reader.GetReadAddress();

    read_err = reader.ReadU8(byte);
    if (read_err != ReadError{})
    {
        out_result.decode_last = reader.GetReadAddress();
        return DecodeError::NeedMoreBytes;
    }

    Dispatcher handler = GetDispatcherTop(byte);
    if (handler)
    {
        CachedInstruction instr{};
        DecodeError       d_error;

        d_error                = (*handler)(reader, byte, instr);
        out_result.decode_last = reader.GetReadAddress();

        if (d_error != DecodeError{})
        {
            return d_error;
        }

        out_result.instruction = instr;
    }
    else
    {
        out_result.decode_last = reader.GetReadAddress();
        return DecodeError::UnrecognizedInstruction;
    }

    return DecodeError{};
}

void FetchDecodeExecuteNext(mcu_t& mcu)
{
    uint32_t instr_start = MCU_GetAddress(mcu.cp, mcu.pc);

    if (const CachedInstruction& instr = mcu.icache.Lookup(instr_start); instr.handler)
    {
        instr.handler(mcu, instr.params);
        return;
    }

    DecodeResult decode_result;
    DecodeError  err = FetchDecode(mcu, decode_result);
    if (err != DecodeError{})
    {
        PrintDecodeError(mcu, err, decode_result);
        exit(1);
    }

    mcu.icache.Write(instr_start, decode_result.instruction);
    decode_result.instruction.handler(mcu, decode_result.instruction.params);
}

} // namespace decoder2
