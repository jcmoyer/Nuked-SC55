/*
 * Copyright (C) 2021, 2024 nukeykt
 *
 *  Redistribution and use of this code or any derivative works are permitted
 *  provided that the following conditions are met:
 *
 *   - Redistributions may not be sold, nor may they be used in a commercial
 *     product or activity.
 *
 *   - Redistributions that are modified from the original source must include the
 *     complete source code, including the source code for all components used by a
 *     binary built from the modified sources. However, as a special exception, the
 *     source code distributed need not include anything that is normally distributed
 *     (in either source or binary form) with the major components (compiler, kernel,
 *     and so on) of the operating system on which the executable runs, unless that
 *     component itself accompanies the executable.
 *
 *   - Redistributions must reproduce the above copyright notice, this list of
 *     conditions and the following disclaimer in the documentation and/or other
 *     materials provided with the distribution.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 */
#include <stdint.h>
#include <string.h>
#include <xmmintrin.h>
#include <smmintrin.h>
#include "mcu.h"
#include "mcu_timer.h"

enum {
    REG_TCR = 0x00,
    REG_TCSR = 0x01,
    REG_FRCH = 0x02,
    REG_FRCL = 0x03,
    REG_OCRAH = 0x04,
    REG_OCRAL = 0x05,
    REG_OCRBH = 0x06,
    REG_OCRBL = 0x07,
    REG_ICRH = 0x08,
    REG_ICRL = 0x09,
};

void TIMER_Init(mcu_timer_t& timer, mcu_t& mcu)
{
    timer.mcu = &mcu;
}

void TIMER_Write(mcu_timer_t& timer, uint32_t address, uint8_t data)
{
    uint32_t t = (address >> 4) - 1;
    if (t > 2)
        return;
    address &= 0x0f;
    switch (address)
    {
    case REG_TCR:
        timer.frt.tcr[t] = data;
        break;
    case REG_TCSR:
        timer.frt.tcsr[t] &= ~0xf;
        timer.frt.tcsr[t] |= data & 0xf;
        if ((data & 0x10) == 0 && (timer.frt.status_rd[t] & 0x10) != 0)
        {
            timer.frt.tcsr[t] &= ~0x10;
            timer.frt.status_rd[t] &= ~0x10;
            MCU_Interrupt_SetRequest(*timer.mcu, INTERRUPT_SOURCE_FRT0_FOVI + t * 4, 0);
        }
        if ((data & 0x20) == 0 && (timer.frt.status_rd[t] & 0x20) != 0)
        {
            timer.frt.tcsr[t] &= ~0x20;
            timer.frt.status_rd[t] &= ~0x20;
            MCU_Interrupt_SetRequest(*timer.mcu, INTERRUPT_SOURCE_FRT0_OCIA + t * 4, 0);
        }
        if ((data & 0x40) == 0 && (timer.frt.status_rd[t] & 0x40) != 0)
        {
            timer.frt.tcsr[t] &= ~0x40;
            timer.frt.status_rd[t] &= ~0x40;
            MCU_Interrupt_SetRequest(*timer.mcu, INTERRUPT_SOURCE_FRT0_OCIB + t * 4, 0);
        }
        break;
    case REG_FRCH:
    case REG_OCRAH:
    case REG_OCRBH:
    case REG_ICRH:
        timer.timer_tempreg = data;
        break;
    case REG_FRCL:
        timer.frt.frc[t] = (timer.timer_tempreg << 8) | data;
        break;
    case REG_OCRAL:
        timer.frt.ocra[t] = (timer.timer_tempreg << 8) | data;
        break;
    case REG_OCRBL:
        timer.frt.ocrb[t] = (timer.timer_tempreg << 8) | data;
        break;
    case REG_ICRL:
        timer.frt.icr[t] = (timer.timer_tempreg << 8) | data;
        break;
    }
}

uint8_t TIMER_Read(mcu_timer_t& timer, uint32_t address)
{
    uint32_t t = (address >> 4) - 1;
    if (t > 2)
        return 0xff;
    address &= 0x0f;
    switch (address)
    {
    case REG_TCR:
        return timer.frt.tcr[t];
    case REG_TCSR:
    {
        uint8_t ret = timer.frt.tcsr[t];
        timer.frt.status_rd[t] |= timer.frt.tcsr[t] & 0xf0;
        //timer.frt.status_rd |= 0xf0;
        return ret;
    }
    case REG_FRCH:
        timer.timer_tempreg = timer.frt.frc[t] & 0xff;
        return timer.frt.frc [t]>> 8;
    case REG_OCRAH:
        timer.timer_tempreg = timer.frt.ocra[t] & 0xff;
        return timer.frt.ocra [t]>> 8;
    case REG_OCRBH:
        timer.timer_tempreg = timer.frt.ocrb[t] & 0xff;
        return timer.frt.ocrb[t] >> 8;
    case REG_ICRH:
        timer.timer_tempreg = timer.frt.icr[t] & 0xff;
        return timer.frt.icr [t]>> 8;
    case REG_FRCL:
    case REG_OCRAL:
    case REG_OCRBL:
    case REG_ICRL:
        return timer.timer_tempreg;
    }
    return 0xff;
}

