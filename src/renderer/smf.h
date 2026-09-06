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

// Somewhat minimal MIDI file reader.

#pragma once

#include <cstdint>
#include <filesystem>
#include <span>
#include <vector>

using SMF_ByteSpan = std::span<const uint8_t>;

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
    // Number of MTrk chunk containing this event.
    uint16_t track_id;
    // MIDI message type.
    uint8_t status;
    // Offset to raw data bytes for this message within an SMF_ByteSpan.
    size_t data_first, data_last;

    uint8_t GetChannel() const
    {
        return status & 0x0f;
    }

    bool IsControlChange() const
    {
        return (status & 0xf0) == 0xb0;
    }

    bool IsSystem() const
    {
        return status >= 0xf0;
    }

    bool IsSystemExclusive() const
    {
        return status == 0xf0 || status == 0xf7;
    }

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

    SMF_ByteSpan GetData(SMF_ByteSpan bytes) const
    {
        return SMF_ByteSpan(&bytes[data_first], &bytes[data_last]);
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

const size_t SMF_CHANNEL_COUNT = 16;

void SMF_SetDeltasFromTimestamps(SMF_Track& track);
SMF_Track SMF_MergeTracks(const SMF_Data& data);
void SMF_PrintStats(const SMF_Data& data);
SMF_Data SMF_LoadEvents(const char* filename);
SMF_Data SMF_LoadEvents(const std::filesystem::path& filename);

inline uint64_t SMF_TicksToUS(uint64_t ticks, uint64_t us_per_qn, uint64_t division)
{
    return ticks * us_per_qn / division;
}

