#pragma once

#include <array>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <unordered_map>
#include <vector>

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

using FileFilter = bool (*)(const std::filesystem::directory_entry&);

// If `filter` returns true for a file, it will be hashed; otherwise it will be skipped.
bool HashDirectoryFiles(const std::filesystem::path& dir_path, HashedFileRegistry& registry, FileFilter filter);