void TIMER2_Write(mcu_timer_t& timer, uint32_t address, uint8_t data)
{
    switch (address)
    {
    case DEV_TMR_TCR:
        timer.tcr = data;
        break;
    case DEV_TMR_TCSR:
        timer.tcsr &= ~0xf;
        timer.tcsr |= data & 0xf;
        if ((data & 0x20) == 0 && (timer.status_rd & 0x20) != 0)
        {
            timer.tcsr &= ~0x20;
            timer.status_rd &= ~0x20;
            MCU_Interrupt_SetRequest(*timer.mcu, INTERRUPT_SOURCE_TIMER_OVI, 0);
        }
        if ((data & 0x40) == 0 && (timer.status_rd & 0x40) != 0)
        {
            timer.tcsr &= ~0x40;
            timer.status_rd &= ~0x40;
            MCU_Interrupt_SetRequest(*timer.mcu, INTERRUPT_SOURCE_TIMER_CMIA, 0);
        }
        if ((data & 0x80) == 0 && (timer.status_rd & 0x80) != 0)
        {
            timer.tcsr &= ~0x80;
            timer.status_rd &= ~0x80;
            MCU_Interrupt_SetRequest(*timer.mcu, INTERRUPT_SOURCE_TIMER_CMIB, 0);
        }
        break;
    case DEV_TMR_TCORA:
        timer.tcora = data;
        break;
    case DEV_TMR_TCORB:
        timer.tcorb = data;
        break;
    case DEV_TMR_TCNT:
        timer.tcnt = data;
        break;
    }
}
uint8_t TIMER_Read2(mcu_timer_t& timer, uint32_t address)
{
    switch (address)
    {
    case DEV_TMR_TCR:
        return timer.tcr;
    case DEV_TMR_TCSR:
    {
        uint8_t ret = timer.tcsr;
        timer.status_rd |= timer.tcsr & 0xe0;
        return ret;
    }
    case DEV_TMR_TCORA:
        return timer.tcora;
    case DEV_TMR_TCORB:
        return timer.tcorb;
    case DEV_TMR_TCNT:
        return timer.tcnt;
    }
    return 0xff;
}

