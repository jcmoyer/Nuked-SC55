// This is a very minimal WAVE writer. It only exists to output something other
// than raw sample data.

#pragma once

#include <fstream>
#include <cstdint>

class WAV_Handle
{
public:
    WAV_Handle() = default;
    ~WAV_Handle() = default;
    // moveable
    WAV_Handle(WAV_Handle&&) = default;
    WAV_Handle& operator=(WAV_Handle&&) = default;
    // noncopyable
    WAV_Handle(const WAV_Handle&) = delete;
    WAV_Handle& operator=(const WAV_Handle&) = delete;

    void Open(const char* filename);
    void Close();
    void WriteSample(short left, short right);
    void Finish(uint32_t sample_rate);

private:
    std::ofstream output;
    uint64_t samples_written = 0;
};

