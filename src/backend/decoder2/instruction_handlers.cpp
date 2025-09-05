#include "instruction_handlers.h"

#include "mcu.h"

void I_NOP(mcu_t&, const I_CachedInstruction&)
{
}

void I_Bcc_d8_BRA(mcu_t& mcu, const I_CachedInstruction& st)
{
    mcu.pc = st.br_true;
}

void I_Bcc_d8_BRN(mcu_t& mcu, const I_CachedInstruction& st)
{
    mcu.pc = st.br_false;
}

void I_Bcc_d8_BHI(mcu_t& mcu, const I_CachedInstruction& st)
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

void I_Bcc_d8_BLS(mcu_t& mcu, const I_CachedInstruction& st)
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

void I_Bcc_d8_BCC(mcu_t& mcu, const I_CachedInstruction& st)
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

void I_Bcc_d8_BCS(mcu_t& mcu, const I_CachedInstruction& st)
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

void I_Bcc_d8_BNE(mcu_t& mcu, const I_CachedInstruction& st)
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

void I_Bcc_d8_BEQ(mcu_t& mcu, const I_CachedInstruction& st)
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

void I_Bcc_d8_BVC(mcu_t& mcu, const I_CachedInstruction& st)
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

void I_Bcc_d8_BVS(mcu_t& mcu, const I_CachedInstruction& st)
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

void I_Bcc_d8_BPL(mcu_t& mcu, const I_CachedInstruction& st)
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

void I_Bcc_d8_BMI(mcu_t& mcu, const I_CachedInstruction& st)
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

void I_Bcc_d8_BGE(mcu_t& mcu, const I_CachedInstruction& st)
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

void I_Bcc_d8_BLT(mcu_t& mcu, const I_CachedInstruction& st)
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

void I_Bcc_d8_BGT(mcu_t& mcu, const I_CachedInstruction& st)
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

void I_Bcc_d8_BLE(mcu_t& mcu, const I_CachedInstruction& st)
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
