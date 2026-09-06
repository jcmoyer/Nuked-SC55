/*
 * Copyright (C) 2021, 2024 nukeykt
 * Modified by J.C. Moyer
 * Original source file: src/mcu_interrupt.cpp
 *
 * This file is part of Nuked-SC55.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#include "mcu_interrupt.h"
#include "mcu.h"

void MCU_Interrupt_Start(mcu_t& mcu, int32_t mask)
{
    MCU_PushStack(mcu, mcu.pc);
    MCU_PushStack(mcu, mcu.cp);
    MCU_PushStack(mcu, mcu.sr);
    mcu.sr &= ~STATUS_T;
    if (mask >= 0)
    {
        mcu.sr &= ~STATUS_INT_MASK;
        mcu.sr |= (uint16_t)(mask << 8);
    }
    mcu.sleep = 0;
}

void MCU_Interrupt_SetRequest(mcu_t& mcu, MCU_Interrupt_Source interrupt, bool value)
{
    if (value)
    {
        mcu.interrupt_pending.Include(interrupt);
    }
    else
    {
        mcu.interrupt_pending.Exclude(interrupt);
    }
}

void MCU_Interrupt_Exception(mcu_t& mcu, MCU_Exception_Source exception)
{
#if 0
    if (interrupt == INTERRUPT_SOURCE_IRQ0 && (mcu.dev_register[DEV_P1CR] & 0x20) == 0)
        return;
    if (interrupt == INTERRUPT_SOURCE_IRQ1 && (mcu.dev_register[DEV_P1CR] & 0x40) == 0)
        return;
#endif
    mcu.exception_pending = exception;
}

void MCU_Interrupt_TRAPA(mcu_t& mcu, uint8_t vector)
{
    mcu.trapa_pending.Include(vector);
}

void MCU_Interrupt_StartVector(mcu_t& mcu, uint32_t vector, int32_t mask)
{
    uint32_t address = MCU_GetVectorAddress(mcu, vector);
    MCU_Interrupt_Start(mcu, mask);
    mcu.cp = (uint8_t)(address >> 16);
    mcu.pc = (uint16_t)address;
}

static void MCU_Interrupt_GetVL(const mcu_t& mcu, uint32_t source, int32_t& vector, int32_t& level)
{
    switch (source)
    {
    case INTERRUPT_SOURCE_IRQ0:
        if ((mcu.dev_register[DEV_P1CR] & 0x20) == 0)
            break;
        vector = VECTOR_IRQ0;
        level  = (mcu.dev_register[DEV_IPRA] >> 4) & 7;
        break;
    case INTERRUPT_SOURCE_IRQ1:
        if ((mcu.dev_register[DEV_P1CR] & 0x40) == 0)
            break;
        vector = VECTOR_IRQ1;
        level  = (mcu.dev_register[DEV_IPRA] >> 0) & 7;
        break;
    case INTERRUPT_SOURCE_FRT0_OCIA:
        vector = VECTOR_INTERNAL_INTERRUPT_94;
        level  = (mcu.dev_register[DEV_IPRB] >> 4) & 7;
        break;
    case INTERRUPT_SOURCE_FRT0_OCIB:
        vector = VECTOR_INTERNAL_INTERRUPT_98;
        level  = (mcu.dev_register[DEV_IPRB] >> 4) & 7;
        break;
    case INTERRUPT_SOURCE_FRT0_FOVI:
        vector = VECTOR_INTERNAL_INTERRUPT_9C;
        level  = (mcu.dev_register[DEV_IPRB] >> 4) & 7;
        break;
    case INTERRUPT_SOURCE_FRT1_OCIA:
        vector = VECTOR_INTERNAL_INTERRUPT_A4;
        level  = (mcu.dev_register[DEV_IPRB] >> 0) & 7;
        break;
    case INTERRUPT_SOURCE_FRT1_OCIB:
        vector = VECTOR_INTERNAL_INTERRUPT_A8;
        level  = (mcu.dev_register[DEV_IPRB] >> 0) & 7;
        break;
    case INTERRUPT_SOURCE_FRT1_FOVI:
        vector = VECTOR_INTERNAL_INTERRUPT_AC;
        level  = (mcu.dev_register[DEV_IPRB] >> 0) & 7;
        break;
    case INTERRUPT_SOURCE_FRT2_OCIA:
        vector = VECTOR_INTERNAL_INTERRUPT_B4;
        level  = (mcu.dev_register[DEV_IPRC] >> 4) & 7;
        break;
    case INTERRUPT_SOURCE_FRT2_OCIB:
        vector = VECTOR_INTERNAL_INTERRUPT_B8;
        level  = (mcu.dev_register[DEV_IPRC] >> 4) & 7;
        break;
    case INTERRUPT_SOURCE_FRT2_FOVI:
        vector = VECTOR_INTERNAL_INTERRUPT_BC;
        level  = (mcu.dev_register[DEV_IPRC] >> 4) & 7;
        break;
    case INTERRUPT_SOURCE_TIMER_CMIA:
        vector = VECTOR_INTERNAL_INTERRUPT_C0;
        level  = (mcu.dev_register[DEV_IPRC] >> 0) & 7;
        break;
    case INTERRUPT_SOURCE_TIMER_CMIB:
        vector = VECTOR_INTERNAL_INTERRUPT_C4;
        level  = (mcu.dev_register[DEV_IPRC] >> 0) & 7;
        break;
    case INTERRUPT_SOURCE_TIMER_OVI:
        vector = VECTOR_INTERNAL_INTERRUPT_C8;
        level  = (mcu.dev_register[DEV_IPRC] >> 0) & 7;
        break;
    case INTERRUPT_SOURCE_ANALOG:
        vector = VECTOR_INTERNAL_INTERRUPT_E0;
        level  = (mcu.dev_register[DEV_IPRD] >> 0) & 7;
        break;
    case INTERRUPT_SOURCE_UART_RX:
        vector = VECTOR_INTERNAL_INTERRUPT_D4;
        level  = (mcu.dev_register[DEV_IPRD] >> 4) & 7;
        break;
    case INTERRUPT_SOURCE_UART_TX:
        vector = VECTOR_INTERNAL_INTERRUPT_D8;
        level  = (mcu.dev_register[DEV_IPRD] >> 4) & 7;
        break;
    default:
        break;
    }
}

void MCU_Interrupt_Handle(mcu_t& mcu)
{
#if 0
    if (mcu.cycles % 2000 == 0 && mcu.sleep)
    {
        MCU_Interrupt_StartVector(VECTOR_INTERNAL_INTERRUPT_94);
        return;
    }
    if (mcu.cycles % 2000 == 1000 && mcu.sleep)
    {
        MCU_Interrupt_StartVector(VECTOR_INTERNAL_INTERRUPT_A4);
        return;
    }
    if (mcu.cycles % 2000 == 1500 && mcu.sleep)
    {
        MCU_Interrupt_StartVector(VECTOR_INTERNAL_INTERRUPT_B4);
        return;
    }
#endif
    if (auto it = mcu.trapa_pending.begin(); it != mcu.trapa_pending.end())
    {
        mcu.trapa_pending.Exclude(*it);
        MCU_Interrupt_StartVector(mcu, VECTOR_TRAPA_0 + (*it), -1);
        return;
    }
    if (mcu.exception_pending >= 0)
    {
        switch (mcu.exception_pending)
        {
            case EXCEPTION_SOURCE_ADDRESS_ERROR:
                MCU_Interrupt_StartVector(mcu, VECTOR_ADDRESS_ERROR, -1);
                break;
            case EXCEPTION_SOURCE_INVALID_INSTRUCTION:
                MCU_Interrupt_StartVector(mcu, VECTOR_INVALID_INSTRUCTION, -1);
                break;
            case EXCEPTION_SOURCE_TRACE:
                MCU_Interrupt_StartVector(mcu, VECTOR_TRACE, -1);
                break;

        }
        mcu.exception_pending = (MCU_Exception_Source)-1;
        return;
    }
    if (mcu.interrupt_pending.Contains(INTERRUPT_SOURCE_NMI))
    {
        // mcu.interrupt_pending[INTERRUPT_SOURCE_NMI] = 0;
        MCU_Interrupt_StartVector(mcu, VECTOR_NMI, 7);
        return;
    }
    uint32_t mask = (mcu.sr >> 8) & 7;
    for (MCU_Interrupt_Source source : mcu.interrupt_pending)
    {
        int32_t vector = -1;
        int32_t level = 0;
        MCU_Interrupt_GetVL(mcu, source, vector, level);
        if ((int32_t)mask < level)
        {
            // mcu.interrupt_pending[INTERRUPT_SOURCE_NMI] = 0;
            MCU_Interrupt_StartVector(mcu, (uint32_t)vector, level);
            return;
        }
    }
}
