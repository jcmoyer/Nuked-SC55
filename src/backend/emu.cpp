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
#include "emu.h"
#include "lcd.h"
#include "mcu.h"
#include "mcu_timer.h"
#include "pcm.h"
#include "submcu.h"
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

    try
    {
        m_mcu   = std::make_unique<mcu_t>();
        m_sm    = std::make_unique<submcu_t>();
        m_timer = std::make_unique<mcu_timer_t>();
        m_lcd   = std::make_unique<lcd_t>();
        m_pcm   = std::make_unique<pcm_t>();
    }
    catch (const std::bad_alloc&)
    {
        m_mcu.reset();
        m_sm.reset();
        m_timer.reset();
        m_lcd.reset();
        m_pcm.reset();
        return false;
    }

    MCU_Init(*m_mcu, *m_sm, *m_pcm, *m_timer, *m_lcd);
    SM_Init(*m_sm, *m_mcu);
    PCM_Init(*m_pcm, *m_mcu);
    TIMER_Init(*m_timer, *m_mcu);
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

bool Emulator::LoadRoms(Romset romset, const AllRomsetInfo& all_info, RomLocationSet* loaded)
{
    if (loaded)
    {
        loaded->fill(false);
    }

    MCU_SetRomset(GetMCU(), romset);

    const RomsetInfo& info = all_info.romsets[(size_t)romset];

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
    switch (location)
    {
    case RomLocation::ROM1:
        return GetMCU().rom1;
    case RomLocation::ROM2:
        return GetMCU().rom2;
    case RomLocation::WAVEROM1:
        return GetPCM().waverom1;
    case RomLocation::WAVEROM2:
        return GetPCM().waverom2;
    case RomLocation::WAVEROM3:
        return GetPCM().waverom3;
    case RomLocation::WAVEROM_CARD:
        return GetPCM().waverom_card;
    case RomLocation::WAVEROM_EXP:
        return GetPCM().waverom_exp;
    case RomLocation::SMROM:
        return m_sm->rom;
    }
    fprintf(stderr, "FATAL: MapBuffer called with invalid location %d\n", (int)location);
    std::abort();
}

bool Emulator::LoadRom(RomLocation location, std::span<const uint8_t> source)
{
    auto buffer = MapBuffer(location);

    if (buffer.size() < source.size())
    {
        fprintf(stderr,
                "FATAL: rom for %s is too large; max size is %d bytes\n",
                ToCString(location),
                (int)buffer.size());
        return false;
    }

    if (location == RomLocation::ROM2)
    {
        if (!std::has_single_bit(source.size()))
        {
            fprintf(stderr, "FATAL: %s requires a power-of-2 size\n", ToCString(location));
            return false;
        }
        GetMCU().rom2_mask = (uint32_t)source.size() - 1;
    }

    std::copy(source.begin(), source.end(), buffer.begin());

    return true;
}

