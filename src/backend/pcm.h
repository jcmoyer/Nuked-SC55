/*
 * Copyright (C) 2021, 2024 nukeykt
 * Modified by J.C. Moyer
 * Original source file: src/pcm.h
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
 *  Thanks:
 *      John McMaster (https://siliconprawn.org):
 *          PCM chip decap
 *
 */
#pragma once

#include <cstdint>

struct mcu_t;

struct PCM_Config
{
    // config_reg_3c
    uint32_t orval        = 0;
    int      dac_mask     = 0; // unused
    uint8_t  noise_mask   = 0;
    uint8_t  write_mask   = 0;
    bool     oversampling = false;

    // config_reg_3d
    // important that this starts at 1, see derivation in PCM_Write
    uint8_t reg_slots = 1;
};

struct pcm_t
{
    uint32_t ram1[32][8]{};
    uint16_t ram2[32][16]{};
    mcu_t*   mcu                 = nullptr;
    uint64_t cycles              = 0;
    uint32_t voice_mask          = 0; // same size as voice_mask_pending
    uint32_t voice_mask_pending  = 0; // 28 bits wide?
    uint32_t write_latch         = 0; // 20 bits wide?
    uint32_t read_latch          = 0; // 20 bits wide?
    uint32_t wave_read_address   = 0;
    uint16_t tv_counter          = 0; // 14 bits wide?
    uint8_t  wave_byte_latch     = 0;
    uint8_t  select_channel      = 0; // 5 bits wide?
    uint8_t  config_reg_3c       = 0; // SC55:c3 JV880:c0
    uint8_t  config_reg_3d       = 0;
    uint8_t  irq_channel         = 0; // range 1..32
    bool     irq_assert          = 0;
    bool     voice_mask_updating = false;
    bool     nfs                 = false;
    int32_t  accum_l             = 0;
    int32_t  accum_r             = 0;
    int32_t  rcsum[2]{};

    PCM_Config config{};

    uint16_t eram[0x4000]{};

    uint8_t waverom1[0x200000]{};
    uint8_t waverom2[0x200000]{};
    uint8_t waverom3[0x100000]{};
    uint8_t waverom_card[0x200000]{};
    uint8_t waverom_exp[0x800000]{};

    bool enable_oversampling = true;
};

void PCM_Write(pcm_t& pcm, uint32_t address, uint8_t data);
uint8_t PCM_Read(pcm_t& pcm, uint32_t address);
void PCM_Init(pcm_t& pcm, mcu_t& mcu);
void PCM_Update(pcm_t& pcm, uint64_t cycles);
uint32_t PCM_GetOutputFrequency(const pcm_t& pcm);
void PCM_GetConfig(PCM_Config& config, uint8_t config_byte);
