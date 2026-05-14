#include "instruction_handlers.h"

#include "mcu.h"

namespace decoder2
{

void I_NOP(mcu_t& mcu, const DecodedInstructionParams&)
{
    ++mcu.pc;
}

void I_BRA(mcu_t& mcu, const DecodedInstructionParams& st)
{
    mcu.pc = st.br_true;
}

void I_BRN(mcu_t& mcu, const DecodedInstructionParams& st)
{
    mcu.pc = st.br_false;
}

void I_BHI(mcu_t& mcu, const DecodedInstructionParams& st)
{
    const bool C = mcu.sr & STATUS_C;
    const bool Z = mcu.sr & STATUS_Z;
    if ((C || Z) == false)
    {
        mcu.pc = st.br_true;
    }
    else
    {
        mcu.pc = st.br_false;
    }
}

void I_BLS(mcu_t& mcu, const DecodedInstructionParams& st)
{
    const bool C = mcu.sr & STATUS_C;
    const bool Z = mcu.sr & STATUS_Z;
    if ((C || Z) == true)
    {
        mcu.pc = st.br_true;
    }
    else
    {
        mcu.pc = st.br_false;
    }
}

void I_BCC(mcu_t& mcu, const DecodedInstructionParams& st)
{
    const bool C = mcu.sr & STATUS_C;
    if (C == false)
    {
        mcu.pc = st.br_true;
    }
    else
    {
        mcu.pc = st.br_false;
    }
}

void I_BCS(mcu_t& mcu, const DecodedInstructionParams& st)
{
    const bool C = mcu.sr & STATUS_C;
    if (C == true)
    {
        mcu.pc = st.br_true;
    }
    else
    {
        mcu.pc = st.br_false;
    }
}

void I_BNE(mcu_t& mcu, const DecodedInstructionParams& st)
{
    const bool Z = mcu.sr & STATUS_Z;
    if (Z == false)
    {
        mcu.pc = st.br_true;
    }
    else
    {
        mcu.pc = st.br_false;
    }
}

void I_BEQ(mcu_t& mcu, const DecodedInstructionParams& st)
{
    const bool Z = mcu.sr & STATUS_Z;
    if (Z == true)
    {
        mcu.pc = st.br_true;
    }
    else
    {
        mcu.pc = st.br_false;
    }
}

void I_BVC(mcu_t& mcu, const DecodedInstructionParams& st)
{
    const bool V = mcu.sr & STATUS_V;
    if (V == false)
    {
        mcu.pc = st.br_true;
    }
    else
    {
        mcu.pc = st.br_false;
    }
}

void I_BVS(mcu_t& mcu, const DecodedInstructionParams& st)
{
    const bool V = mcu.sr & STATUS_V;
    if (V == true)
    {
        mcu.pc = st.br_true;
    }
    else
    {
        mcu.pc = st.br_false;
    }
}

void I_BPL(mcu_t& mcu, const DecodedInstructionParams& st)
{
    const bool N = mcu.sr & STATUS_N;
    if (N == false)
    {
        mcu.pc = st.br_true;
    }
    else
    {
        mcu.pc = st.br_false;
    }
}

void I_BMI(mcu_t& mcu, const DecodedInstructionParams& st)
{
    const bool N = mcu.sr & STATUS_N;
    if (N == true)
    {
        mcu.pc = st.br_true;
    }
    else
    {
        mcu.pc = st.br_false;
    }
}

void I_BGE(mcu_t& mcu, const DecodedInstructionParams& st)
{
    const bool N = mcu.sr & STATUS_N;
    const bool V = mcu.sr & STATUS_V;
    if ((N ^ V) == false)
    {
        mcu.pc = st.br_true;
    }
    else
    {
        mcu.pc = st.br_false;
    }
}

void I_BLT(mcu_t& mcu, const DecodedInstructionParams& st)
{
    const bool N = mcu.sr & STATUS_N;
    const bool V = mcu.sr & STATUS_V;
    if ((N ^ V) == true)
    {
        mcu.pc = st.br_true;
    }
    else
    {
        mcu.pc = st.br_false;
    }
}

void I_BGT(mcu_t& mcu, const DecodedInstructionParams& st)
{
    const bool N = mcu.sr & STATUS_N;
    const bool V = mcu.sr & STATUS_V;
    const bool Z = mcu.sr & STATUS_Z;
    if ((Z || (N ^ V)) == false)
    {
        mcu.pc = st.br_true;
    }
    else
    {
        mcu.pc = st.br_false;
    }
}

void I_BLE(mcu_t& mcu, const DecodedInstructionParams& st)
{
    const bool N = mcu.sr & STATUS_N;
    const bool V = mcu.sr & STATUS_V;
    const bool Z = mcu.sr & STATUS_Z;
    if ((Z || (N ^ V)) == true)
    {
        mcu.pc = st.br_true;
    }
    else
    {
        mcu.pc = st.br_false;
    }
}

void I_RTE(mcu_t& mcu, const DecodedInstructionParams& st)
{
    (void)st;
    mcu.sr        = MCU_PopStack(mcu);
    mcu.cp        = (uint8_t)MCU_PopStack(mcu);
    mcu.pc        = MCU_PopStack(mcu);
    mcu.ex_ignore = 1;
}

void I_RTS(mcu_t& mcu, const DecodedInstructionParams& st)
{
    (void)st;
    mcu.pc = MCU_PopStack(mcu);
}

void I_PRTS(mcu_t& mcu, const DecodedInstructionParams& st)
{
    (void)st;
    mcu.cp = (uint8_t)MCU_PopStack(mcu);
    mcu.pc = MCU_PopStack(mcu);
}

void I_SLEEP(mcu_t& mcu, const DecodedInstructionParams& st)
{
    (void)st;
    mcu.sleep = 1;
    ++mcu.pc;
}

void I_JMP_ARn(mcu_t& mcu, const DecodedInstructionParams& st)
{
    mcu.pc = mcu.r[st.op_reg];
}

void I_JMP_aa16(mcu_t& mcu, const DecodedInstructionParams& st)
{
    mcu.pc = st.br_true;
}

void I_JSR_aa16(mcu_t& mcu, const DecodedInstructionParams& st)
{
    MCU_PushStack(mcu, st.br_false);
    mcu.pc = st.br_true;
}

void I_JSR_ARn(mcu_t& mcu, const DecodedInstructionParams& st)
{
    MCU_PushStack(mcu, st.br_false);
    mcu.pc = mcu.r[st.op_reg];
}

void I_PJMP_aa24(mcu_t& mcu, const DecodedInstructionParams& st)
{
    mcu.cp = st.op_page;
    mcu.pc = st.op_data;
}

void I_PJSR_aa24(mcu_t& mcu, const DecodedInstructionParams& st)
{
    MCU_PushStack(mcu, st.br_false);
    MCU_PushStack(mcu, mcu.cp);
    mcu.cp = st.op_page;
    mcu.pc = st.br_true;
}

void I_PJSR_ARn(mcu_t& mcu, const DecodedInstructionParams& st)
{
    MCU_PushStack(mcu, st.br_false);
    MCU_PushStack(mcu, mcu.cp);
    mcu.cp = static_cast<uint8_t>(mcu.r[st.op_reg]);
    mcu.pc = mcu.r[st.op_reg + 1];
}

void I_PJMP_ARn(mcu_t& mcu, const DecodedInstructionParams& st)
{
    mcu.cp = (uint8_t)mcu.r[st.op_reg];
    mcu.pc = mcu.r[st.op_reg + 1];
}

void I_RTD_immXX(mcu_t& mcu, const DecodedInstructionParams& st)
{
    mcu.pc   = MCU_PopStack(mcu);
    mcu.r[7] = (uint16_t)(mcu.r[7] + (int16_t)st.op_data);
}

void I_TRAPA_imm4(mcu_t& mcu, const DecodedInstructionParams& st)
{
    MCU_Interrupt_TRAPA(mcu, (uint8_t)st.op_data);
    mcu.pc += 2;
}

void I_BSR(mcu_t& mcu, const DecodedInstructionParams& instr)
{
    MCU_PushStack(mcu, instr.br_false);
    mcu.pc = instr.br_true;
}

} // namespace decoder2
