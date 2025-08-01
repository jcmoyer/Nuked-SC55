#include "rom_io.h"
#include "cast.h"
#include <fstream>

extern "C"
{
#include "sha/sha.h"
}

using SHA256Digest = std::array<uint8_t, SHA256HashSize>;

const char* legacy_rom_names[(size_t)ROMSET_COUNT][ROMLOCATION_COUNT] = {
    // MK2
    {
        // ROM1
        "rom1.bin",
        // ROM2
        "rom2.bin",
        // SMROM
        "rom_sm.bin",
        // WAVEROM1
        "waverom1.bin",
        // WAVEROM2
        "waverom2.bin",
        // WAVEROM3
        "",
        // WAVEROM_CARD
        "",
        // WAVEROM_EXP
        "",
    },
    // ST
    {
        // ROM1
        "rom1.bin",
        // ROM2
        "rom2_st.bin",
        // SMROM
        "rom_sm.bin",
        // WAVEROM1
        "waverom1.bin",
        // WAVEROM2
        "waverom2.bin",
        // WAVEROM3
        "",
        // WAVEROM_CARD
        "",
        // WAVEROM_EXP
        "",
    },
    // MK1
    {
        // ROM1
        "sc55_rom1.bin",
        // ROM2
        "sc55_rom2.bin",
        // SMROM
        "",
        // WAVEROM1
        "sc55_waverom1.bin",
        // WAVEROM2
        "sc55_waverom2.bin",
        // WAVEROM3
        "sc55_waverom3.bin",
        // WAVEROM_CARD
        "",
        // WAVEROM_EXP
        "",
    },
    // CM300
    {
        // ROM1
        "cm300_rom1.bin",
        // ROM2
        "cm300_rom2.bin",
        // SMROM
        "",
        // WAVEROM1
        "cm300_waverom1.bin",
        // WAVEROM2
        "cm300_waverom2.bin",
        // WAVEROM3
        "cm300_waverom3.bin",
        // WAVEROM_CARD
        "",
        // WAVEROM_EXP
        "",
    },
    // JV880
    {
        // ROM1
        "jv880_rom1.bin",
        // ROM2
        "jv880_rom2.bin",
        // SMROM
        "",
        // WAVEROM1
        "jv880_waverom1.bin",
        // WAVEROM2
        "jv880_waverom2.bin",
        // WAVEROM3
        "",
        // WAVEROM_CARD
        "jv880_waverom_pcmcard.bin",
        // WAVEROM_EXP
        "jv880_waverom_expansion.bin",
    },
    // SCB55
    {
        // ROM1
        "scb55_rom1.bin",
        // ROM2
        "scb55_rom2.bin",
        // SMROM
        "",
        // WAVEROM1
        "scb55_waverom1.bin",
        // WAVEROM2
        "",
        // WAVEROM3 - this file being named waverom2 is intentional
        "scb55_waverom2.bin",
        // WAVEROM_CARD
        "",
        // WAVEROM_EXP
        "",
    },
    // RLP3237
    {
        // ROM1
        "rlp3237_rom1.bin",
        // ROM2
        "rlp3237_rom2.bin",
        // SMROM
        "",
        // WAVEROM1
        "rlp3237_waverom1.bin",
        // WAVEROM2
        "",
        // WAVEROM3
        "",
        // WAVEROM_CARD
        "",
        // WAVEROM_EXP
        "",
    },
    // SC155
    {
        // ROM1
        "sc155_rom1.bin",
        // ROM2
        "sc155_rom2.bin",
        // SMROM
        "",
        // WAVEROM1
        "sc155_waverom1.bin",
        // WAVEROM2
        "sc155_waverom2.bin",
        // WAVEROM3
        "sc155_waverom3.bin",
        // WAVEROM_CARD
        "",
        // WAVEROM_EXP
        "",
    },
    // SC155MK2
    {
        // ROM1
        "rom1.bin",
        // ROM2
        "rom2.bin",
        // SMROM
        "rom_sm.bin",
        // WAVEROM1
        "waverom1.bin",
        // WAVEROM2
        "waverom2.bin",
        // WAVEROM3
        "",
        // WAVEROM_CARD
        "",
        // WAVEROM_EXP
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

bool ReadAllBytes(const std::filesystem::path& filename, std::vector<uint8_t>& buffer)
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

constexpr uint8_t HexValue(char x)
{
    if (x >= '0' && x <= '9')
    {
        return (uint8_t)(x - '0');
    }
    else if (x >= 'a' && x <= 'f')
    {
        return 10 + (uint8_t)(x - 'a');
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
        hash[i] = (uint8_t)((HexValue(s[2 * i + 0]) << 4) | HexValue(s[2 * i + 1]));
    }

    return hash;
}

struct KnownHash
{
    SHA256Digest hash;
    Romset       romset;
    RomLocation  location;
};

// clang-format off
static constexpr KnownHash ROM_HASHES[] = {
    ///////////////////////////////////////////////////////////////////////////
    // SC-55mk2/SC-155mk2 (v1.01)
    ///////////////////////////////////////////////////////////////////////////

    // R15199858 (H8/532 mcu)
    {ToDigest("8a1eb33c7599b746c0c50283e4349a1bb1773b5c0ec0e9661219bf6c067d2042"), Romset::MK2, RomLocation::ROM1},
    // R00233567 (H8/532 extra code)
    {ToDigest("a4c9fd821059054c7e7681d61f49ce6f42ed2fe407a7ec1ba0dfdc9722582ce0"), Romset::MK2, RomLocation::ROM2},
    // R15199880 (M37450M2 mcu)
    {ToDigest("b0b5f865a403f7308b4be8d0ed3ba2ed1c22db881b8a8326769dea222f6431d8"), Romset::MK2, RomLocation::SMROM},
    // R15209359 (WAVE 16M)
    {ToDigest("c6429e21b9b3a02fbd68ef0b2053668433bee0bccd537a71841bc70b8874243b"), Romset::MK2, RomLocation::WAVEROM1},
    // R15279813 (WAVE 8M)
    {ToDigest("5b753f6cef4cfc7fcafe1430fecbb94a739b874e55356246a46abe24097ee491"), Romset::MK2, RomLocation::WAVEROM2},

    // R15199858 (H8/532 mcu)
    {ToDigest("8a1eb33c7599b746c0c50283e4349a1bb1773b5c0ec0e9661219bf6c067d2042"), Romset::SC155MK2, RomLocation::ROM1},
    // R00233567 (H8/532 extra code)
    {ToDigest("a4c9fd821059054c7e7681d61f49ce6f42ed2fe407a7ec1ba0dfdc9722582ce0"), Romset::SC155MK2, RomLocation::ROM2},
    // R15199880 (M37450M2 mcu)
    {ToDigest("b0b5f865a403f7308b4be8d0ed3ba2ed1c22db881b8a8326769dea222f6431d8"), Romset::SC155MK2, RomLocation::SMROM},
    // R15209359 (WAVE 16M)
    {ToDigest("c6429e21b9b3a02fbd68ef0b2053668433bee0bccd537a71841bc70b8874243b"), Romset::SC155MK2, RomLocation::WAVEROM1},
    // R15279813 (WAVE 8M)
    {ToDigest("5b753f6cef4cfc7fcafe1430fecbb94a739b874e55356246a46abe24097ee491"), Romset::SC155MK2, RomLocation::WAVEROM2},

    ///////////////////////////////////////////////////////////////////////////
    // SC-55st (v1.01)
    ///////////////////////////////////////////////////////////////////////////

    // R15199858 (H8/532 mcu)
    {ToDigest("8a1eb33c7599b746c0c50283e4349a1bb1773b5c0ec0e9661219bf6c067d2042"), Romset::ST, RomLocation::ROM1},
    // R00561413 (H8/532 extra code)
    {ToDigest("0000000000000000000000000000000000000000000000000000000000000000"), Romset::ST, RomLocation::ROM2},
    // R15199880 (M37450M2 mcu)
    {ToDigest("b0b5f865a403f7308b4be8d0ed3ba2ed1c22db881b8a8326769dea222f6431d8"), Romset::ST, RomLocation::SMROM},
    // R15209359 (WAVE 16M)
    {ToDigest("c6429e21b9b3a02fbd68ef0b2053668433bee0bccd537a71841bc70b8874243b"), Romset::ST, RomLocation::WAVEROM1},
    // R15279813 (WAVE 8M)
    {ToDigest("5b753f6cef4cfc7fcafe1430fecbb94a739b874e55356246a46abe24097ee491"), Romset::ST, RomLocation::WAVEROM2},

    ///////////////////////////////////////////////////////////////////////////
    // SC-55 (v1.21)
    ///////////////////////////////////////////////////////////////////////////

    // R15199748 (H8/532 mcu)
    {ToDigest("7e1bacd1d7c62ed66e465ba05597dcd60dfc13fc23de0287fdbce6cf906c6544"), Romset::MK1, RomLocation::ROM1},
    // R1544925800 (H8/532 extra code)
    {ToDigest("effc6132d68f7e300aaef915ccdd08aba93606c22d23e580daf9ea6617913af1"), Romset::MK1, RomLocation::ROM2},
    // R15209276 (WAVE A)
    {ToDigest("5655509a531804f97ea2d7ef05b8fec20ebf46216b389a84c44169257a4d2007"), Romset::MK1, RomLocation::WAVEROM1},
    // R15209277 (WAVE B)
    {ToDigest("c655b159792d999b90df9e4fa782cf56411ba1eaa0bb3ac2bdaf09e1391006b1"), Romset::MK1, RomLocation::WAVEROM2},
    // R15209281 (WAVE C)
    {ToDigest("334b2d16be3c2362210fdbec1c866ad58badeb0f84fd9bf5d0ac599baf077cc2"), Romset::MK1, RomLocation::WAVEROM3},

    ///////////////////////////////////////////////////////////////////////////
    // CM-300/SCC-1 (v1.10)
    ///////////////////////////////////////////////////////////////////////////

    // R15199774 (H8/532 mcu)
    {ToDigest("0000000000000000000000000000000000000000000000000000000000000000"), Romset::CM300, RomLocation::ROM1},
    // R15279809 (H8/532 extra code)
    {ToDigest("0283d32e6993a0265710c4206463deb937b0c3a4819b69f471a0eca5865719f9"), Romset::CM300, RomLocation::ROM2},
    // R15279806 (WAVE A)
    {ToDigest("40c093cbfb4441a5c884e623f882a80b96b2527f9fd431e074398d206c0f073d"), Romset::CM300, RomLocation::WAVEROM1},
    // R15279807 (WAVE B)
    {ToDigest("9bbbcac747bd6f7a2693f4ef10633db8ab626f17d3d9c47c83c3839d4dd2f613"), Romset::CM300, RomLocation::WAVEROM2},
    // R15279808 (WAVE C)
    {ToDigest("5b753f6cef4cfc7fcafe1430fecbb94a739b874e55356246a46abe24097ee491"), Romset::CM300, RomLocation::WAVEROM3},

    ///////////////////////////////////////////////////////////////////////////
    // CM-300/SCC-1 (v1.20)
    ///////////////////////////////////////////////////////////////////////////

    // R15199774 (H8/532 mcu)
    {ToDigest("0000000000000000000000000000000000000000000000000000000000000000"), Romset::CM300, RomLocation::ROM1},
    // R15279812 (H8/532 extra code)
    {ToDigest("fef1acb1969525d66238be5e7811108919b07a4df5fbab656ad084966373483f"), Romset::CM300, RomLocation::ROM2},
    // R15279806 (WAVE A)
    {ToDigest("40c093cbfb4441a5c884e623f882a80b96b2527f9fd431e074398d206c0f073d"), Romset::CM300, RomLocation::WAVEROM1},
    // R15279807 (WAVE B)
    {ToDigest("9bbbcac747bd6f7a2693f4ef10633db8ab626f17d3d9c47c83c3839d4dd2f613"), Romset::CM300, RomLocation::WAVEROM2},
    // R15279808 (WAVE C)
    {ToDigest("5b753f6cef4cfc7fcafe1430fecbb94a739b874e55356246a46abe24097ee491"), Romset::CM300, RomLocation::WAVEROM3},

    ///////////////////////////////////////////////////////////////////////////
    // SCC-1A
    ///////////////////////////////////////////////////////////////////////////

    // R00128523 (H8/532 mcu)
    {ToDigest("9ec66abb5231b6c6f46f48b33d5412703041037d69a6803626ac402f25552af2"), Romset::CM300, RomLocation::ROM1},
    // R00128567 (H8/532 extra code)
    {ToDigest("f89442734fdebacae87c7707c01b2d7fdbf5940abae738987aee912d34b5882e"), Romset::CM300, RomLocation::ROM2},
    // R15279806 (WAVE A)
    {ToDigest("40c093cbfb4441a5c884e623f882a80b96b2527f9fd431e074398d206c0f073d"), Romset::CM300, RomLocation::WAVEROM1},
    // R15279807 (WAVE B)
    {ToDigest("9bbbcac747bd6f7a2693f4ef10633db8ab626f17d3d9c47c83c3839d4dd2f613"), Romset::CM300, RomLocation::WAVEROM2},
    // R15279808 (WAVE C)
    {ToDigest("5b753f6cef4cfc7fcafe1430fecbb94a739b874e55356246a46abe24097ee491"), Romset::CM300, RomLocation::WAVEROM3},

    ///////////////////////////////////////////////////////////////////////////
    // JV-880 (v1.0.0)
    ///////////////////////////////////////////////////////////////////////////

    // R15199810 (H8/532 mcu)
    {ToDigest("aabfcf883b29060198566440205f2fae1ce689043ea0fc7074842aaa4fd4823e"), Romset::JV880, RomLocation::ROM1},
    // R15209386 (H8/532 extra code)
    {ToDigest("ed437f1bc75cc558f174707bcfeb45d5e03483efd9bfd0a382ca57c0edb2a40c"), Romset::JV880, RomLocation::ROM2},
    // R15209312 (WAVE A)
    {ToDigest("aa3101a76d57992246efeda282a2cb0c0f8fdb441c2eed2aa0b0fad4d81f3ad4"), Romset::JV880, RomLocation::WAVEROM1},
    // R15209313 (WAVE B)
    {ToDigest("a7b50bb47734ee9117fa16df1f257990a9a1a0b5ed420337ae4310eb80df75c8"), Romset::JV880, RomLocation::WAVEROM2},
    // R00000000 (placeholder)
    {ToDigest("0000000000000000000000000000000000000000000000000000000000000000"), Romset::JV880, RomLocation::WAVEROM_CARD},
    // R00000000 (placeholder)
    {ToDigest("0000000000000000000000000000000000000000000000000000000000000000"), Romset::JV880, RomLocation::WAVEROM_EXP},

    // TODO: missing jv880 optional roms

    ///////////////////////////////////////////////////////////////////////////
    // SCB-55/RLP-3194
    ///////////////////////////////////////////////////////////////////////////

    // TODO: missing hashes for this romset

    // R15199827 (H8/532 mcu)
    {ToDigest("0000000000000000000000000000000000000000000000000000000000000000"), Romset::SCB55, RomLocation::ROM1},
    // R15279828 (H8/532 extra code)
    {ToDigest("0000000000000000000000000000000000000000000000000000000000000000"), Romset::SCB55, RomLocation::ROM2},
    // R15209359 (WAVE 16M)
    {ToDigest("c6429e21b9b3a02fbd68ef0b2053668433bee0bccd537a71841bc70b8874243b"), Romset::SCB55, RomLocation::WAVEROM1},
    // R15279813 (WAVE 8M)
    {ToDigest("5b753f6cef4cfc7fcafe1430fecbb94a739b874e55356246a46abe24097ee491"), Romset::SCB55, RomLocation::WAVEROM3},
    // ^NOTE: legacy loader looks for a file called wav "scb55_waverom2.bin", but during loading it is actually placed in WAVEROM3

    ///////////////////////////////////////////////////////////////////////////
    // RLP-3237
    ///////////////////////////////////////////////////////////////////////////

    // TODO: missing hashes for this romset

    // R15199827 (H8/532 mcu)
    {ToDigest("0000000000000000000000000000000000000000000000000000000000000000"), Romset::RLP3237, RomLocation::ROM1},
    // R15209486 (H8/532 extra code)
    {ToDigest("0000000000000000000000000000000000000000000000000000000000000000"), Romset::RLP3237, RomLocation::ROM2},
    // R15279824 (WAVE 16M)
    {ToDigest("0000000000000000000000000000000000000000000000000000000000000000"), Romset::RLP3237, RomLocation::WAVEROM1},

    ///////////////////////////////////////////////////////////////////////////
    // SC-155 (rev 1)
    ///////////////////////////////////////////////////////////////////////////

    // R15199799 (H8/532 mcu)
    {ToDigest("24a65c97cdbaa847d6f59193523ce63c73394b4b693a6517ee79441f2fb8a3ee"), Romset::SC155, RomLocation::ROM1},
    // R15209361 (H8/532 extra code)
    {ToDigest("ceb7b9d3d9d264efe5dc3ba992b94f3be35eb6d0451abc574b6f6b5dc3db237b"), Romset::SC155, RomLocation::ROM2},
    // R15209276 (WAVE A)
    {ToDigest("5655509a531804f97ea2d7ef05b8fec20ebf46216b389a84c44169257a4d2007"), Romset::SC155, RomLocation::WAVEROM1},
    // R15209277 (WAVE B)
    {ToDigest("c655b159792d999b90df9e4fa782cf56411ba1eaa0bb3ac2bdaf09e1391006b1"), Romset::SC155, RomLocation::WAVEROM2},
    // R15209281 (WAVE C)
    {ToDigest("334b2d16be3c2362210fdbec1c866ad58badeb0f84fd9bf5d0ac599baf077cc2"), Romset::SC155, RomLocation::WAVEROM3},

    ///////////////////////////////////////////////////////////////////////////
    // SC-155 (rev 2)
    ///////////////////////////////////////////////////////////////////////////

    // TODO: missing hashes for this romset

    // R15199799 (H8/532 mcu)
    {ToDigest("24a65c97cdbaa847d6f59193523ce63c73394b4b693a6517ee79441f2fb8a3ee"), Romset::SC155, RomLocation::ROM1},
    // R15209400 (H8/532 extra code)
    {ToDigest("0000000000000000000000000000000000000000000000000000000000000000"), Romset::SC155, RomLocation::ROM2},
    // R15209276 (WAVE A)
    {ToDigest("5655509a531804f97ea2d7ef05b8fec20ebf46216b389a84c44169257a4d2007"), Romset::SC155, RomLocation::WAVEROM1},
    // R15209277 (WAVE B)
    {ToDigest("c655b159792d999b90df9e4fa782cf56411ba1eaa0bb3ac2bdaf09e1391006b1"), Romset::SC155, RomLocation::WAVEROM2},
    // R15209281 (WAVE C)
    {ToDigest("334b2d16be3c2362210fdbec1c866ad58badeb0f84fd9bf5d0ac599baf077cc2"), Romset::SC155, RomLocation::WAVEROM3},

    ///////////////////////////////////////////////////////////////////////////
    // Extra/modified roms
    ///////////////////////////////////////////////////////////////////////////

    // CTF patched roms from https://github.com/shingo45endo/sc55mk2-ctf-patcher

    // Tone: Strict SC-55 | Drum: SC-55 v1.21 or earlier
    {ToDigest("64f8c9daf1021cf86ea4ddf03a29b81b5ea0c18e74f462833023436388bb9dc4"), Romset::MK2, RomLocation::ROM2},
    // Tone: Strict SC-55 | Drum: SC-55 v2.00
    {ToDigest("14d14778caf46ffa9e3d608aa8e9c1a60c32bd4a536c26af3b2e1d81784c60f9"), Romset::MK2, RomLocation::ROM2},
    // Tone: SC-55 | Drum: SC-55 v1.21 or earlier
    {ToDigest("10b3f09485a74bb014f1a940d5c67f380c7979b62891d540d788154c83f17430"), Romset::MK2, RomLocation::ROM2},
    // Tone: SC-55 | Drum: SC-55 v2.00
    {ToDigest("a2c720be1ab9115930d27f821a413c0366b7bf0c4ddfe0dadc5086136a1a4345"), Romset::MK2, RomLocation::ROM2},
    // Tone: SC-55mkII | Drum: SC-55 v1.21 or earlier
    {ToDigest("16cec615da10089beffe6de5129ba8ba33fa1bf017a5e6b78ad1d6d15cf4708e"), Romset::MK2, RomLocation::ROM2},
    // Tone: SC-55mkII | Drum: SC-55 v2.00
    {ToDigest("c22bf7d34a3406530924d750b007bbdb470f3216c65086edb6e53023383ee907"), Romset::MK2, RomLocation::ROM2},
};
// clang-format on


bool DetectRomsetsByHash(const std::filesystem::path& base_path,
                             AllRomsetInfo&           all_info,
                             RomLocationSet*          desired)
{
    std::error_code ec;

    std::filesystem::directory_iterator dir_iter(base_path, ec);

    if (ec)
    {
        fprintf(stderr, "Failed to walk rom directory: %s\n", ec.message().c_str());
        return false;
    }

    std::vector<uint8_t> buffer;

    while (dir_iter != std::filesystem::directory_iterator{})
    {
        const bool is_file = dir_iter->is_regular_file(ec);
        if (ec)
        {
            fprintf(stderr,
                    "Failed to check file type of `%s`: %s\n",
                    dir_iter->path().generic_string().c_str(),
                    ec.message().c_str());
            return false;
        }

        if (!is_file)
        {
            dir_iter.increment(ec);
            if (ec)
            {
                fprintf(stderr, "Failed to get next file: %s\n", ec.message().c_str());
                return false;
            }
            continue;
        }

        const uintmax_t file_size = dir_iter->file_size(ec);
        if (ec)
        {
            fprintf(stderr,
                    "Failed to get file size of `%s`: %s\n",
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
                fprintf(stderr, "Failed to get next file: %s\n", ec.message().c_str());
                return false;
            }
            continue;
        }

        ReadAllBytes(dir_iter->path(), buffer);

        SHA256Context ctx;
        SHA256Digest  digest_bytes;

        SHA256Reset(&ctx);
        SHA256Input(&ctx, buffer.data(), (unsigned int)buffer.size());
        SHA256Result(&ctx, digest_bytes.data());

        for (const auto& known : ROM_HASHES)
        {
            if (known.hash == digest_bytes && !all_info.romsets[(size_t)known.romset].HasRom(known.location))
            {
                all_info.romsets[(size_t)known.romset].rom_paths[(size_t)known.location] = dir_iter->path();

                if (desired && (*desired)[(size_t)known.location])
                {
                    auto& rom_data = all_info.romsets[(size_t)known.romset].rom_data[(size_t)known.location];
                    if (IsWaverom(known.location))
                    {
                        rom_data.resize(buffer.size());
                        unscramble(rom_data.data(), buffer.data(), (int)buffer.size());
                    }
                    else
                    {
                        rom_data = std::move(buffer);
                        buffer   = {};
                    }
                }
            }
        }

        dir_iter.increment(ec);
        if (ec)
        {
            fprintf(stderr, "Failed to get next file: %s\n", ec.message().c_str());
            return false;
        }
    }

    return true;
}

bool IsCompleteRomset(const AllRomsetInfo& all_info, Romset romset, RomCompletionStatusSet* status)
{
    bool is_complete = true;

    if (status)
    {
        status->fill(RomCompletionStatus::Unused);
    }

    const auto& info = all_info.romsets[(size_t)romset];

    for (const auto& known : ROM_HASHES)
    {
        if (known.romset != romset)
        {
            continue;
        }

        if (!info.HasRom(known.location) && !IsOptionalRom(romset, known.location))
        {
            is_complete = false;
            if (status)
            {
                (*status)[(size_t)known.location] = RomCompletionStatus::Missing;
            }
        }
        else if (info.HasRom(known.location))
        {
            if (status)
            {
                (*status)[(size_t)known.location] = RomCompletionStatus::Present;
            }
        }
    }

    return is_complete;
}

size_t CountPresent(const RomCompletionStatusSet& status)
{
    size_t count = 0;
    for (auto s : status)
    {
        if (s == RomCompletionStatus::Present)
        {
            ++count;
        }
    }
    return count;
}

bool PickCompleteRomset(const AllRomsetInfo& all_info, Romset& out_romset)
{
    for (size_t i = 0; i < ROMSET_COUNT; ++i)
    {
        if (IsCompleteRomset(all_info, (Romset)i))
        {
            out_romset = (Romset)i;
            return true;
        }
    }
    return false;
}

bool DetectRomsetsByFilename(const std::filesystem::path& base_path,
                                 AllRomsetInfo&           all_info,
                                 RomLocationSet*          desired)
{
    for (size_t romset = 0; romset < ROMSET_COUNT; ++romset)
    {
        if (desired && !(*desired)[romset])
        {
            continue;
        }

        for (size_t rom = 0; rom < ROMLOCATION_COUNT; ++rom)
        {
            if (legacy_rom_names[romset][rom][0] == '\0')
            {
                continue;
            }

            std::filesystem::path rom_path = base_path / legacy_rom_names[romset][rom];

            all_info.romsets[romset].rom_paths[rom] = std::move(rom_path);
        }
    }

    return true;
}

bool ReadStreamExact(std::ifstream& s, void* into, std::streamsize byte_count)
{
    if (s.read((char*)into, byte_count))
    {
        return s.gcount() == byte_count;
    }
    return false;
}

bool ReadStreamExact(std::ifstream& s, std::span<uint8_t> into, std::streamsize byte_count)
{
    return ReadStreamExact(s, into.data(), byte_count);
}

std::streamsize ReadStreamUpTo(std::ifstream& s, void* into, std::streamsize byte_count)
{
    s.read((char*)into, byte_count);
    return s.gcount();
}

void RomsetInfo::PurgeRomData()
{
    for (auto& vec : rom_data)
    {
        vec = {};
    }
}

bool RomsetInfo::HasRom(RomLocation location) const
{
    return !(rom_paths[(size_t)location].empty() && rom_data[(size_t)location].empty());
}

void AllRomsetInfo::PurgeRomData()
{
    for (auto& romset : romsets)
    {
        romset.PurgeRomData();
    }
}

bool LoadRomset(Romset romset, AllRomsetInfo& all_info, RomLoadStatusSet* loaded)
{
    bool all_loaded = true;

    // We cannot unscramble in-place.
    std::vector<uint8_t> on_demand_buffer;

    RomsetInfo& info = all_info.romsets[(size_t)romset];

    for (size_t i = 0; i < ROMLOCATION_COUNT; ++i)
    {
        const RomLocation location = (RomLocation)i;

        if (info.rom_paths[i].empty() && info.rom_data[i].empty())
        {
            if (loaded)
            {
                (*loaded)[i] = RomLoadStatus::Unused;
            }
            continue;
        }
        else if (!info.rom_paths[i].empty() && info.rom_data[i].empty())
        {
            if (!ReadAllBytes(info.rom_paths[i], on_demand_buffer))
            {
                all_loaded = false;
                if (loaded)
                {
                    (*loaded)[i] = RomLoadStatus::Failed;
                }
                continue;
            }

            if (IsWaverom(location))
            {
                info.rom_data[i].resize(on_demand_buffer.size());
                unscramble(on_demand_buffer.data(), info.rom_data[i].data(), (int)on_demand_buffer.size());
            }
            else
            {
                info.rom_data[i] = std::move(on_demand_buffer);
                on_demand_buffer = {};
            }

            if (loaded)
            {
                (*loaded)[i] = RomLoadStatus::Loaded;
            }
        }
        else if (!info.rom_data[i].empty())
        {
            if (loaded)
            {
                (*loaded)[i] = RomLoadStatus::Loaded;
            }
        }
    }

    return all_loaded;
}

const char* ToCString(RomLoadStatus status)
{
    switch (status)
    {
    case RomLoadStatus::Loaded:
        return "Loaded";
    case RomLoadStatus::Failed:
        return "Failed";
    case RomLoadStatus::Unused:
        return "Unused";
    }
    return "Unknown status";
}

const char* ToCString(RomCompletionStatus status)
{
    switch (status)
    {
    case RomCompletionStatus::Present:
        return "Present";
    case RomCompletionStatus::Missing:
        return "Missing";
    case RomCompletionStatus::Unused:
        return "Unused";
    }
    return "Unknown status";
}
