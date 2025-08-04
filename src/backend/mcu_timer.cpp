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
#include "mcu_timer.h"
#include "mcu.h"
#include <cstdint>

enum TMR_TCR_Bits : uint8_t
{
    TMR_TCR_CKS0  = 1 << 0, // Clock Select 0
    TMR_TCR_CKS1  = 1 << 1, // Clock Select 1
    TMR_TCR_CKS2  = 1 << 2, // Clock Select 2
    TMR_TCR_CCLR0 = 1 << 3, // Counter Clear 0
    TMR_TCR_CCLR1 = 1 << 4, // Counter Clear 1
    TMR_TCR_OVIE  = 1 << 5, // Timer Overflow Interrupt Enable
    TMR_TCR_CMIEA = 1 << 6, // Compare-match Interrupt Enable A
    TMR_TCR_CMIEB = 1 << 7, // Compare-match Interrupt Enable B
};

enum TMR_TCSR_Bits : uint8_t
{
    TMR_TCSR_OS0  = 1 << 0, // Output Select 0
    TMR_TCSR_OS1  = 1 << 1, // Output Select 1
    TMR_TCSR_OS2  = 1 << 2, // Output Select 2
    TMR_TCSR_OS3  = 1 << 3, // Output Select 3
    TMR_TCSR_BIT4 = 1 << 4, // Reserved
    TMR_TCSR_OVF  = 1 << 5, // Timer Overflow Flag
    TMR_TCSR_CMFA = 1 << 6, // Compare-Match Flag A
    TMR_TCSR_CMFB = 1 << 7, // Compare-Match Flag B
};

enum FRT_TCR_Bits : uint8_t
{
    FRT_TCR_CKS0  = 1 << 0, // Clock Select 0
    FRT_TCR_CKS1  = 1 << 1, // Clock Select 1
    FRT_TCR_OEA   = 1 << 2, // Output Enable A
    FRT_TCR_OEB   = 1 << 3, // Output Enable B
    FRT_TCR_OVIE  = 1 << 4, // Timer overflow Interrupt Enable
    FRT_TCR_OCIEA = 1 << 5, // Output Compare Interrupt Enable A
    FRT_TCR_OCIEB = 1 << 6, // Output Compare Interrupt Enable B
    FRT_TCR_ICIE  = 1 << 7, // Input Capture Interrupt Enable
};

enum FRT_TCSR_Bits : uint8_t
{
    FRT_TCSR_CCLRA = 1 << 0, // Counter Clear A
    FRT_TCSR_IEDG  = 1 << 1, // Input Edge Select
    FRT_TCSR_OLVLA = 1 << 2, // Output Level A
    FRT_TCSR_OLVLB = 1 << 3, // Output Level B
    FRT_TCSR_OVF   = 1 << 4, // Timer Overflow Flag
    FRT_TCSR_OCFA  = 1 << 5, // Output Compare Flag A
    FRT_TCSR_OCFB  = 1 << 6, // Output Compare Flag B
    FRT_TCSR_ICF   = 1 << 7, // Input Capture Flag
};

// Values are byte offsets from start of FRTs in memory (ffa0, ffb0, ffc0)
enum FRT_Field_Offset : uint8_t
{
    REG_TCR   = 0x00,
    REG_TCSR  = 0x01,
    REG_FRCH  = 0x02,
    REG_FRCL  = 0x03,
    REG_OCRAH = 0x04,
    REG_OCRAL = 0x05,
    REG_OCRBH = 0x06,
    REG_OCRBL = 0x07,
    REG_ICRH  = 0x08,
    REG_ICRL  = 0x09,
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
    frt_t& frt = timer.frt[t];

    address &= 0x0f;
    switch (address)
    {
    case REG_TCR:
        frt.tcr = data;
        break;
    case REG_TCSR:
        frt.tcsr &= ~0xf;
        frt.tcsr |= data & 0xf;
        if ((data & FRT_TCSR_OVF) == 0 && (frt.status_rd & FRT_TCSR_OVF) != 0)
        {
            frt.tcsr      &= ~FRT_TCSR_OVF;
            frt.status_rd &= ~FRT_TCSR_OVF;
            MCU_Interrupt_SetRequest(*timer.mcu, (MCU_Interrupt_Source)(INTERRUPT_SOURCE_FRT0_FOVI + t * 4), 0);
        }
        if ((data & FRT_TCSR_OCFA) == 0 && (frt.status_rd & FRT_TCSR_OCFA) != 0)
        {
            frt.tcsr      &= ~FRT_TCSR_OCFA;
            frt.status_rd &= ~FRT_TCSR_OCFA;
            MCU_Interrupt_SetRequest(*timer.mcu, (MCU_Interrupt_Source)(INTERRUPT_SOURCE_FRT0_OCIA + t * 4), 0);
        }
        if ((data & FRT_TCSR_OCFB) == 0 && (frt.status_rd & FRT_TCSR_OCFB) != 0)
        {
            frt.tcsr      &= ~FRT_TCSR_OCFB;
            frt.status_rd &= ~FRT_TCSR_OCFB;
            MCU_Interrupt_SetRequest(*timer.mcu, (MCU_Interrupt_Source)(INTERRUPT_SOURCE_FRT0_OCIB + t * 4), 0);
        }
        break;
    case REG_FRCH:
    case REG_OCRAH:
    case REG_OCRBH:
    case REG_ICRH:
        timer.tempreg = data;
        break;
    case REG_FRCL:
        frt.frc = (uint16_t)((timer.tempreg << 8) | data);
        break;
    case REG_OCRAL:
        frt.ocra = (uint16_t)((timer.tempreg << 8) | data);
        break;
    case REG_OCRBL:
        frt.ocrb = (uint16_t)((timer.tempreg << 8) | data);
        break;
    case REG_ICRL:
        frt.icr = (uint16_t)((timer.tempreg << 8) | data);
        break;
    }
}

