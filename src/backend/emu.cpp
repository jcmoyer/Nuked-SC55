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
#include "cast.h"
#include "lcd.h"
#include "mcu.h"
#include "mcu_timer.h"
#include "pcm.h"
#include "submcu.h"
#include <fstream>
#include <span>
#include <string>
#include <vector>

extern "C"
{
#include "sha/sha.h"
}

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

const char* rs_name[(size_t)ROMSET_COUNT] = {
    "SC-55mk2",
    "SC-55st",
    "SC-55mk1",
    "CM-300/SCC-1",
    "JV-880",
    "SCB-55",
    "RLP-3237",
    "SC-155",
    "SC-155mk2"
};

const char* rs_name_simple[(size_t)ROMSET_COUNT] = {
    "mk2",
    "st",
    "mk1",
    "cm300",
    "jv880",
    "scb55",
    "rlp3237",
    "sc155",
    "sc155mk2"
};

constexpr int ROM_SET_N_FILES = 6;

const char* roms[(size_t)ROMSET_COUNT][ROM_SET_N_FILES] =
{
    {
        "rom1.bin",
        "rom2.bin",
        "waverom1.bin",
        "waverom2.bin",
        "rom_sm.bin",
        "",
    },

    {
        "rom1.bin",
        "rom2_st.bin",
        "waverom1.bin",
        "waverom2.bin",
        "rom_sm.bin",
        "",
    },

    {
        "sc55_rom1.bin",
        "sc55_rom2.bin",
        "sc55_waverom1.bin",
        "sc55_waverom2.bin",
        "sc55_waverom3.bin",
        "",
    },

    {
        "cm300_rom1.bin",
        "cm300_rom2.bin",
        "cm300_waverom1.bin",
        "cm300_waverom2.bin",
        "cm300_waverom3.bin",
        "",
    },

    {
        "jv880_rom1.bin",
        "jv880_rom2.bin",
        "jv880_waverom1.bin",
        "jv880_waverom2.bin",
        "jv880_waverom_expansion.bin",
        "jv880_waverom_pcmcard.bin",
    },

    {
        "scb55_rom1.bin",
        "scb55_rom2.bin",
        "scb55_waverom1.bin",
        "scb55_waverom2.bin",
        "",
        "",
    },

    {
        "rlp3237_rom1.bin",
        "rlp3237_rom2.bin",
        "rlp3237_waverom1.bin",
        "",
        "",
        "",
    },

    {
        "sc155_rom1.bin",
        "sc155_rom2.bin",
        "sc155_waverom1.bin",
        "sc155_waverom2.bin",
        "sc155_waverom3.bin",
        "",
    },

    {
        "rom1.bin",
        "rom2.bin",
        "waverom1.bin",
        "waverom2.bin",
        "rom_sm.bin",
        "",
    },
};

void unscramble(const uint8_t *src, uint8_t *dst, int len)
{
    for (int i = 0; i < len; i++)
    {
        int address = i & ~0xfffff;
        static const int aa[] = {
            2, 0, 3, 4, 1, 9, 13, 10, 18, 17, 6, 15, 11, 16, 8, 5, 12, 7, 14, 19
        };
        for (int j = 0; j < 20; j++)
        {
            if (i & (1 << j))
                address |= 1<<aa[j];
        }
        uint8_t srcdata = src[address];
        uint8_t data = 0;
        static const int dd[] = {
            2, 0, 4, 5, 7, 6, 3, 1
        };
        for (int j = 0; j < 8; j++)
        {
            if (srcdata & (1 << dd[j]))
                data |= 1<<j;
        }
        dst[i] = data;
    }
}

bool EMU_ReadAllBytes(const std::filesystem::path& filename, std::vector<uint8_t>& buffer)
{
    std::ifstream input(filename, std::ios::binary);

    if (!input)
    {
        return false;
    }

    input.seekg(0, std::ios::end);
    std::streamoff byte_count = input.tellg();
    input.seekg(0, std::ios::beg);

    buffer.resize(RangeCast<size_t>(byte_count));

    input.read((char*)buffer.data(), RangeCast<std::streamsize>(byte_count));

    return input.good();
}

// This is wrapped in a structure so we can return it from a function.
struct SHA256Digest
{
    uint8_t bytes[SHA256HashSize];

    SHA256Digest() = default;

    SHA256Digest(const uint8_t (&from_bytes)[SHA256HashSize])
    {
        memcpy(bytes, from_bytes, SHA256HashSize);
    }

