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

#include "lcd.h"
#include "mcu.h"
#include "mcu_timer.h"
#include "pcm.h"
#include "rom.h"
#include "rom_io.h"
#include "submcu.h"
#include <filesystem>
#include <memory>
#include <span>

struct EMU_Options
{
    // The backend provided here will receive callbacks from the emulator.
    // If left null, LCD processing will be skipped.
    LCD_Backend* lcd_backend = nullptr;

    // If not empty, nvram will be saved to and loaded from here. JV-880 only.
    std::filesystem::path nvram_filename;
};

enum class EMU_SystemReset {
    NONE,
    GS_RESET,
    GM_RESET,
};

struct Emulator {
public:
    Emulator() = default;

    virtual ~Emulator();

    Emulator& operator=(Emulator&&) = default;
    Emulator(Emulator&&)            = default;

    Emulator(const Emulator&)            = delete;
    Emulator& operator=(const Emulator&) = delete;

    bool Init(const EMU_Options& options);

    // Should be called after loading roms
    void Reset();

    // Should be called after reset. Has no effect if the `lcd_backend` passed to `Init` was null.
    bool StartLCD();

    void StopLCD();

    void SetSampleCallback(mcu_sample_callback callback, void* userdata);

    // Loads roms from buffers referenced by `info`. If the slot for a rom in `info` has a non-empty `rom_data`, it
    // will be loaded even if the romset doesn't require it.
    //
    // It is unspecified whether or not the emulator will copy `rom_data`. `info` should outlive the emulator instance.
    //
    // For roms that were successfully loaded, this function will set their corresponding index in `loaded` to true if
    // `loaded` is non-null.
    //
    // It is recommended to check if the romset has all the necessary roms by first calling `IsCompleteRomset(info,
    // romset)`.
    bool LoadRoms(Romset romset, const RomsetInfo& info, RomLocationSet* loaded = nullptr);

    void PostMIDI(uint8_t data_byte);
    void PostMIDI(std::span<const uint8_t> data);

    void PostSystemReset(EMU_SystemReset reset);

    void Step();

    mcu_t& GetMCU() { return *m_mcu; }
    pcm_t& GetPCM() { return *m_pcm; }
    lcd_t& GetLCD() { return *m_lcd; }

private:
    void SaveNVRAM();
    void LoadNVRAM();

    std::span<uint8_t> MapBuffer(RomLocation location);

    bool LoadRom(RomLocation location, std::span<const uint8_t> source);

private:
    std::unique_ptr<mcu_t>       m_mcu;
    std::unique_ptr<submcu_t>    m_sm;
    std::unique_ptr<mcu_timer_t> m_timer;
    std::unique_ptr<lcd_t>       m_lcd;
    std::unique_ptr<pcm_t>       m_pcm;
    EMU_Options                  m_options;
};
