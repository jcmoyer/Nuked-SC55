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
#include "submcu.h"
#include <filesystem>
#include <memory>
#include <span>
#include <string_view>
#include <vector>

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

// Symbolic name for where a rom will be mapped once loaded
enum class EMU_RomMapLocation
{
    ROM1,
    ROM2,
    SMROM,
    WAVEROM1,
    WAVEROM2,
    WAVEROM3,
    WAVEROM_CARD,
    WAVEROM_EXP,

    // do not reorder these
    COUNT,
    NONE = COUNT,
};

struct EMU_AllRomsetInfo;

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

    // Loads roms according to hardcoded filenames. This function follows the same conventions as
    // `EMU_DetectRomsetByFilename` and will load roms the same way as upstream.
    bool LoadRomsByFilename(Romset romset, const std::filesystem::path& base_path);

    // Loads roms based on the metadata in `all_info`. This structure can be manually populated to override filenames or
    // it can be retreived from `EMU_DetectRomsetsByHash`. If the requested romset info contains `rom_data` (e.g. from a
    // call to `EMU_DetectRomsetsByHash`) that data will be copied into emulator memory. If the `rom_data` is empty,
    // this function will load it from `rom_path`. At least one of these must be present to load a rom. If both are
    // empty, nothing will be loaded for that map location.
    //
    // It is recommended to check if the romset has all the necessary roms by first calling
    // `EMU_IsCompleteRomset(all_info, romset)`.
    bool LoadRomsByInfo(Romset romset, const EMU_AllRomsetInfo& all_info);

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

    std::span<uint8_t> MapBuffer(EMU_RomMapLocation location);

    bool LoadRom(EMU_RomMapLocation location, std::span<const uint8_t> source);

private:
    std::unique_ptr<mcu_t>       m_mcu;
    std::unique_ptr<submcu_t>    m_sm;
    std::unique_ptr<mcu_timer_t> m_timer;
    std::unique_ptr<lcd_t>       m_lcd;
    std::unique_ptr<pcm_t>       m_pcm;
    EMU_Options                  m_options;
};

// For a single romset, this structure maps each rom in the set to a filename on disk and that file's contents.
struct EMU_RomsetInfo
{
    // Array indexed by EMU_RomMapLocation
    std::filesystem::path rom_paths[(size_t)EMU_RomMapLocation::COUNT]{};
    std::vector<uint8_t>  rom_data[(size_t)EMU_RomMapLocation::COUNT]{};

    // Release all rom_data for all roms in this romset.
    void PurgeRomData();

    // Returns true if at least one of `rom_path` or `rom_data` is populated for `location`.
    bool HasRom(EMU_RomMapLocation location) const;
};

// Contains EMU_RomsetInfo for all supported romsets.
struct EMU_AllRomsetInfo
{
    // Array indexed by Romset
    EMU_RomsetInfo romsets[ROMSET_COUNT]{};

    // Release all rom_data for all romsets.
    void PurgeRomData();
};

// Picks a romset based on filenames contained in `base_path`. This function requires every rom in the romset to have a
// specific filename in order for the romset to be considered. Consult the `roms` constant in `emu.cpp` for the exact
// filename requirements. This function will either return the first complete romset it finds or Romset::MK2 if none are
// found.
Romset EMU_DetectRomsetByFilename(const std::filesystem::path& base_path);

// Scans files in `base_path` for roms by hashing them. The locations of each rom will be made available in `info`.
// Unlike the above function, this will return *all* romsets in `base_path`.
//
// If any of the rom locations in `all_info` are already populated with a path or data, this function will not
// overwrite them.
bool EMU_DetectRomsetsByHash(const std::filesystem::path& base_path, EMU_AllRomsetInfo& all_info);

const char* EMU_RomsetName(Romset romset);
bool EMU_ParseRomsetName(std::string_view name, Romset& romset);
std::span<const char*> EMU_GetParsableRomsetNames();

// Returns true if `all_info` contains all the files required to load `romset`. Missing roms will be reported in
// `missing`.
bool EMU_IsCompleteRomset(const EMU_AllRomsetInfo&         all_info,
                          Romset                           romset,
                          std::vector<EMU_RomMapLocation>* missing = nullptr);

// Picks the first complete romset in `all_info` and writes it to `out_romset`. If multiple romsets are present, the one
// returned is unspecified. Returns if successful, or false if there are no complete romsets.
bool EMU_PickCompleteRomset(const EMU_AllRomsetInfo& all_info, Romset& out_romset);

// Returns true if `location` represents a waverom location.
bool EMU_IsWaverom(EMU_RomMapLocation location);

// Returns `location` as a string.
const char* EMU_RomMapLocationToString(EMU_RomMapLocation location);