    const uint8_t* begin() const
    {
        return &bytes[0];
    }

    const uint8_t* end() const
    {
        return &bytes[0] + SHA256HashSize;
    }
};

constexpr bool operator==(const SHA256Digest& a, const SHA256Digest& b)
{
    return std::equal(a.begin(), a.end(), b.begin(), b.end());
}

constexpr uint8_t HexValue(char x)
{
    if (x >= '0' && x <= '9')
    {
        return x - '0';
    }
    else if (x >= 'a' && x <= 'f')
    {
        return 10 + (x - 'a');
    }
    else
    {
        throw "character out of range";
    }
}

// Compile time string-to-SHA256Digest
template <size_t N>
constexpr SHA256Digest ToDigest(const char (&s)[N])
{
    static_assert(N == 65); // 64 + null terminator

    SHA256Digest hash;
    for (size_t i = 0; i < N / 2; ++i)
    {
        hash.bytes[i] = (HexValue(s[2 * i + 0]) << 4) | HexValue(s[2 * i + 1]);
    }

    return hash;
}

struct EMU_KnownHash
{
    SHA256Digest       hash{};
    Romset             romset;
    EMU_RomDestination destination = EMU_RomDestination::NONE;
};

// clang-format off
static constexpr EMU_KnownHash EMU_HASHES[] = {
    {ToDigest("8a1eb33c7599b746c0c50283e4349a1bb1773b5c0ec0e9661219bf6c067d2042"), Romset::MK2, EMU_RomDestination::ROM1},
    {ToDigest("a4c9fd821059054c7e7681d61f49ce6f42ed2fe407a7ec1ba0dfdc9722582ce0"), Romset::MK2, EMU_RomDestination::ROM2},
    {ToDigest("b0b5f865a403f7308b4be8d0ed3ba2ed1c22db881b8a8326769dea222f6431d8"), Romset::MK2, EMU_RomDestination::SMROM},
    {ToDigest("c6429e21b9b3a02fbd68ef0b2053668433bee0bccd537a71841bc70b8874243b"), Romset::MK2, EMU_RomDestination::WAVEROM1},
    {ToDigest("5b753f6cef4cfc7fcafe1430fecbb94a739b874e55356246a46abe24097ee491"), Romset::MK2, EMU_RomDestination::WAVEROM2},

    // TODO: missing hashes for this romset
    {ToDigest("0000000000000000000000000000000000000000000000000000000000000000"), Romset::ST, EMU_RomDestination::ROM1},
    {ToDigest("0000000000000000000000000000000000000000000000000000000000000000"), Romset::ST, EMU_RomDestination::ROM2},
    {ToDigest("0000000000000000000000000000000000000000000000000000000000000000"), Romset::ST, EMU_RomDestination::SMROM},
    {ToDigest("0000000000000000000000000000000000000000000000000000000000000000"), Romset::ST, EMU_RomDestination::WAVEROM1},
    {ToDigest("0000000000000000000000000000000000000000000000000000000000000000"), Romset::ST, EMU_RomDestination::WAVEROM2},

    // TODO: not sure whether this is 1.00/1.21/2.0
    {ToDigest("7e1bacd1d7c62ed66e465ba05597dcd60dfc13fc23de0287fdbce6cf906c6544"), Romset::MK1, EMU_RomDestination::ROM1},
    {ToDigest("effc6132d68f7e300aaef915ccdd08aba93606c22d23e580daf9ea6617913af1"), Romset::MK1, EMU_RomDestination::ROM2},
    {ToDigest("5655509a531804f97ea2d7ef05b8fec20ebf46216b389a84c44169257a4d2007"), Romset::MK1, EMU_RomDestination::WAVEROM1},
    {ToDigest("c655b159792d999b90df9e4fa782cf56411ba1eaa0bb3ac2bdaf09e1391006b1"), Romset::MK1, EMU_RomDestination::WAVEROM2},
    {ToDigest("334b2d16be3c2362210fdbec1c866ad58badeb0f84fd9bf5d0ac599baf077cc2"), Romset::MK1, EMU_RomDestination::WAVEROM3},

    // TODO: missing hashes for this romset; multiple versions
    {ToDigest("0000000000000000000000000000000000000000000000000000000000000000"), Romset::CM300, EMU_RomDestination::ROM1},
    {ToDigest("0000000000000000000000000000000000000000000000000000000000000000"), Romset::CM300, EMU_RomDestination::ROM2},
    {ToDigest("0000000000000000000000000000000000000000000000000000000000000000"), Romset::CM300, EMU_RomDestination::WAVEROM1},
    {ToDigest("0000000000000000000000000000000000000000000000000000000000000000"), Romset::CM300, EMU_RomDestination::WAVEROM2},
    {ToDigest("0000000000000000000000000000000000000000000000000000000000000000"), Romset::CM300, EMU_RomDestination::WAVEROM3},

    // TODO: missing jv880 optional roms; optional roms not yet supported
    {ToDigest("aabfcf883b29060198566440205f2fae1ce689043ea0fc7074842aaa4fd4823e"), Romset::JV880, EMU_RomDestination::ROM1},
    {ToDigest("ed437f1bc75cc558f174707bcfeb45d5e03483efd9bfd0a382ca57c0edb2a40c"), Romset::JV880, EMU_RomDestination::ROM2},
    {ToDigest("aa3101a76d57992246efeda282a2cb0c0f8fdb441c2eed2aa0b0fad4d81f3ad4"), Romset::JV880, EMU_RomDestination::WAVEROM1},
    {ToDigest("a7b50bb47734ee9117fa16df1f257990a9a1a0b5ed420337ae4310eb80df75c8"), Romset::JV880, EMU_RomDestination::WAVEROM2},

    // TODO: missing hashes for this romset
    {ToDigest("0000000000000000000000000000000000000000000000000000000000000000"), Romset::SCB55, EMU_RomDestination::ROM1},
    {ToDigest("0000000000000000000000000000000000000000000000000000000000000000"), Romset::SCB55, EMU_RomDestination::ROM2},
    {ToDigest("0000000000000000000000000000000000000000000000000000000000000000"), Romset::SCB55, EMU_RomDestination::WAVEROM1},
    // NOTE: legacy loader looks for a file called wav "scb55_waverom2.bin", but during loading it is actually placed in WAVEROM3
    {ToDigest("0000000000000000000000000000000000000000000000000000000000000000"), Romset::SCB55, EMU_RomDestination::WAVEROM3},

    // TODO: missing hashes for this romset
    {ToDigest("0000000000000000000000000000000000000000000000000000000000000000"), Romset::RLP3237, EMU_RomDestination::ROM1},
    {ToDigest("0000000000000000000000000000000000000000000000000000000000000000"), Romset::RLP3237, EMU_RomDestination::ROM2},
    {ToDigest("0000000000000000000000000000000000000000000000000000000000000000"), Romset::RLP3237, EMU_RomDestination::WAVEROM1},

    // TODO: missing hashes for this romset; multiple versions(?)
    {ToDigest("24a65c97cdbaa847d6f59193523ce63c73394b4b693a6517ee79441f2fb8a3ee"), Romset::SC155, EMU_RomDestination::ROM1},
    {ToDigest("ceb7b9d3d9d264efe5dc3ba992b94f3be35eb6d0451abc574b6f6b5dc3db237b"), Romset::SC155, EMU_RomDestination::ROM2},
    {ToDigest("5655509a531804f97ea2d7ef05b8fec20ebf46216b389a84c44169257a4d2007"), Romset::SC155, EMU_RomDestination::WAVEROM1},
    {ToDigest("c655b159792d999b90df9e4fa782cf56411ba1eaa0bb3ac2bdaf09e1391006b1"), Romset::SC155, EMU_RomDestination::WAVEROM2},
    {ToDigest("334b2d16be3c2362210fdbec1c866ad58badeb0f84fd9bf5d0ac599baf077cc2"), Romset::SC155, EMU_RomDestination::WAVEROM3},

    {ToDigest("0000000000000000000000000000000000000000000000000000000000000000"), Romset::SC155MK2, EMU_RomDestination::ROM1},
    {ToDigest("0000000000000000000000000000000000000000000000000000000000000000"), Romset::SC155MK2, EMU_RomDestination::ROM2},
    {ToDigest("0000000000000000000000000000000000000000000000000000000000000000"), Romset::SC155MK2, EMU_RomDestination::WAVEROM1},
    {ToDigest("0000000000000000000000000000000000000000000000000000000000000000"), Romset::SC155MK2, EMU_RomDestination::WAVEROM2},
    {ToDigest("0000000000000000000000000000000000000000000000000000000000000000"), Romset::SC155MK2, EMU_RomDestination::SMROM},
};
// clang-format on

