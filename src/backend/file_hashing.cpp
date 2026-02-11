#include "file_hashing.h"

bool HashedFileRegistry::AddFile(SHA256_Digest hash, HashedFile file)
{
    const size_t next_index = m_files.size();
    auto [it, inserted]     = m_hash_map.emplace(std::make_pair(hash, next_index));
    if (inserted)
    {
        m_files.emplace_back(std::move(file));
        return true;
    }
    return false;
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
