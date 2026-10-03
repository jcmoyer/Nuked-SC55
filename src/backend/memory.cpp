#include "backend/memory.h"

// required for type sizes
#include "backend/lcd.h"
#include "backend/mcu.h"
#include "backend/mcu_timer.h"
#include "backend/pcm.h"
#include "backend/submcu.h"

// TODO guard on ifdef
#include "backend/decoder2/instruction.h"

// typical page alignment
const size_t DEFAULT_ALIGNMENT = 4096;

std::span<uint8_t> MapBuffer(EmulatorMemory& memory, RomLocation location)
{
    void* base = GetOffsetPtr(memory, memory.layout.rom_base[(size_t)location]);
    return std::span((uint8_t*)base, memory.layout.rom_size[(size_t)location]);
}

void CreateMemoryLayout(Romset family, size_t instance_count, EmulatorMemoryLayout& layout)
{
    (void)instance_count;

    layout.romset = family;

    layout.total_alloc_size = 0;
    layout.rom_base.fill(UNUSED_COMPONENT);
    layout.rom_size.fill(0);

#define ALLOC_SECTION(loc, size)                                                                                       \
    layout.rom_base[static_cast<size_t>(loc)]  = static_cast<uint32_t>(layout.total_alloc_size);                       \
    layout.rom_size[static_cast<size_t>(loc)]  = static_cast<uint32_t>(size);                                          \
    layout.total_alloc_size                   += static_cast<uint32_t>(size);

    // ROM1 and ROM2 are always present.
    ALLOC_SECTION(RomLocation::ROM1, ROM1_SIZE);

    // mk2 uses 512k rom2, other romsets are all 256k
    if (family == Romset::MK2)
    {
        ALLOC_SECTION(RomLocation::ROM2, ROM2_SIZE);
    }
    else
    {
        ALLOC_SECTION(RomLocation::ROM2, ROM2_SIZE / 2);
    }

    // TODO: move?
    layout.total_alloc_size  = AlignForward(layout.total_alloc_size, alignof(decoder2::DecodedInstruction));
    layout.icache_base       = (uint32_t)layout.total_alloc_size;
    layout.total_alloc_size += sizeof(decoder2::DecodedInstruction) * 16 * 0x10000;

    // TODO: overaligned?
    layout.total_alloc_size = AlignForward(layout.total_alloc_size, (size_t)16);

    // Optional.
    if (RomsetSupportsSubmcu(family))
    {
        ALLOC_SECTION(RomLocation::SMROM, SMROM_SIZE);
    }

    // WAVEROM configuration varies.
    if (IsRequiredRom(family, RomLocation::WAVEROM1))
    {
        ALLOC_SECTION(RomLocation::WAVEROM1, WAVEROM1_SIZE);
    }

    if (IsRequiredRom(family, RomLocation::WAVEROM2))
    {
        ALLOC_SECTION(RomLocation::WAVEROM2, WAVEROM2_SIZE);
    }

    if (IsRequiredRom(family, RomLocation::WAVEROM3))
    {
        ALLOC_SECTION(RomLocation::WAVEROM3, WAVEROM3_SIZE);
    }

    // TODO: we should probably pass a hint whether or not we need
    // these depending on what the rom loader determined is present
    if (IsOptionalRom(family, RomLocation::WAVEROM_CARD))
    {
        ALLOC_SECTION(RomLocation::WAVEROM_CARD, WAVEROM_CARD_SIZE);
    }

    if (IsOptionalRom(family, RomLocation::WAVEROM_EXP))
    {
        ALLOC_SECTION(RomLocation::WAVEROM_EXP, WAVEROM_EXP_SIZE);
    }

#undef ALLOC_SECTION

    layout.instance_count = instance_count;

    for (size_t i = 0; i < instance_count; ++i)
    {
        constexpr auto L1 = std::hardware_destructive_interference_size;

        // over-align first component to avoid false sharing
        layout.total_alloc_size           = AlignForward(layout.total_alloc_size, L1);
        layout.instance_base[i].mcu_base  = static_cast<uint32_t>(layout.total_alloc_size);
        layout.total_alloc_size          += sizeof(mcu_t);

        // TODO: need to modify MCU_Init to support optional sm
        // if (RomsetSupportsSubmcu(family))
        //{
        layout.total_alloc_size              = AlignForward(layout.total_alloc_size, alignof(submcu_t));
        layout.instance_base[i].submcu_base  = static_cast<uint32_t>(layout.total_alloc_size);
        layout.total_alloc_size             += sizeof(submcu_t);
        //}

        layout.total_alloc_size             = AlignForward(layout.total_alloc_size, alignof(mcu_timer_t));
        layout.instance_base[i].timer_base  = static_cast<uint32_t>(layout.total_alloc_size);
        layout.total_alloc_size            += sizeof(mcu_timer_t);

        layout.total_alloc_size           = AlignForward(layout.total_alloc_size, alignof(lcd_t));
        layout.instance_base[i].lcd_base  = static_cast<uint32_t>(layout.total_alloc_size);
        layout.total_alloc_size          += sizeof(lcd_t);

        layout.total_alloc_size           = AlignForward(layout.total_alloc_size, alignof(pcm_t));
        layout.instance_base[i].pcm_base  = static_cast<uint32_t>(layout.total_alloc_size);
        layout.total_alloc_size          += sizeof(pcm_t);
    }
}