bool EMU_GetRomsets(const std::filesystem::path& base_path, EMU_AllRomsetMaps& all_maps)
{
    std::error_code ec;

    std::filesystem::directory_iterator dir_iter(base_path, ec);

    if (ec)
    {
        fprintf(stderr, "EMU_GetRomsets failed: %s\n", ec.message().c_str());
        return false;
    }

    std::vector<uint8_t> buffer;

    while (dir_iter != std::filesystem::directory_iterator{})
    {
        const bool is_file = dir_iter->is_regular_file(ec);
        if (ec)
        {
            fprintf(stderr,
                    "EMU_GetRomsets failed to check type of `%s`: %s\n",
                    dir_iter->path().generic_string().c_str(),
                    ec.message().c_str());
            return false;
        }

        if (!is_file)
        {
            dir_iter.increment(ec);
            if (ec)
            {
                fprintf(stderr, "EMU_GetRomsets failed to get next file: %s\n", ec.message().c_str());
                return false;
            }
            continue;
        }

        const uintmax_t file_size = dir_iter->file_size(ec);
        if (ec)
        {
            fprintf(stderr,
                    "EMU_GetRomsets failed to get file size of `%s`: %s\n",
                    dir_iter->path().generic_string().c_str(),
                    ec.message().c_str());
            return false;
        }

        // Skip files larger than 4MB
        if (file_size > (uintmax_t)(4 * 1024 * 1024))
        {
            dir_iter.increment(ec);
            if (ec)
            {
                fprintf(stderr, "EMU_GetRomsets failed to get next file: %s\n", ec.message().c_str());
                return false;
            }
            continue;
        }

        EMU_ReadAllBytes(dir_iter->path(), buffer);

        SHA256Context ctx;
        uint8_t       digest_bytes[SHA256HashSize]{};
        SHA256Reset(&ctx);
        SHA256Input(&ctx, buffer.data(), buffer.size());
        SHA256Result(&ctx, digest_bytes);

        for (const auto& known : EMU_HASHES)
        {
            if (known.hash == SHA256Digest(digest_bytes))
            {
                all_maps.maps[(size_t)known.romset].rom_paths[(size_t)known.destination] = dir_iter->path();
                all_maps.maps[(size_t)known.romset].rom_data[(size_t)known.destination]  = std::move(buffer);

                buffer = {};
            }
        }

        dir_iter.increment(ec);
        if (ec)
        {
            fprintf(stderr, "EMU_GetRomsets failed to get next file: %s\n", ec.message().c_str());
            return false;
        }
    }

    return true;
}

