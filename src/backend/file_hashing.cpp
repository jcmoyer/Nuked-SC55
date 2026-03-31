#include "file_hashing.h"

#include "diagnostics.h"
#include "file_io.h"

extern "C"
{
#include "sha/sha.h"
}

bool HashAllFiles(const std::filesystem::path& base_path, HashedFileRegistry& registry)
{
    using namespace std::filesystem;

    // std::fileystem cannot guarantee exceptions won't be thrown even for the error code overloads
    try
    {
        std::vector<uint8_t> buffer;

        for (directory_iterator dir_iter(base_path); dir_iter != directory_iterator{}; ++dir_iter)
        {
            if (!dir_iter->is_regular_file())
            {
                continue;
            }

            const uintmax_t file_size = dir_iter->file_size();

            // Skip files larger than 4MB
            if (file_size > (uintmax_t)(4 * 1024 * 1024))
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

void HashedFileRegistry::AddFile(SHA256Digest hash, HashedFile file)
{
    const size_t next_index = m_files.size();
    m_hash_map.emplace(std::make_pair(hash, next_index));
    m_files.emplace_back(std::move(file));
}

bool HashedFileRegistry::Contains(SHA256Digest hash) const
{
    return m_hash_map.contains(hash);
}

const HashedFile* HashedFileRegistry::GetFile(SHA256Digest hash) const
{
    const auto it = m_hash_map.find(hash);

    if (it == m_hash_map.end())
    {
        return nullptr;
    }

    return &m_files[it->second];
}
