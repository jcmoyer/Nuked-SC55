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

// Read/write 16-bit FRTs
void TIMER_Write(mcu_timer_t& timer, uint32_t address, uint8_t data);
uint8_t TIMER_Read(mcu_timer_t& timer, uint32_t address);

// Read/write 8-bit timer
void TIMER2_Write(mcu_timer_t& timer, uint32_t address, uint8_t data);
uint8_t TIMER_Read2(mcu_timer_t& timer, uint32_t address);

// Update all timers and trigger interrupts
void TIMER_Clock(mcu_timer_t& timer, uint64_t cycles);

void TIMER_NotifyRomsetChange(mcu_timer_t& timer);