bool EMU_IsCompleteRomset(const EMU_AllRomsetMaps& all_maps, Romset romset)
{
    std::vector<EMU_RomDestination> missing;
    return EMU_IsCompleteRomset(all_maps, romset, missing);
}

bool EMU_IsCompleteRomset(const EMU_AllRomsetMaps& all_maps, Romset romset, std::vector<EMU_RomDestination>& missing)
{
    missing.clear();

    const auto& map = all_maps.maps[(size_t)romset];

    for (const auto& known : EMU_HASHES)
    {
        if (known.romset == romset && map.rom_paths[(size_t)known.destination].empty())
        {
            missing.push_back(known.destination);
        }
    }

    return missing.empty();
}

bool EMU_IsWaverom(EMU_RomDestination destination)
{
    switch (destination)
    {
    case EMU_RomDestination::WAVEROM1:
    case EMU_RomDestination::WAVEROM2:
    case EMU_RomDestination::WAVEROM3:
        return true;
    default:
        return false;
    }
}

const char* EMU_RomDestinationToString(EMU_RomDestination destination)
{
    switch (destination)
    {
    case EMU_RomDestination::ROM1:
        return "ROM1";
    case EMU_RomDestination::ROM2:
        return "ROM2";
    case EMU_RomDestination::SMROM:
        return "SMROM";
    case EMU_RomDestination::WAVEROM1:
        return "WAVEROM1";
    case EMU_RomDestination::WAVEROM2:
        return "WAVEROM2";
    case EMU_RomDestination::WAVEROM3:
        return "WAVEROM3";
    case EMU_RomDestination::COUNT:
        // also NONE
        break;
    }
    return "invalid destination";
}

Romset EMU_DetectRomset(const std::filesystem::path& base_path)
{
    for (size_t i = 0; i < (size_t)ROMSET_COUNT; i++)
    {
        bool good = true;
        for (size_t j = 0; j < 5; j++)
        {
            if (roms[i][j][0] == '\0')
                continue;
            if (!std::filesystem::exists(base_path / roms[i][j]))
            {
                good = false;
                break;
            }
        }
        if (good)
        {
            return (Romset)i;
        }
    }
    return Romset::MK2;
}

