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

#pragma once

#include <concepts>
#include <cstdint>
#include <filesystem>
#include <unordered_map>
#include <vector>

#include "diagnostics.h"
#include "file_io.h"
#include "sha256.h"

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
    // Returns true if the file was added, or false if there is already one
    // with the same hash. Duplicate hashes is not an error condition.
    bool AddFile(SHA256_Digest hash, HashedFile file);

    bool Contains(SHA256_Digest hash) const;

    // The returned pointer is invalidated when the registry is modified.
    // This function returns `nullptr` when `hash` is not in the registry.
    const HashedFile* GetFile(SHA256_Digest hash) const;

    void Purge();

private:
    std::vector<HashedFile> m_files;
    // SHA256 to index in `files`
    std::unordered_map<SHA256_Digest, size_t> m_hash_map;
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

            SHA256_Digest digest_bytes;

            if (!SHA256_HashBytes(buffer, digest_bytes))
            {
                return false;
            }

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
