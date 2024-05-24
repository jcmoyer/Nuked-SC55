// This is a very minimal WAVE writer. It only exists to output something other
// than raw sample data.

#pragma once

#include <cstdio>
#include <cstdint>

class WAV_Handle
{
public:
    WAV_Handle() = default;
    ~WAV_Handle();
    // moveable
    WAV_Handle(WAV_Handle&&) noexcept;
    WAV_Handle& operator=(WAV_Handle&&) noexcept;
    // noncopyable
    WAV_Handle(const WAV_Handle&) = delete;
    WAV_Handle& operator=(const WAV_Handle&) = delete;

    void SetSampleRate(uint32_t sample_rate);

    void OpenStdout();
    void Open(const char* filename);
    void Close();
    void WriteSample(int16_t left, int16_t right);
    void Finish();

private:
    FILE*    m_output         = nullptr;
    uint64_t m_frames_written = 0;
    uint32_t m_sample_rate;
};
