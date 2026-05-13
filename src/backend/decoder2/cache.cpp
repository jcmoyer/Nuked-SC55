#include "cache.h"

namespace decoder2
{

InstructionCache::InstructionCache()
{
    m_cache = std::make_unique<ArrayType>();
    m_cache->fill({});
}

size_t InstructionCache::CountCached() const
{
    size_t count = 0;
    for (const auto& i : *m_cache)
    {
        if (i.handler)
        {
            ++count;
        }
    }
    return count;
}

} // namespace decoder2
