#include "file_hashing.h"

void HashedFileRegistry::AddFile(SHA256_Digest hash, HashedFile file)
{
    const size_t next_index = m_files.size();
    m_hash_map.emplace(std::make_pair(hash, next_index));
    m_files.emplace_back(std::move(file));
}

bool HashedFileRegistry::Contains(SHA256_Digest hash) const
{
    return m_hash_map.contains(hash);
}

const HashedFile* HashedFileRegistry::GetFile(SHA256_Digest hash) const
{
    const auto it = m_hash_map.find(hash);

    if (it == m_hash_map.end())
    {
        return nullptr;
    }

    return &m_files[it->second];
}

void HashedFileRegistry::Purge()
{
    m_files.clear();
    m_hash_map.clear();
}
