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
#include <cstring>
#include <cstdint>

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
    frt_t *ftimer = &timer.frt[t];
    switch (address)
    {
    case REG_TCR:
        ftimer->tcr = data;
        break;
    case REG_TCSR:
        ftimer->tcsr &= ~0xf;
        ftimer->tcsr |= data & 0xf;
        if ((data & 0x10) == 0 && (ftimer->status_rd & 0x10) != 0)
        {
            ftimer->tcsr &= ~0x10;
            ftimer->status_rd &= ~0x10;
            MCU_Interrupt_SetRequest(*timer.mcu, (MCU_Interrupt_Source)(INTERRUPT_SOURCE_FRT0_FOVI + t * 4), 0);
        }
        if ((data & 0x20) == 0 && (ftimer->status_rd & 0x20) != 0)
        {
            ftimer->tcsr &= ~0x20;
            ftimer->status_rd &= ~0x20;
            MCU_Interrupt_SetRequest(*timer.mcu, (MCU_Interrupt_Source)(INTERRUPT_SOURCE_FRT0_OCIA + t * 4), 0);
        }
        if ((data & 0x40) == 0 && (ftimer->status_rd & 0x40) != 0)
        {
            ftimer->tcsr &= ~0x40;
            ftimer->status_rd &= ~0x40;
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
        ftimer->frc = (uint16_t)((timer.tempreg << 8) | data);
        break;
    case REG_OCRAL:
        ftimer->ocra = (uint16_t)((timer.tempreg << 8) | data);
        break;
    case REG_OCRBL:
        ftimer->ocrb = (uint16_t)((timer.tempreg << 8) | data);
        break;
    case REG_ICRL:
        ftimer->icr = (uint16_t)((timer.tempreg << 8) | data);
        break;
    }
}

uint8_t TIMER_Read(mcu_timer_t& timer, uint32_t address)
{
    uint32_t t = (address >> 4) - 1;
    if (t > 2)
        return 0xff;
    address &= 0x0f;
    frt_t *ftimer = &timer.frt[t];
    switch (address)
    {
    case REG_TCR:
        return ftimer->tcr;
    case REG_TCSR:
    {
        uint8_t ret = ftimer->tcsr;
        ftimer->status_rd |= ftimer->tcsr & 0xf0;
        //ftimer->status_rd |= 0xf0;
        return ret;
    }
    case REG_FRCH:
        timer.tempreg = (uint8_t)ftimer->frc;
        return (uint8_t)(ftimer->frc >> 8);
    case REG_OCRAH:
        timer.tempreg = (uint8_t)ftimer->ocra;
        return (uint8_t)(ftimer->ocra >> 8);
    case REG_OCRBH:
        timer.tempreg = (uint8_t)ftimer->ocrb;
        return (uint8_t)(ftimer->ocrb >> 8);
    case REG_ICRH:
        timer.tempreg = (uint8_t)ftimer->icr;
        return (uint8_t)(ftimer->icr >> 8);
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

constexpr frt_step_table_type FRT_STEP_TABLE_GENERIC = {3, 7, 31, 1};

constexpr frt_step_table_type FRT_STEP_TABLE_MK1 = {3, 7, 31, 3};

constexpr timer_step_table_type TIMER_STEP_TABLE_GENERIC = {0, 7, 63, 1023, 0, 1, 1, 1};

constexpr timer_step_table_type TIMER_STEP_TABLE_MK1 = {0, 7, 63, 1023, 0, 3, 3, 3};

void TIMER_ClockFrt(mcu_timer_t& timer, int frt_id)
{
    frt_t& frt = timer.frt[frt_id];

    if (timer.cycles & timer.frt_step_table[frt.tcr & 3])
    {
        return;
    }

    uint32_t   value  = frt.frc;
    const bool matcha = value == frt.ocra;
    const bool matchb = value == frt.ocrb;
    if ((frt.tcsr & 1) != 0 && matcha) // CCLRA
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
        frt.tcsr |= 0x10;
    if (matcha)
        frt.tcsr |= 0x20;
    if (matchb)
        frt.tcsr |= 0x40;
    if ((frt.tcr & 0x10) != 0 && (frt.tcsr & 0x10) != 0)
        MCU_Interrupt_SetRequest(*timer.mcu, (MCU_Interrupt_Source)(INTERRUPT_SOURCE_FRT0_FOVI + frt_id * 4), 1);
    if ((frt.tcr & 0x20) != 0 && (frt.tcsr & 0x20) != 0)
        MCU_Interrupt_SetRequest(*timer.mcu, (MCU_Interrupt_Source)(INTERRUPT_SOURCE_FRT0_OCIA + frt_id * 4), 1);
    if ((frt.tcr & 0x40) != 0 && (frt.tcsr & 0x40) != 0)
        MCU_Interrupt_SetRequest(*timer.mcu, (MCU_Interrupt_Source)(INTERRUPT_SOURCE_FRT0_OCIB + frt_id * 4), 1);
}

void TIMER_ClockTimer(mcu_timer_t& timer)
{
    if (timer.cycles & timer.timer_step_table[timer.tcr & 7])
    {
        return;
    }

    uint32_t   value  = timer.tcnt;
    const bool matcha = value == timer.tcora;
    const bool matchb = value == timer.tcorb;
    if ((timer.tcr & 24) == 8 && matcha)
    {
        value = 0;
    }
    else if ((timer.tcr & 24) == 16 && matchb)
    {
        value = 0;
    }
    else
    {
        ++value;
    }
    const bool of = (value >> 8) & 1;
    timer.tcnt    = (uint8_t)value;

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

void TIMER_Clock(mcu_timer_t& timer, uint64_t cycles)
{
    while (timer.cycles * 2 < cycles) // FIXME
    {
        for (int i = 0; i < 3; i++)
        {
            TIMER_ClockFrt(timer, i);
        }

        TIMER_ClockTimer(timer);

        ++timer.cycles;
    }
}

void TIMER_NotifyRomsetChange(mcu_timer_t& timer)
{
    const bool mk1 = timer.mcu->is_mk1;
    const auto& FRT_STEP_TABLE = mk1 ? FRT_STEP_TABLE_MK1 : FRT_STEP_TABLE_GENERIC;
    const auto& TIMER_STEP_TABLE = mk1 ? TIMER_STEP_TABLE_MK1 : TIMER_STEP_TABLE_GENERIC;
    memcpy(timer.frt_step_table, FRT_STEP_TABLE, sizeof(frt_step_table_type));
    memcpy(timer.timer_step_table, TIMER_STEP_TABLE, sizeof(timer_step_table_type));
}
