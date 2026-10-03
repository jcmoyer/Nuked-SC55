/*
 * Copyright (C) 2024-2026 J.C. Moyer
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

#include "emu.h"
#include "diagnostics.h"
#include "lcd.h"
#include "mcu.h"
#include "mcu_timer.h"
#include "pcm.h"
#include "submcu.h"
#include "memory.h"
#include <bit>
#include <fstream>
#include <span>
#include <vector>

Emulator::~Emulator()
{
    SaveNVRAM();
}

bool Emulator::Init(const EMU_Options& options)
{
    m_options = options;

    if (!options.memory)
    {
        return false;
    }

    if (!ConstructEmulator(*options.memory, options.instance_id))
    {
        return false;
    }

    m_mcu   = (mcu_t*)GetMCUPtr(*options.memory, options.instance_id);
    m_sm    = (submcu_t*)GetSMPtr(*options.memory, options.instance_id);
    m_pcm   = (pcm_t*)GetPCMPtr(*options.memory, options.instance_id);
    m_lcd   = (lcd_t*)GetLCDPtr(*options.memory, options.instance_id);
    m_timer = (mcu_timer_t*)GetTimerPtr(*options.memory, options.instance_id);

    // TODO: timer must be initialized before mcu, because MCU_Init
    // sets romset from memory and then calls timer_notifyromsetchange
    // which accesses timer.mcu
    TIMER_Init(*m_timer, *m_mcu);
    MCU_Init(*m_mcu, *options.memory, *m_sm, *m_pcm, *m_timer, *m_lcd);
    SM_Init(*m_sm, *options.memory, *m_mcu);
    PCM_Init(*m_pcm, *options.memory, *m_mcu);
    LCD_Init(*m_lcd, *m_mcu);
    m_lcd->backend = options.lcd_backend;

    return true;
}

void Emulator::Reset()
{
    MCU_Reset(*m_mcu);
    SM_Reset(*m_sm);
}

bool Emulator::StartLCD()
{
    return LCD_Start(*m_lcd);
}

void Emulator::StopLCD()
{
    LCD_Stop(*m_lcd);
}

void Emulator::SetSampleCallback(mcu_sample_callback callback, void* userdata)
{
    m_mcu->callback_userdata = userdata;
    m_mcu->sample_callback = callback;
}

bool Emulator::LoadRoms(Romset romset, const RomsetInfo& info, RomLocationSet* loaded)
{
    if (loaded)
    {
        loaded->fill(false);
    }

    MCU_SetRomset(GetMCU(), romset);

    for (size_t i = 0; i < ROMLOCATION_COUNT; ++i)
    {
        const RomLocation location = (RomLocation)i;

        // rom_data should be populated at this point
        // if it isn't, then there isn't a rom for this location
        if (info.rom_data[i].empty())
        {
            continue;
        }

        if (!LoadRom(location, info.rom_data[i]))
        {
            return false;
        }

        if (loaded)
        {
            (*loaded)[i] = true;
        }
    }

    if (m_mcu->is_jv880)
    {
        LoadNVRAM();
    }

    MCU_PatchROM(*m_mcu);

    return true;
}

void Emulator::PostMIDI(uint8_t byte)
{
    MCU_PostUART(*m_mcu, byte);
}

void Emulator::PostMIDI(std::span<const uint8_t> data)
{
    for (uint8_t byte : data)
    {
        PostMIDI(byte);
    }
}

constexpr uint8_t GM_RESET_SEQ[] = { 0xF0, 0x7E, 0x7F, 0x09, 0x01, 0xF7 };
constexpr uint8_t GS_RESET_SEQ[] = { 0xF0, 0x41, 0x10, 0x42, 0x12, 0x40, 0x00, 0x7F, 0x00, 0x41, 0xF7 };

void Emulator::PostSystemReset(EMU_SystemReset reset)
{
    switch (reset)
    {
        case EMU_SystemReset::NONE:
            // explicitly do nothing
            break;
        case EMU_SystemReset::GS_RESET:
            PostMIDI(GS_RESET_SEQ);
            break;
        case EMU_SystemReset::GM_RESET:
            PostMIDI(GM_RESET_SEQ);
            break;
    }
}

void Emulator::Step()
{
    MCU_Step(*m_mcu);
}

void Emulator::SaveNVRAM()
{
    // emulator was constructed, but never init
    if (!m_mcu)
    {
        return;
    }

    if (!m_options.nvram_filename.empty() && m_mcu->is_jv880)
    {
        std::ofstream file(m_options.nvram_filename, std::ios::binary);
        file.write((const char*)m_mcu->nvram, NVRAM_SIZE);
    }
}

void Emulator::LoadNVRAM()
{
    if (!m_options.nvram_filename.empty() && m_mcu->is_jv880)
    {
        std::ifstream file(m_options.nvram_filename, std::ios::binary);
        file.read((char*)m_mcu->nvram, NVRAM_SIZE);
    }
}

std::span<uint8_t> Emulator::MapBuffer(RomLocation location)
{
    auto span = ::MapBuffer(*m_options.memory, location);
    if (!span.data())
    {
        Diag_Printf(Diag_Category::Error, "MapBuffer returned null for location %d\n", (int)location);
        std::abort();
    }
    return span;
}

bool Emulator::LoadRom(RomLocation location, std::span<const uint8_t> source)
{
    auto buffer = MapBuffer(location);

    if (buffer.size() < source.size())
    {
        Diag_Printf(Diag_Category::Error,
                    "rom for %s is too large; max size is %d bytes\n",
                    ToCString(location),
                    (int)buffer.size());
        return false;
    }

    if (location == RomLocation::ROM2)
    {
        if (!std::has_single_bit(source.size()))
        {
            Diag_Printf(Diag_Category::Error, "%s requires a power-of-2 size\n", ToCString(location));
            return false;
        }
        GetMCU().rom2_mask = (uint32_t)source.size() - 1;
    }

    std::copy(source.begin(), source.end(), buffer.begin());

    return true;
}
