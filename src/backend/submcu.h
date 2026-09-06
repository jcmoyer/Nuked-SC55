/*
 * Copyright (C) 2021, 2024 nukeykt
 * Original source file: src/submcu.h
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

#include <cstdint>

struct mcu_t;

enum {
    SM_STATUS_C = 1,
    SM_STATUS_Z = 2,
    SM_STATUS_I = 4,
    SM_STATUS_D = 8,
    SM_STATUS_B = 16,
    SM_STATUS_T = 32,
    SM_STATUS_V = 64,
    SM_STATUS_N = 128
};

struct submcu_t {
    uint16_t pc = 0;
    uint8_t a = 0;
    uint8_t x = 0;
    uint8_t y = 0;
    uint8_t s = 0;
    uint8_t sr = 0;
    uint64_t cycles = 0;
    uint8_t sleep = 0;
    mcu_t* mcu = nullptr;
    uint8_t rom[4096]{};

    uint8_t ram[128]{};
    uint8_t shared_ram[192]{};
    uint8_t access[0x18]{};

    uint8_t p0_dir = 0;
    uint8_t p1_dir = 0;

    uint8_t device_mode[32]{};
    uint8_t cts = 0;

    uint64_t timer_cycles = 0;
    uint8_t timer_prescaler = 0;
    uint8_t timer_counter = 0;

    uint8_t uart_rx_gotbyte = 0;
};

void SM_Init(submcu_t& sm, mcu_t& mcu);
void SM_Reset(submcu_t& sm);
void SM_Update(submcu_t& sm, uint64_t cycles);
void SM_SysWrite(submcu_t& sm, uint32_t address, uint8_t data);
uint8_t SM_SysRead(submcu_t& sm, uint32_t address);
void SM_PostUART(submcu_t& sm, uint8_t data);
