// Somewhat minimal MIDI file reader.

#pragma once

#include <cstdint>
#include <vector>

// Should use C++17 span but the project only targets C++11 for now.
class SMF_ByteSpan
{
public:
    SMF_ByteSpan() = default;

    SMF_ByteSpan(const std::vector<uint8_t>& source)
        : ptr(source.data()), len(source.size())
    {
    }

    ~SMF_ByteSpan() = default;
    // copyable
    SMF_ByteSpan(const SMF_ByteSpan&) = default;
    SMF_ByteSpan& operator=(const SMF_ByteSpan&) = default;
    // moveable
    SMF_ByteSpan(SMF_ByteSpan&&) = default;
    SMF_ByteSpan& operator=(SMF_ByteSpan&&) = default;

    const uint8_t& operator[](size_t offset) const
    {
        return ptr[offset];
    }

    size_t Size() const
    {
        return len;
    }

private:
    const uint8_t* ptr = nullptr;
    size_t         len = 0;
};

struct SMF_Header
{
    uint16_t format;
    uint16_t ntrks;
    uint16_t division;
};

struct SMF_Event
{
    // Position of this event within the track. Used during sorting so events
    // with the same timestamp preserve ordering.
    uint64_t seq_id;
    // Absolute timestamp relative to track start.
    uint64_t timestamp;
    // Time since track start (only for first event in a track) or the prior event.
    uint32_t delta_time;
    // MIDI message type.
    uint8_t status;
    // Offset to raw data bytes for this message within an SMF_ByteSpan.
    uint32_t data_first, data_last;

    bool IsMetaEvent() const
    {
        return status == 0xff;
    }

    bool IsTempo(SMF_ByteSpan bytes) const
    {
        return IsMetaEvent() && bytes[data_first] == 0x51;
    }

    uint32_t GetTempoUS(SMF_ByteSpan bytes) const
    {
        return ((uint32_t)bytes[data_first + 2]) << 16 |
               ((uint32_t)bytes[data_first + 3]) << 8  |
               ((uint32_t)bytes[data_first + 4]);
    }
};

struct SMF_Track
{
    std::vector<SMF_Event> events;
};

struct SMF_Data
{
    SMF_Header header;
    std::vector<uint8_t> bytes;
    std::vector<SMF_Track> tracks;
};

SMF_Track SMF_MergeTracks(const SMF_Data& data);
void SMF_PrintStats(const SMF_Data& data);
SMF_Data SMF_LoadEvents(const char* filename);

inline uint64_t SMF_TicksToUS(uint64_t ticks, uint64_t us_per_qn, uint64_t division)
{
    return ticks * us_per_qn / division;
}

