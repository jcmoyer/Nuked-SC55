/*
 * Copyright (C) 2021, 2024 nukeykt
 * Modified by J.C. Moyer
 * Original source file: src/mcu_timer.h
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
#pragma once

#include <array>
#include <cstdint>

struct mcu_t;

using FRT_Step_Table = std::array<uint8_t, 4>;
using TMR_Step_Table = std::array<uint16_t, 8>;

// 16-bit free running timers
struct frt_t
{
    uint8_t  tcr       = 0;
    uint8_t  tcsr      = 0;
    uint16_t frc       = 0;
    uint16_t ocra      = 0;
    uint16_t ocrb      = 0;
    uint16_t icr       = 0;
    uint8_t  status_rd = 0; // not an actual FRT register
};

// 8-bit timer
struct tmr_t
{
    uint8_t tcr       = 0;
    uint8_t tcsr      = 0;
    uint8_t tcora     = 0;
    uint8_t tcorb     = 0;
    uint8_t tcnt      = 0;
    uint8_t status_rd = 0;
};

struct mcu_timer_t
{
    uint64_t cycles = 0;
    FRT_Step_Table frt_step_table;
    TMR_Step_Table tmr_step_table;

    mcu_t* mcu = nullptr;
    frt_t   frt[3]{};
    tmr_t   tmr{};
    uint8_t tempreg = 0;
};

void TIMER_Init(mcu_timer_t& timer, mcu_t& mcu);
void TIMER_Reset(mcu_timer_t& timer);

// Read/write 16-bit FRTs
void TIMER_Write(mcu_timer_t& timer, uint32_t address, uint8_t data);
uint8_t TIMER_Read(mcu_timer_t& timer, uint32_t address);

// Read/write 8-bit timer
void TIMER2_Write(mcu_timer_t& timer, uint32_t address, uint8_t data);
uint8_t TIMER_Read2(mcu_timer_t& timer, uint32_t address);

// Update all timers and trigger interrupts
void TIMER_Clock(mcu_timer_t& timer, uint64_t cycles);

void TIMER_NotifyRomsetChange(mcu_timer_t& timer);