bool EMU_ReadStreamExact(std::ifstream& s, void* into, std::streamsize byte_count)
{
    if (s.read((char*)into, byte_count))
    {
        return s.gcount() == byte_count;
    }
    return false;
}

bool EMU_ReadStreamExact(std::ifstream& s, std::span<uint8_t> into, std::streamsize byte_count)
{
    return EMU_ReadStreamExact(s, into.data(), byte_count);
}

std::streamsize EMU_ReadStreamUpTo(std::ifstream& s, void* into, std::streamsize byte_count)
{
    s.read((char*)into, byte_count);
    return s.gcount();
}

bool Emulator::LoadRoms(Romset romset, const std::filesystem::path& base_path)
{
    MCU_SetRomset(GetMCU(), romset);

    std::vector<uint8_t> tempbuf(0x800000);

    std::ifstream s_rf[ROM_SET_N_FILES];

    std::filesystem::path rpaths[ROM_SET_N_FILES];

    bool r_ok = true;
    std::string errors_list;

    for(size_t i = 0; i < ROM_SET_N_FILES; ++i)
    {
        if (roms[(size_t)romset][i][0] == '\0')
        {
            continue;
        }
        rpaths[i] = base_path / roms[(size_t)romset][i];
        s_rf[i] = std::ifstream(rpaths[i].c_str(), std::ios::binary);
        bool optional = m_mcu->is_jv880 && i >= 4;
        r_ok &= optional || s_rf[i];
        if (!s_rf[i])
        {
            if(!errors_list.empty())
                errors_list.append(", ");

            errors_list.append(rpaths[i].generic_string());
        }
    }

    if (!r_ok)
    {
        fprintf(stderr, "FATAL ERROR: One of required data ROM files is missing: %s.\n", errors_list.c_str());
        fflush(stderr);
        return false;
    }

    if (!EMU_ReadStreamExact(s_rf[0], m_mcu->rom1, ROM1_SIZE))
    {
        fprintf(stderr, "FATAL ERROR: Failed to read the mcu ROM1.\n");
        fflush(stderr);
        return false;
    }

    std::streamsize rom2_read = EMU_ReadStreamUpTo(s_rf[1], m_mcu->rom2, ROM2_SIZE);

    if (rom2_read == ROM2_SIZE || rom2_read == ROM2_SIZE / 2)
    {
        m_mcu->rom2_mask = rom2_read - 1;
    }
    else
    {
        fprintf(stderr, "FATAL ERROR: Failed to read the mcu ROM2.\n");
        fflush(stderr);
        return false;
    }

    if (m_mcu->is_mk1)
    {
        if (!EMU_ReadStreamExact(s_rf[2], tempbuf, 0x100000))
        {
            fprintf(stderr, "FATAL ERROR: Failed to read the WaveRom1.\n");
            fflush(stderr);
            return false;
        }

        unscramble(tempbuf.data(), m_pcm->waverom1, 0x100000);

        if (!EMU_ReadStreamExact(s_rf[3], tempbuf, 0x100000))
        {
            fprintf(stderr, "FATAL ERROR: Failed to read the WaveRom2.\n");
            fflush(stderr);
            return false;
        }

        unscramble(tempbuf.data(), m_pcm->waverom2, 0x100000);

        if (!EMU_ReadStreamExact(s_rf[4], tempbuf, 0x100000))
        {
            fprintf(stderr, "FATAL ERROR: Failed to read the WaveRom3.\n");
            fflush(stderr);
            return false;
        }

        unscramble(tempbuf.data(), m_pcm->waverom3, 0x100000);
    }
    else if (m_mcu->is_jv880)
    {
        if (!EMU_ReadStreamExact(s_rf[2], tempbuf, 0x200000))
        {
            fprintf(stderr, "FATAL ERROR: Failed to read the WaveRom1.\n");
            fflush(stderr);
            return false;
        }

        unscramble(tempbuf.data(), m_pcm->waverom1, 0x200000);

        if (!EMU_ReadStreamExact(s_rf[3], tempbuf, 0x200000))
        {
            fprintf(stderr, "FATAL ERROR: Failed to read the WaveRom2.\n");
            fflush(stderr);
            return false;
        }

        unscramble(tempbuf.data(), m_pcm->waverom2, 0x200000);

        if (s_rf[4] && EMU_ReadStreamExact(s_rf[4], tempbuf, 0x800000))
            unscramble(tempbuf.data(), m_pcm->waverom_exp, 0x800000);
        else
            fprintf(stderr, "WaveRom EXP not found, skipping it.\n");

        if (s_rf[5] && EMU_ReadStreamExact(s_rf[5], tempbuf, 0x200000))
            unscramble(tempbuf.data(), m_pcm->waverom_card, 0x200000);
        else
            fprintf(stderr, "WaveRom PCM not found, skipping it.\n");
    }
    else
    {
        if (!EMU_ReadStreamExact(s_rf[2], tempbuf, 0x200000))
        {
            fprintf(stderr, "FATAL ERROR: Failed to read the WaveRom1.\n");
            fflush(stderr);
            return false;
        }

        unscramble(tempbuf.data(), m_pcm->waverom1, 0x200000);

        if (s_rf[3])
        {
            if (!EMU_ReadStreamExact(s_rf[3], tempbuf, 0x100000))
            {
                fprintf(stderr, "FATAL ERROR: Failed to read the WaveRom2.\n");
                fflush(stderr);
                return false;
            }

            unscramble(tempbuf.data(), m_mcu->is_scb55 ? m_pcm->waverom3 : m_pcm->waverom2, 0x100000);
        }

        if (s_rf[4] && !EMU_ReadStreamExact(s_rf[4], m_sm->rom, ROMSM_SIZE))
        {
            fprintf(stderr, "FATAL ERROR: Failed to read the sub mcu ROM.\n");
            fflush(stderr);
            return false;
        }
    }

    if (m_mcu->is_jv880)
    {
        LoadNVRAM();
    }

    MCU_PatchROM(*m_mcu);

    return true;
}