uint8_t TIMER_Read(mcu_timer_t& timer, uint32_t address)
{
    uint32_t t = (address >> 4) - 1;
    if (t > 2)
        return 0xff;
    frt_t& frt = timer.frt[t];

    address &= 0x0f;
    switch (address)
    {
    case REG_TCR:
        return frt.tcr;
    case REG_TCSR: {
        uint8_t ret    = frt.tcsr;
        frt.status_rd |= frt.tcsr & 0xf0;
        // frt.status_rd |= 0xf0;
        return ret;
    }
    case REG_FRCH:
        timer.tempreg = (uint8_t)frt.frc;
        return (uint8_t)(frt.frc >> 8);
    case REG_OCRAH:
        timer.tempreg = (uint8_t)frt.ocra;
        return (uint8_t)(frt.ocra >> 8);
    case REG_OCRBH:
        timer.tempreg = (uint8_t)frt.ocrb;
        return (uint8_t)(frt.ocrb >> 8);
    case REG_ICRH:
        timer.tempreg = (uint8_t)frt.icr;
        return (uint8_t)(frt.icr >> 8);
    case REG_FRCL:
    case REG_OCRAL:
    case REG_OCRBL:
    case REG_ICRL:
        return timer.tempreg;
    }
    return 0xff;
}

void TIMER2_Write(mcu_timer_t& timer, uint32_t address, uint8_t data)
{
    tmr_t& tmr = timer.tmr;

    switch (address)
    {
    case DEV_TMR_TCR:
        tmr.tcr = data;
        break;
    case DEV_TMR_TCSR:
        tmr.tcsr &= ~0xf;
        tmr.tcsr |= data & 0xf;
        if ((data & TMR_TCSR_OVF) == 0 && (tmr.status_rd & TMR_TCSR_OVF) != 0)
        {
            tmr.tcsr      &= ~TMR_TCSR_OVF;
            tmr.status_rd &= ~TMR_TCSR_OVF;
            MCU_Interrupt_SetRequest(*timer.mcu, INTERRUPT_SOURCE_TIMER_OVI, 0);
        }
        if ((data & TMR_TCSR_CMFA) == 0 && (tmr.status_rd & TMR_TCSR_CMFA) != 0)
        {
            tmr.tcsr      &= ~TMR_TCSR_CMFA;
            tmr.status_rd &= ~TMR_TCSR_CMFA;
            MCU_Interrupt_SetRequest(*timer.mcu, INTERRUPT_SOURCE_TIMER_CMIA, 0);
        }
        if ((data & TMR_TCSR_CMFB) == 0 && (tmr.status_rd & TMR_TCSR_CMFB) != 0)
        {
            tmr.tcsr      &= ~TMR_TCSR_CMFB;
            tmr.status_rd &= ~TMR_TCSR_CMFB;
            MCU_Interrupt_SetRequest(*timer.mcu, INTERRUPT_SOURCE_TIMER_CMIB, 0);
        }
        break;
    case DEV_TMR_TCORA:
        tmr.tcora = data;
        break;
    case DEV_TMR_TCORB:
        tmr.tcorb = data;
        break;
    case DEV_TMR_TCNT:
        tmr.tcnt = data;
        break;
    }
}

uint8_t TIMER_Read2(mcu_timer_t& timer, uint32_t address)
{
    tmr_t& tmr = timer.tmr;

    switch (address)
    {
    case DEV_TMR_TCR:
        return tmr.tcr;
    case DEV_TMR_TCSR: {
        uint8_t ret    = tmr.tcsr;
        tmr.status_rd |= tmr.tcsr & (TMR_TCSR_OVF | TMR_TCSR_CMFA | TMR_TCSR_CMFB);
        return ret;
    }
    case DEV_TMR_TCORA:
        return tmr.tcora;
    case DEV_TMR_TCORB:
        return tmr.tcorb;
    case DEV_TMR_TCNT:
        return tmr.tcnt;
    }
    return 0xff;
}

