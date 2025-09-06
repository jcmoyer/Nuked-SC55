#include "cache.h"

#include "mcu.h"

I_InstructionCache::I_InstructionCache()
{
    m_cache = std::make_unique<ArrayType>();
    m_cache->fill({});
}

void I_InstructionCache::DoCache(mcu_t&                     mcu,
                                 uint32_t                   instr_start,
                                 I_Handler_Erased_Func      func,
                                 const I_CachedInstruction& st)
{
    (*m_cache)[instr_start] = {.F = func, .instr = st, .size = (uint8_t)(mcu.pc - instr_start)};
    func(mcu, st);
}

void I_InstructionCache::DoCacheJump(mcu_t& mcu, uint32_t instr_start, I_Handler_Erased_Func func, int16_t disp)
{
    const uint16_t next_ip = mcu.pc;

    I_CachedInstruction st;
    st.br_true              = (uint16_t)(next_ip + disp);
    st.br_false             = (uint16_t)(next_ip + disp);
    (*m_cache)[instr_start] = {.F = func, .instr = st, .size = 0};
    func(mcu, st);
}

void I_InstructionCache::DoCacheBranch(mcu_t& mcu, uint32_t instr_start, I_Handler_Erased_Func func, int16_t disp)
{
    const uint16_t next_ip = mcu.pc;

    I_CachedInstruction st;
    st.br_true              = (uint16_t)(next_ip + disp);
    st.br_false             = next_ip;
    (*m_cache)[instr_start] = {.F = func, .instr = st, .size = 0};
    func(mcu, st);
}

size_t I_InstructionCache::CountCached() const
{
    size_t count = 0;
    for (const auto& i : *m_cache)
    {
        if (i.F)
        {
            ++count;
        }
    }
    return count;
}
