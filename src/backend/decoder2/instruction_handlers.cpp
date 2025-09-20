#include "instruction_handlers.h"

#include "mcu.h"

void I_NOP(mcu_t& mcu, const I_CachedInstruction&)
{
    ++mcu.pc;
}

void I_BRA(mcu_t& mcu, const I_CachedInstruction& st)
{
    mcu.pc = st.br_true;
}

void I_BRN(mcu_t& mcu, const I_CachedInstruction& st)
{
    mcu.pc = st.br_false;
}

void I_BHI(mcu_t& mcu, const I_CachedInstruction& st)
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

void I_BLS(mcu_t& mcu, const I_CachedInstruction& st)
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

void I_BCC(mcu_t& mcu, const I_CachedInstruction& st)
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

void I_BCS(mcu_t& mcu, const I_CachedInstruction& st)
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

void I_BNE(mcu_t& mcu, const I_CachedInstruction& st)
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

void I_BEQ(mcu_t& mcu, const I_CachedInstruction& st)
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

void I_BVC(mcu_t& mcu, const I_CachedInstruction& st)
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

void I_BVS(mcu_t& mcu, const I_CachedInstruction& st)
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

void I_BPL(mcu_t& mcu, const I_CachedInstruction& st)
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

void I_BMI(mcu_t& mcu, const I_CachedInstruction& st)
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

void I_BGE(mcu_t& mcu, const I_CachedInstruction& st)
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

void I_BLT(mcu_t& mcu, const I_CachedInstruction& st)
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

void I_BGT(mcu_t& mcu, const I_CachedInstruction& st)
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

void I_BLE(mcu_t& mcu, const I_CachedInstruction& st)
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

void I_RTE(mcu_t& mcu, const I_CachedInstruction& st)
{
    (void)st;
    mcu.sr        = MCU_PopStack(mcu);
    mcu.cp        = (uint8_t)MCU_PopStack(mcu);
    mcu.pc        = MCU_PopStack(mcu);
    mcu.ex_ignore = 1;
}

void I_BSR(mcu_t& mcu, const I_CachedInstruction& instr)
{
    MCU_PushStack(mcu, instr.br_false);
    mcu.pc = instr.br_true;
}
