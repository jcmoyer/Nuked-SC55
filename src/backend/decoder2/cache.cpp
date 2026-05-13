#include "cache.h"

namespace decoder2
{

I_InstructionCache::I_InstructionCache()
{
    m_cache = std::make_unique<ArrayType>();
    m_cache->fill({});
}

size_t I_InstructionCache::CountCached() const
{
    size_t count = 0;
    for (const auto& i : *m_cache)
    {
        if (i.F)
        {
            ++count;
        }
    }
    return count;
}

} // namespace decoder2