bool AllocateMemory(const EmulatorMemoryLayout& layout, EmulatorMemory& mem)
{
    // TODO: user provided allocator
    void* ptr = malloc(layout.total_alloc_size + DEFAULT_ALIGNMENT);

    if (!ptr)
    {
        return false;
    }

    mem.base_ptr    = ptr;
    mem.aligned_ptr = (void*)AlignForward((uintptr_t)ptr, DEFAULT_ALIGNMENT);
    mem.layout      = layout;

    return true;
}

void DeallocateMemory(EmulatorMemory& mem)
{
    free(mem.base_ptr);

    mem.base_ptr    = nullptr;
    mem.aligned_ptr = nullptr;
    mem.layout      = {};
}

void* GetOffsetPtr(EmulatorMemory& mem, uint32_t offset)
{
    unsigned char* const base_ptr = (unsigned char*)mem.aligned_ptr;
    return base_ptr + offset;
}

void* GetMCUPtr(EmulatorMemory& mem, size_t instance_id)
{
    if (instance_id >= mem.layout.instance_count)
    {
        return nullptr;
    }
    if (mem.layout.instance_base[instance_id].mcu_base == UNUSED_COMPONENT)
    {
        return nullptr;
    }
    return GetOffsetPtr(mem, mem.layout.instance_base[instance_id].mcu_base);
}

void* GetSMPtr(EmulatorMemory& mem, size_t instance_id)
{
    if (instance_id >= mem.layout.instance_count)
    {
        return nullptr;
    }
    if (mem.layout.instance_base[instance_id].submcu_base == UNUSED_COMPONENT)
    {
        return nullptr;
    }
    return GetOffsetPtr(mem, mem.layout.instance_base[instance_id].submcu_base);
}

void* GetTimerPtr(EmulatorMemory& mem, size_t instance_id)
{
    if (instance_id >= mem.layout.instance_count)
    {
        return nullptr;
    }
    if (mem.layout.instance_base[instance_id].timer_base == UNUSED_COMPONENT)
    {
        return nullptr;
    }
    return GetOffsetPtr(mem, mem.layout.instance_base[instance_id].timer_base);
}

void* GetLCDPtr(EmulatorMemory& mem, size_t instance_id)
{
    if (instance_id >= mem.layout.instance_count)
    {
        return nullptr;
    }
    if (mem.layout.instance_base[instance_id].lcd_base == UNUSED_COMPONENT)
    {
        return nullptr;
    }
    return GetOffsetPtr(mem, mem.layout.instance_base[instance_id].lcd_base);
}

void* GetPCMPtr(EmulatorMemory& mem, size_t instance_id)
{
    if (instance_id >= mem.layout.instance_count)
    {
        return nullptr;
    }
    if (mem.layout.instance_base[instance_id].pcm_base == UNUSED_COMPONENT)
    {
        return nullptr;
    }
    return GetOffsetPtr(mem, mem.layout.instance_base[instance_id].pcm_base);
}

bool ConstructEmulator(EmulatorMemory& mem, size_t instance_id)
{
    if (instance_id >= mem.layout.instance_count)
    {
        return false;
    }

    const auto& bases = mem.layout.instance_base[instance_id];

    unsigned char* const base_ptr = (unsigned char*)mem.aligned_ptr;

    if (bases.mcu_base != UNUSED_COMPONENT)
    {
        new (base_ptr + bases.mcu_base) mcu_t();
    }

    if (bases.submcu_base != UNUSED_COMPONENT)
    {
        new (base_ptr + bases.submcu_base) submcu_t();
    }

    if (bases.timer_base != UNUSED_COMPONENT)
    {
        new (base_ptr + bases.timer_base) mcu_timer_t();
    }

    if (bases.lcd_base != UNUSED_COMPONENT)
    {
        new (base_ptr + bases.lcd_base) lcd_t();
    }

    if (bases.pcm_base != UNUSED_COMPONENT)
    {
        new (base_ptr + bases.pcm_base) pcm_t();
    }

    return true;
}
