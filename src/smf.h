// Somewhat minimal MIDI file reader.

#pragma once

#include <cstdint>
#include <vector>

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
    // Raw data bytes representing this message.
    std::vector<uint8_t> payload;

    bool is_metaevent() const
    {
        return payload.size() > 0 && payload[0] == 0xff;
    }

    bool is_tempo() const
    {
        return payload.size() == 6 && payload[0] == 0xff && payload[1] == 0x51;
    }

    uint32_t get_tempo_us() const
    {
        return ((uint32_t)payload[3]) << 16 |
               ((uint32_t)payload[4]) << 8  |
               ((uint32_t)payload[5]);
    }
};

struct SMF_Track
{
    std::vector<SMF_Event> events;
};

struct SMF_Data
{
    SMF_Header header;
    std::vector<SMF_Track> tracks;
};

SMF_Track SMF_MergeTracks(const SMF_Data& data);
SMF_Data SMF_LoadEvents(const char* filename);

inline uint64_t SMF_TicksToUS(uint64_t ticks, uint64_t us_per_qn, uint64_t division)
{
    return ticks * us_per_qn / division;
}

