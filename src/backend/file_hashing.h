#pragma once

#include <array>
#include <concepts>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <unordered_map>
#include <vector>

#include "diagnostics.h"
#include "file_io.h"

extern "C"
{
#include "sha/sha.h"
}

using SHA256Digest = std::array<uint8_t, 32>;

template <>
struct std::hash<SHA256Digest>
{
    size_t operator()(const SHA256Digest& digest) const
    {
        size_t result = 0;
        for (size_t i = 0; i < sizeof(SHA256Digest) / sizeof(size_t); ++i)
        {
            size_t block;
            memcpy(&block, &digest[i * sizeof(size_t)], sizeof(size_t));
            result ^= block;
        }
        return result;
    }
};

namespace detail
{
consteval uint8_t HexValue(char x)
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
} // namespace detail

// Compile time string-to-SHA256Digest
template <size_t N>
consteval SHA256Digest ToDigest(const char (&s)[N])
{
    static_assert(N == 65); // 64 + null terminator

    SHA256Digest hash;
    for (size_t i = 0; i < N / 2; ++i)
    {
        hash[i] = (uint8_t)((detail::HexValue(s[2 * i + 0]) << 4) | detail::HexValue(s[2 * i + 1]));
    }

    return hash;
}

// Contains the path and contents of a hashed file.
struct HashedFile
{
    std::filesystem::path path;
    std::vector<uint8_t>  data;
};

// Contains a list of hashed files and provides constant-time lookup by hash.
class HashedFileRegistry
{
public:
    void AddFile(SHA256Digest hash, HashedFile file);
    bool Contains(SHA256Digest hash) const;

    // The returned pointer is invalidated when the registry is modified.
    // This function returns `nullptr` when `hash` is not in the registry.
    const HashedFile* GetFile(SHA256Digest hash) const;

    void Purge();

private:
    std::vector<HashedFile> m_files;
    // SHA256 to index in `files`
    std::unordered_map<SHA256Digest, size_t> m_hash_map;
};

// If `filter` returns true for a file, it will be hashed; otherwise it will be skipped.
template <std::invocable<const std::filesystem::directory_entry&> FileFilter>
bool HashDirectoryFiles(const std::filesystem::path& dir_path, HashedFileRegistry& registry, FileFilter filter)
{
    using namespace std::filesystem;

    // std::fileystem cannot guarantee exceptions won't be thrown even for the error code overloads
    try
    {
        std::vector<uint8_t> buffer;

        for (directory_iterator dir_iter(dir_path); dir_iter != directory_iterator{}; ++dir_iter)
        {
            if (!dir_iter->is_regular_file())
            {
                continue;
            }

            if (!filter(*dir_iter))
            {
                continue;
            }

            if (!FIO_ReadAllBytes(dir_iter->path(), buffer))
            {
                Diag_Printf(
                    Diag_Category::Error, "Failed to read file: %s\n", dir_iter->path().generic_string().c_str());
                return false;
            }

            SHA256Context ctx;
            SHA256Digest  digest_bytes;

            SHA256Reset(&ctx);
            SHA256Input(&ctx, buffer.data(), (unsigned int)buffer.size());
            SHA256Result(&ctx, digest_bytes.data());

            registry.AddFile(digest_bytes,
                             HashedFile{
                                 .path = dir_iter->path(),
                                 .data = std::move(buffer),
                             });
        }
    }
    catch (const std::exception& e)
    {
        Diag_Printf(Diag_Category::Error, "Failed to hash roms: %s\n", e.what());
        return false;
    }
    return true;
}
