#pragma once

#include <array>

#include "backend/diagnostics.h"
#include "backend/math_util.h"
#include "backend/ringbuffer.h"
#include "backend/rom.h"

// TODO feature guard
#include "backend/decoder2/cache.h"

constexpr uint32_t UNUSED_COMPONENT = static_cast<uint32_t>(-1);

struct EmulatorInstanceLayout
{
    uint32_t mcu_base    = UNUSED_COMPONENT;
    uint32_t submcu_base = UNUSED_COMPONENT;
    uint32_t timer_base  = UNUSED_COMPONENT;
    uint32_t lcd_base    = UNUSED_COMPONENT;
    uint32_t pcm_base    = UNUSED_COMPONENT;
};

struct EmulatorMemoryLayout
{
    size_t total_alloc_size;

    std::array<uint32_t, ROMLOCATION_COUNT> rom_base;
    std::array<uint32_t, ROMLOCATION_COUNT> rom_size;

    // TODO feature guard to avoid accidental use
    uint32_t icache_base;

    std::array<EmulatorInstanceLayout, 16> instance_base;
    size_t                                 instance_count;

    Romset romset;
};

struct EmulatorMemory
{
    // pointer passed back to free
    void* base_ptr;
    // pointer to the memory we actually use
    void* aligned_ptr;
    // layout this memory was allocated with
    EmulatorMemoryLayout layout;
};

std::span<uint8_t> MapBuffer(EmulatorMemory& memory, RomLocation location);

void* GetOffsetPtr(EmulatorMemory& mem, uint32_t offset);

// Creates a memory layout. This function does not perform any
// allocation. The result is available through `layout`. The total
// size of the allocation depends on `romset` and `instance_count`.
void CreateMemoryLayout(Romset family, size_t instance_count, EmulatorMemoryLayout& layout);

bool AllocateMemory(const EmulatorMemoryLayout& layout, EmulatorMemory& mem);

void DeallocateMemory(EmulatorMemory& mem);

bool ConstructEmulator(EmulatorMemory& mem, size_t instance_id);

void* GetMCUPtr(EmulatorMemory& mem, size_t instance_id);
void* GetSMPtr(EmulatorMemory& mem, size_t instance_id);
void* GetTimerPtr(EmulatorMemory& mem, size_t instance_id);
void* GetLCDPtr(EmulatorMemory& mem, size_t instance_id);
void* GetPCMPtr(EmulatorMemory& mem, size_t instance_id);

inline bool LoadRom(EmulatorMemory& mem, RomLocation location, std::span<const uint8_t> source)
{
    auto buffer = MapBuffer(mem, location);

    if (buffer.size() < source.size())
    {
        Diag_Printf(Diag_Category::Error,
                    "rom for %s is too large; max size is %d bytes\n",
                    ToCString(location),
                    (int)buffer.size());
        return false;
    }

    std::copy(source.begin(), source.end(), buffer.begin());

    return true;
}

#include "backend/rom_io.h"

inline bool LoadRoms(EmulatorMemory& mem, const RomsetInfo& info, RomLocationSet* loaded)
{
    if (loaded)
    {
        loaded->fill(false);
    }

    for (size_t i = 0; i < ROMLOCATION_COUNT; ++i)
    {
        const RomLocation location = (RomLocation)i;

        // rom_data should be populated at this point
        // if it isn't, then there isn't a rom for this location
        if (info.rom_data[i].empty())
        {
            continue;
        }

        if (!LoadRom(mem, location, info.rom_data[i]))
        {
            return false;
        }

        if (loaded)
        {
            (*loaded)[i] = true;
        }
    }

    return true;
}