inline void TIMER_ClockFrt(mcu_timer_t& timer, int frt_id)
{
    frt_t& frt = timer.frt[frt_id];

    if (timer.cycles & timer.frt_step_table[frt.tcr & (FRT_TCR_CKS0 | FRT_TCR_CKS1)])
    {
        return;
    }

    uint32_t   value  = frt.frc;
    const bool matcha = value == frt.ocra;
    const bool matchb = value == frt.ocrb;
    if ((frt.tcsr & FRT_TCSR_CCLRA) && matcha) // CCLRA
    {
        value = 0;
    }
    else
    {
        ++value;
    }
    const bool of = (value >> 16) & 1;
    frt.frc       = (uint16_t)value;

    // flags
    if (of)
        frt.tcsr |= FRT_TCSR_OVF;
    if (matcha)
        frt.tcsr |= FRT_TCSR_OCFA;
    if (matchb)
        frt.tcsr |= FRT_TCSR_OCFB;

    if ((frt.tcr & FRT_TCR_OVIE) != 0 && (frt.tcsr & FRT_TCSR_OVF) != 0)
        MCU_Interrupt_SetRequest(*timer.mcu, (MCU_Interrupt_Source)(INTERRUPT_SOURCE_FRT0_FOVI + frt_id * 4), 1);
    if ((frt.tcr & FRT_TCR_OCIEA) != 0 && (frt.tcsr & FRT_TCSR_OCFA) != 0)
        MCU_Interrupt_SetRequest(*timer.mcu, (MCU_Interrupt_Source)(INTERRUPT_SOURCE_FRT0_OCIA + frt_id * 4), 1);
    if ((frt.tcr & FRT_TCR_OCIEB) != 0 && (frt.tcsr & FRT_TCSR_OCFB) != 0)
        MCU_Interrupt_SetRequest(*timer.mcu, (MCU_Interrupt_Source)(INTERRUPT_SOURCE_FRT0_OCIB + frt_id * 4), 1);
}

inline void TIMER_ClockTmr(mcu_timer_t& timer)
{
    tmr_t& tmr = timer.tmr;

    if (timer.cycles & timer.tmr_step_table[tmr.tcr & (TMR_TCR_CKS0 | TMR_TCR_CKS1 | TMR_TCR_CKS2)])
    {
        return;
    }

    uint32_t   value  = tmr.tcnt;
    const bool matcha = value == tmr.tcora;
    const bool matchb = value == tmr.tcorb;
    if ((tmr.tcr & (TMR_TCR_CCLR0 | TMR_TCR_CCLR1)) == TMR_TCR_CCLR0 && matcha)
    {
        value = 0;
    }
    else if ((tmr.tcr & (TMR_TCR_CCLR0 | TMR_TCR_CCLR1)) == TMR_TCR_CCLR1 && matchb)
    {
        value = 0;
    }
    else
    {
        ++value;
    }
    const bool of = (value >> 8) & 1;
    tmr.tcnt      = (uint8_t)value;

    // flags
    if (of)
        tmr.tcsr |= TMR_TCSR_OVF;
    if (matcha)
        tmr.tcsr |= TMR_TCSR_CMFA;
    if (matchb)
        tmr.tcsr |= TMR_TCSR_CMFB;

    if ((tmr.tcr & TMR_TCR_OVIE) != 0 && (tmr.tcsr & TMR_TCSR_OVF) != 0)
        MCU_Interrupt_SetRequest(*timer.mcu, INTERRUPT_SOURCE_TIMER_OVI, 1);
    if ((tmr.tcr & TMR_TCR_CMIEA) != 0 && (tmr.tcsr & TMR_TCSR_CMFA) != 0)
        MCU_Interrupt_SetRequest(*timer.mcu, INTERRUPT_SOURCE_TIMER_CMIA, 1);
    if ((tmr.tcr & TMR_TCR_CMIEB) != 0 && (tmr.tcsr & TMR_TCSR_CMFB) != 0)
        MCU_Interrupt_SetRequest(*timer.mcu, INTERRUPT_SOURCE_TIMER_CMIB, 1);
}

void TIMER_Clock(mcu_timer_t& timer, uint64_t cycles)
{
    while (timer.cycles * 2 < cycles) // FIXME
    {
        for (int i = 0; i < 3; i++)
        {
            TIMER_ClockFrt(timer, i);
        }

        TIMER_ClockTmr(timer);

        ++timer.cycles;
    }
}

// These tables are indexed by the low CKSn bits of the TCR.
constexpr FRT_Step_Table FRT_STEP_TABLE_GENERIC = {3, 7, 31, 1};
constexpr FRT_Step_Table FRT_STEP_TABLE_MK1     = {3, 7, 31, 3};

constexpr TMR_Step_Table TMR_STEP_TABLE_GENERIC = {0, 7, 63, 1023, 0, 1, 1, 1};
constexpr TMR_Step_Table TMR_STEP_TABLE_MK1     = {0, 7, 63, 1023, 0, 3, 3, 3};

void TIMER_NotifyRomsetChange(mcu_timer_t& timer)
{
    const bool is_mk1    = timer.mcu->is_mk1;
    timer.frt_step_table = is_mk1 ? FRT_STEP_TABLE_MK1 : FRT_STEP_TABLE_GENERIC;
    timer.tmr_step_table = is_mk1 ? TMR_STEP_TABLE_MK1 : TMR_STEP_TABLE_GENERIC;
}