alignas(16) constexpr uint8_t FRT_STEP_TABLE_GENERIC[16] = {
    3, 7, 31, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

alignas(16) constexpr uint8_t SSE_ONES[16] = {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
};

alignas(16) constexpr uint16_t SSE_LOW_1_16[8] = {
    1, 1, 1, 1, 1, 1, 1, 1,
};

alignas(16) constexpr uint8_t SHUFFLE_16_TO_8[16] = {
    0, 2, 4, 6, 8, 10, 12, 14, 0, 0, 0, 0, 0, 0, 0, 0,
};

alignas(16) constexpr uint8_t SSE_LOW_3[16] = {
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
};

alignas(16) constexpr uint16_t OVERFLOW_TCSR_BITS[8] = {
    0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
};

alignas(16) constexpr uint16_t MATCHA_TCSR_BITS[8] = {
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
};

alignas(16) constexpr uint16_t MATCHB_TCSR_BITS[8] = {
    0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40,
};

alignas(16) constexpr uint8_t FRT_STEP_TABLE_MK1[16] = {
    3, 7, 31, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

constexpr uint64_t TIMER_STEP_TABLE_GENERIC[8] = {
    0, 7, 63, 1023, 0, 1, 1, 1
};

constexpr uint64_t TIMER_STEP_TABLE_MK1[8] = {
    0, 7, 63, 1023, 0, 3, 3, 3
};

static void TIMER_FRT_Reference(mcu_timer_t& timer, bool mk1)
{
    const auto& FRT_STEP_TABLE = mk1 ? FRT_STEP_TABLE_MK1 : FRT_STEP_TABLE_GENERIC;

    bool frt_step[3];
    uint32_t value[3];
    bool matcha[3];
    bool matchb[3];
    bool of[3];
    for (int i = 0; i < 3; i++)
    {
        frt_step[i] = !(timer.timer_cycles & FRT_STEP_TABLE[timer.frt.tcr[i] & 3]);
        value[i] = timer.frt.frc[i];
        matcha[i] = value[i] == timer.frt.ocra[i];
        matchb[i] = value[i] == timer.frt.ocrb[i];
        if (timer.frt.tcsr[i] & 1 && matcha[i])
            value[i] = 0;
        else
            ++value[i];
        of[i] = (value[i] >> 16) & 1;

        if (frt_step[i])
        {
            timer.frt.frc[i] = value[i];
            // flags
            if (of[i])
                timer.frt.tcsr[i] |= 0x10;
            if (matcha[i])
                timer.frt.tcsr[i] |= 0x20;
            if (matchb[i])
                timer.frt.tcsr[i] |= 0x40;
            if ((timer.frt.tcr[i] & 0x10) != 0 && (timer.frt.tcsr[i] & 0x10) != 0)
                MCU_Interrupt_SetRequest(*timer.mcu, INTERRUPT_SOURCE_FRT0_FOVI + i * 4, 1);
            if ((timer.frt.tcr[i] & 0x20) != 0 && (timer.frt.tcsr[i] & 0x20) != 0)
                MCU_Interrupt_SetRequest(*timer.mcu, INTERRUPT_SOURCE_FRT0_OCIA + i * 4, 1);
            if ((timer.frt.tcr[i] & 0x40) != 0 && (timer.frt.tcsr[i] & 0x40) != 0)
                MCU_Interrupt_SetRequest(*timer.mcu, INTERRUPT_SOURCE_FRT0_OCIB + i * 4, 1);
        }
    }
}

template <size_t I>
void TIMER_FRT_SSE_Dispatch(const mcu_timer_t& timer, uint64_t word)
{
    if (word & (0x10ull << (I * 16))) {
        MCU_Interrupt_SetRequest(*timer.mcu, INTERRUPT_SOURCE_FRT0_FOVI + I * 4, 1);
    }
    if (word & (0x20ull << (I * 16))) {
        MCU_Interrupt_SetRequest(*timer.mcu, INTERRUPT_SOURCE_FRT0_OCIA + I * 4, 1);
    }
    if (word & (0x40ull << (I * 16))) {
        MCU_Interrupt_SetRequest(*timer.mcu, INTERRUPT_SOURCE_FRT0_OCIB + I * 4, 1);
    }
}

static void TIMER_FRT_SSE(mcu_timer_t& timer, bool mk1)
{
    const auto& FRT_STEP_TABLE = mk1 ? FRT_STEP_TABLE_MK1 : FRT_STEP_TABLE_GENERIC;

    const __m128i step_table = _mm_load_si128((const __m128i*)FRT_STEP_TABLE);
    const __m128i lo3        = _mm_load_si128((const __m128i*)SSE_LOW_3);
    const __m128i shuf_16_8  = _mm_load_si128((const __m128i*)SHUFFLE_16_TO_8);
    const __m128i lo1_16     = _mm_load_si128((const __m128i*)SSE_LOW_1_16);
    const __m128i all_one    = _mm_load_si128((const __m128i*)SSE_ONES);
    const __m128i all_zero   = _mm_setzero_si128();

    __m128i v_cycles = _mm_set1_epi16(timer.timer_cycles);

    __m128i v_table = _mm_load_si128((const __m128i*)timer.frt.tcr);
    const __m128i tcr_wide = _mm_cvtepu8_epi16(v_table);

    // compute indices for LUT
    v_table = _mm_and_si128(v_table, lo3);
    // select from LUT and convert to u16 for the rest of the algo
    v_table = _mm_shuffle_epi8(step_table, v_table);
    v_table = _mm_cvtepu8_epi16(v_table);

    v_cycles = _mm_and_si128(v_table, v_cycles);

    // construct mask for timers that will be stepped (ffff = step, 0000 = no step)
    const __m128i step_mask = _mm_cmpeq_epi16(v_cycles, all_zero);

    const __m128i value = _mm_load_si128((const __m128i*)timer.frt.frc);
    // note tcsr is stored as packed u8, we need packed u16
    const __m128i tcsr  = _mm_cvtepu8_epi16(_mm_load_si128((const __m128i*)timer.frt.tcsr));
    const __m128i ocra  = _mm_load_si128((const __m128i*)timer.frt.ocra);
    const __m128i ocrb  = _mm_load_si128((const __m128i*)timer.frt.ocrb);

    __m128i matcha = _mm_cmpeq_epi16(value, ocra);
    __m128i matchb = _mm_cmpeq_epi16(value, ocrb);

    __m128i tcsr_cond = _mm_cmpeq_epi16(_mm_and_si128(tcsr, lo1_16), lo1_16);
    tcsr_cond = _mm_and_si128(tcsr_cond, matcha);

    // incremented form of frc will only be applied if step_mask is set
    __m128i new_frc = _mm_add_epi16(value, lo1_16);
    // select zero or +1 based on result of cond
    new_frc = _mm_blendv_epi8(new_frc, all_zero, tcsr_cond);
    // however we only take the new value if we step
    new_frc = _mm_blendv_epi8(value, new_frc, step_mask);

    _mm_store_si128((__m128i*)timer.frt.frc, new_frc);

    // check if any adds overflowed
    const __m128i old_is_max16 = _mm_cmpeq_epi16(value, all_one);
    const __m128i new_is_min16 = _mm_cmpeq_epi16(new_frc, all_zero);
    const __m128i overflow     = _mm_and_si128(old_is_max16, new_is_min16);

    __m128i new_tcsr;
    // first we accumulate the new bits
    new_tcsr = _mm_and_si128(overflow, _mm_load_si128((const __m128i*)OVERFLOW_TCSR_BITS));
    new_tcsr = _mm_or_si128(new_tcsr, _mm_and_si128(matcha, _mm_load_si128((const __m128i*)MATCHA_TCSR_BITS)));
    new_tcsr = _mm_or_si128(new_tcsr, _mm_and_si128(matchb, _mm_load_si128((const __m128i*)MATCHB_TCSR_BITS)));
    // mask out the ones we don't step
    new_tcsr = _mm_and_si128(new_tcsr, step_mask);
    // and combine with the original
    new_tcsr = _mm_or_si128(tcsr, new_tcsr);
    // remember to shuffle down to convert from 16 to 8 bit
    _mm_store_si128((__m128i*)timer.frt.tcsr, _mm_shuffle_epi8(new_tcsr, shuf_16_8));

    // we can compute all 9 interrupt conditions at once but we do still need
    // to extract the bits and branch on them individually to make calls :(
    const __m128i interrupt_mask = _mm_and_si128(new_tcsr, tcr_wide);

    // the interrupt info is in the bottom 64-bit word
    const uint64_t word = _mm_extract_epi64(_mm_and_si128(interrupt_mask, step_mask), 0);

    // Dispatch interrupts for all timers
    TIMER_FRT_SSE_Dispatch<0>(timer, word);
    TIMER_FRT_SSE_Dispatch<1>(timer, word);
    TIMER_FRT_SSE_Dispatch<2>(timer, word);
}

void TIMER_Clock(mcu_timer_t& timer, uint64_t cycles)
{
    const bool mk1 = timer.mcu->mcu_mk1;
    const auto& TIMER_STEP_TABLE = mk1 ? TIMER_STEP_TABLE_MK1 : TIMER_STEP_TABLE_GENERIC;

    while (timer.timer_cycles*2 < cycles) // FIXME
    {
        //TIMER_FRT_Reference(timer, mk1);
        TIMER_FRT_SSE(timer, mk1);

        const bool timer_step = !(timer.timer_cycles & TIMER_STEP_TABLE[timer.tcr & 7]);

        if (timer_step)
        {
            uint32_t value = timer.tcnt;
            uint32_t matcha = value == timer.tcora;
            uint32_t matchb = value == timer.tcorb;
            if ((timer.tcr & 24) == 8 && matcha)
                value = 0;
            else if ((timer.tcr & 24) == 16 && matchb)
                value = 0;
            else
                value++;
            uint32_t of = (value >> 8) & 1;
            value &= 0xff;
            timer.tcnt = value;

            // flags
            if (of)
                timer.tcsr |= 0x20;
            if (matcha)
                timer.tcsr |= 0x40;
            if (matchb)
                timer.tcsr |= 0x80;
            if ((timer.tcr & 0x20) != 0 && (timer.tcsr & 0x20) != 0)
                MCU_Interrupt_SetRequest(*timer.mcu, INTERRUPT_SOURCE_TIMER_OVI, 1);
            if ((timer.tcr & 0x40) != 0 && (timer.tcsr & 0x40) != 0)
                MCU_Interrupt_SetRequest(*timer.mcu, INTERRUPT_SOURCE_TIMER_CMIA, 1);
            if ((timer.tcr & 0x80) != 0 && (timer.tcsr & 0x80) != 0)
                MCU_Interrupt_SetRequest(*timer.mcu, INTERRUPT_SOURCE_TIMER_CMIB, 1);
        }

        timer.timer_cycles++;
    }
}