std::span<uint8_t> Emulator::MapBuffer(EMU_RomDestination romdest)
{
    switch (romdest)
    {
    case EMU_RomDestination::ROM1:
        return GetMCU().rom1;
    case EMU_RomDestination::ROM2:
        return GetMCU().rom2;
    case EMU_RomDestination::WAVEROM1:
        return GetPCM().waverom1;
    case EMU_RomDestination::WAVEROM2:
        return GetPCM().waverom2;
    case EMU_RomDestination::WAVEROM3:
        return GetPCM().waverom3;
    case EMU_RomDestination::SMROM:
        return m_sm->rom;
    case EMU_RomDestination::COUNT:
        // also none
        break;
    }
    fprintf(stderr, "FATAL: MapBuffer called with invalid romdest %d\n", (int)romdest);
    std::abort();
}

bool Emulator::LoadRomsAuto(Romset romset, const EMU_AllRomsetMaps& romset_maps)
{
    MCU_SetRomset(GetMCU(), romset);

    const EMU_RomFilenameMap& map = romset_maps.maps[(size_t)romset];

    for (int i = 0; i < (int)EMU_RomDestination::COUNT; ++i)
    {
        const EMU_RomDestination romdest = (EMU_RomDestination)i;

        if (map.rom_paths[i].empty())
        {
            continue;
        }

        fprintf(stderr,
                "Load %s %s %s\n",
                EMU_RomsetName(romset),
                EMU_RomDestinationToString(romdest),
                map.rom_paths[i].generic_string().c_str());

        auto buffer = MapBuffer(romdest);

        if (buffer.size() < map.rom_data[i].size())
        {
            fprintf(stderr,
                    "FATAL: rom for %s %s is too large: %s\n",
                    EMU_RomsetName(romset),
                    EMU_RomDestinationToString(romdest),
                    map.rom_paths[i].generic_string().c_str());
            return false;
        }

        if (EMU_IsWaverom(romdest))
        {
            unscramble(map.rom_data[i].data(), buffer.data(), (int)map.rom_data[i].size());
        }
        else
        {
            if (romdest == EMU_RomDestination::ROM2)
            {
                GetMCU().rom2_mask = (int)map.rom_data[i].size() - 1;
            }

            std::copy(map.rom_data[i].begin(), map.rom_data[i].end(), buffer.begin());
        }
    }

    MCU_PatchROM(*m_mcu);

    return true;
}

const char* EMU_RomsetName(Romset romset)
{
    return rs_name[(size_t)romset];
}

bool EMU_ParseRomsetName(std::string_view name, Romset& romset)
{
    for (size_t i = 0; i < ROMSET_COUNT; ++i)
    {
        if (rs_name_simple[i] == name)
        {
            romset = (Romset)i;
            return true;
        }
    }
    return false;
}

std::span<const char*> EMU_GetParsableRomsetNames()
{
    return rs_name_simple;
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
