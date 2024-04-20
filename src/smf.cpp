#include "smf.h"

#include <algorithm>
#include <cstdio>
#include <string>
#include <fstream>

static void SMF_ReadU16BE(std::ifstream& input, uint16_t& value)
{
    input.read((char*)&value, sizeof(uint16_t));

    // TODO: don't assume LE host
    value =
        ((value & 0x00FF) << 8) |
        ((value & 0xFF00) >> 8);
}

static void SMF_ReadU32BE(std::ifstream& input, uint32_t& value)
{
    input.read((char*)&value, sizeof(uint32_t));

    // TODO: don't assume LE host
    value =
        ((value & 0x000000FF) << 24) |
        ((value & 0x0000FF00) << 8)  |
        ((value & 0x00FF0000) >> 8)  |
        ((value & 0xFF000000) >> 24);
}

static void SMF_ReadHeader(std::ifstream& input, SMF_Header& header)
{
    SMF_ReadU16BE(input, header.format);
    SMF_ReadU16BE(input, header.ntrks);
    SMF_ReadU16BE(input, header.division);
}

static void SMF_ReadByte(std::ifstream& input, uint8_t& value)
{
    input.read((char*)&value, 1);
}

static void SMF_ReadVarint(std::ifstream& input, uint32_t& value)
{
    value = 0;
    for (int i = 0; i < 4; ++i)
    {
        uint8_t byte;
        input.read((char*)&byte, 1);
        value = (value << 7) | (uint32_t)(byte & 0x7F);
        if ((byte & 0x80) == 0)
        {
            break;
        }
    }
}

static void SMF_ReadBytes(std::ifstream& input, std::vector<uint8_t>& value, size_t count)
{
    value.resize(count);
    input.read((char*)value.data(), count);
}

SMF_Track SMF_MergeTracks(const SMF_Data& data)
{
    SMF_Track merged_track;
    for (const SMF_Track& track : data.tracks)
    {
        merged_track.events.insert(
            merged_track.events.end(),
            track.events.begin(),
            track.events.end());
    }
    std::sort(merged_track.events.begin(), merged_track.events.end(), [](const SMF_Event& left, const SMF_Event& right) {
        if (left.timestamp == right.timestamp)
        {
            return left.seq_id < right.seq_id;
        }
        return left.timestamp < right.timestamp;
    });
    for (size_t i = 1; i < merged_track.events.size(); ++i)
    {
        merged_track.events[i].delta_time =
            merged_track.events[i].timestamp - merged_track.events[i - 1].timestamp;
    }
    return merged_track;
}

void SMF_LoadTrack(std::ifstream& input, SMF_Data& result, uint32_t expected_end)
{
    uint8_t last_status = 0;
    uint64_t total_time = 0;

    result.tracks.emplace_back();
    SMF_Track& new_track = result.tracks.back();

    for (;;)
    {
        uint64_t offset = input.tellg();
        if (offset == expected_end)
        {
            break;
        }
        else if (offset > expected_end)
        {
            printf("Read past expected track end; midi file may be malformed\n");
            exit(1);
        }

        uint32_t delta_time;
        SMF_ReadVarint(input, delta_time);

        uint8_t event_type;
        SMF_ReadByte(input, event_type);

        bool is_status = (event_type & 0x80) != 0;
        if (is_status)
        {
            last_status = event_type;
        }
        else
        {
            // put back, this is actually a data byte
            input.seekg(-1, std::ios::cur);
        }

        total_time += delta_time;

        new_track.events.emplace_back();
        SMF_Event& new_event = new_track.events.back();
        new_event.seq_id = new_track.events.size();
        new_event.delta_time = delta_time;
        new_event.timestamp = total_time;
        uint8_t a, b;

        switch (last_status & 0xF0)
        {
            // 2 param
            case 0x80:
            case 0x90:
            case 0xA0:
            case 0xB0:
            case 0xE0:
                SMF_ReadByte(input, a);
                SMF_ReadByte(input, b);
                new_event.payload.push_back(last_status);
                new_event.payload.push_back(a);
                new_event.payload.push_back(b);
                break;
            // 1 param
            case 0xC0:
            case 0xD0:
                SMF_ReadByte(input, a);
                new_event.payload.push_back(last_status);
                new_event.payload.push_back(a);
                break;
            // variable length
            case 0xF0:
                {
                    uint8_t mode = event_type & 0x0F;

                    uint8_t meta_type;
                    uint32_t meta_len;
                    std::vector<uint8_t> meta_payload;

                    switch (mode)
                    {
                        case 0xF:
                            SMF_ReadByte(input, meta_type);
                            SMF_ReadVarint(input, meta_len);
                            SMF_ReadBytes(input, meta_payload, meta_len);

                            new_event.payload.push_back(event_type);
                            new_event.payload.push_back(meta_type);
                            // TODO: not correct, need to turn varint back into bytes
                            // probably should just load the whole file into memory so we can look backwards for free
                            new_event.payload.push_back(meta_len);
                            new_event.payload.insert(new_event.payload.end(), meta_payload.begin(), meta_payload.end());

                            printf("read but unhandled FF mode %d size %d at %d\n", mode, meta_len, (int)input.tellg());
                            break;

                        default:
                            printf("unhandled Fx message: %x\n", event_type);
                            break;
                    }
                }
                break;
        }
    }
}

void SMF_PrintStats(const SMF_Data& data)
{
    for (size_t i = 0; i < data.tracks.size(); ++i)
    {
        fprintf(stderr, "Track %02lld: %lld events\n", i, data.tracks[i].events.size());
    }
}

SMF_Data SMF_LoadEvents(const char* filename)
{
    std::ifstream input(filename, std::ios::binary);

    if (!input)
    {
        printf("Failed to open input\n");
        exit(1);
    }

    SMF_Data data;

    for (;;)
    {
        std::string chunk_type;
        chunk_type.resize(4);
        input.read((char*)chunk_type.data(), 4);

        if (input.eof())
        {
            break;
        }

        uint32_t chunk_size = 0;
        SMF_ReadU32BE(input, chunk_size);

        uint64_t chunk_data_offset = input.tellg();

        uint64_t expected_end = chunk_data_offset + chunk_size;

        if (chunk_type == "MThd")
        {
            SMF_ReadHeader(input, data.header);
            input.seekg(expected_end, std::ios::beg);
        }
        else if (chunk_type == "MTrk")
        {
            SMF_LoadTrack(input, data, expected_end);
        }
        else
        {
            printf("Unexpected chunk type '%s' at %d \n", chunk_type.c_str(), (int) input.tellg());
            exit(1);
        }
    }

    return data;
}

