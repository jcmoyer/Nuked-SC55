// We use fopen()
#define _CRT_SECURE_NO_WARNINGS

#include "wav.h"

#include <cassert>
#include <cstring>

void WAV_WriteBytes(FILE* output, const char* bytes, size_t len)
{
    fwrite(bytes, 1, len, output);
}

void WAV_WriteCString(FILE* output, const char* s)
{
    WAV_WriteBytes(output, s, strlen(s));
}

void WAV_WriteU16LE(FILE* output, uint16_t value)
{
    // TODO: don't assume LE host
    WAV_WriteBytes(output, (const char*)&value, sizeof(uint16_t));
}

void WAV_WriteU32LE(FILE* output, uint32_t value)
{
    // TODO: don't assume LE host
    WAV_WriteBytes(output, (const char*)&value, sizeof(uint32_t));
}

WAV_Handle::~WAV_Handle()
{
    Close();
}

WAV_Handle::WAV_Handle(WAV_Handle&& rhs) noexcept
{
    m_output         = rhs.m_output;
    rhs.m_output     = nullptr;
    m_sample_rate    = rhs.m_sample_rate;
    m_frames_written = rhs.m_frames_written;
}

WAV_Handle& WAV_Handle::operator=(WAV_Handle&& rhs) noexcept
{
    Close();
    m_output         = rhs.m_output;
    rhs.m_output     = nullptr;
    m_sample_rate    = rhs.m_sample_rate;
    m_frames_written = rhs.m_frames_written;
    return *this;
}

void WAV_Handle::SetSampleRate(uint32_t sample_rate)
{
    m_sample_rate = sample_rate;
}

void WAV_Handle::OpenStdout()
{
    m_output = stdout;
}

void WAV_Handle::Open(const char* filename)
{
    m_output = fopen(filename, "wb");
    fseek(m_output, 44, SEEK_SET);
}

void WAV_Handle::Close()
{
    if (m_output && m_output != stdout)
    {
        fclose(m_output);
    }
    m_output = nullptr;
}

void WAV_Handle::WriteSample(int16_t left, int16_t right)
{
    WAV_WriteBytes(m_output, (const char*)&left, sizeof(int16_t));
    WAV_WriteBytes(m_output, (const char*)&right, sizeof(int16_t));
    ++m_frames_written;
}

void WAV_Handle::Finish()
{
    // we wrote raw samples, nothing to do
    if (m_output == stdout)
    {
        return;
    }

    // go back and fill in the header
    fseek(m_output, 0, SEEK_SET);

    const uint32_t data_size = m_frames_written * 2 * 2;

    // RIFF header
    WAV_WriteCString(m_output, "RIFF");
    WAV_WriteU32LE(m_output, 36 + data_size);
    WAV_WriteCString(m_output, "WAVE");
    // fmt
    WAV_WriteCString(m_output, "fmt ");
    WAV_WriteU32LE(m_output, 16);
    WAV_WriteU16LE(m_output, 1);
    WAV_WriteU16LE(m_output, 2);
    WAV_WriteU32LE(m_output, m_sample_rate);
    WAV_WriteU32LE(m_output, m_sample_rate * 2 * 2);
    WAV_WriteU16LE(m_output, 2 * 2);
    WAV_WriteU16LE(m_output, 16);
    // data
    WAV_WriteCString(m_output, "data");
    WAV_WriteU32LE(m_output, m_frames_written * 2 * 2);

    assert(ftell(m_output) == 44);

    Close();
}

