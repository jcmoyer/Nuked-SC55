#include "wav.h"

#include <cassert>
#include <cstring>

void WAV_WriteBytes(std::ofstream& output, const char* bytes, size_t len)
{
    output.write(bytes, len);
}

void WAV_WriteCString(std::ofstream& output, const char* s)
{
    WAV_WriteBytes(output, s, strlen(s));
}

void WAV_WriteU16LE(std::ofstream& output, uint16_t value)
{
    // TODO: don't assume LE host
    output.write((const char*)&value, sizeof(uint16_t));
}

void WAV_WriteU32LE(std::ofstream& output, uint32_t value)
{
    // TODO: don't assume LE host
    output.write((const char*)&value, sizeof(uint32_t));
}

void WAV_Handle::Open(const char* filename)
{
    output.open(filename, std::ios::binary);

    char zeroes[44] = {0};
    output.write(zeroes, sizeof(zeroes));
}

void WAV_Handle::Close()
{
    output.close();
}

void WAV_Handle::WriteSample(short left, short right)
{
    output.write((const char*)&left, sizeof(short));
    output.write((const char*)&right, sizeof(short));
    ++samples_written;
}

void WAV_Handle::Finish(uint32_t sample_rate)
{
    const uint32_t data_size = samples_written * 2 * 2;

    // go back and fill in the header
    output.seekp(0);

    // RIFF header
    WAV_WriteCString(output, "RIFF");
    WAV_WriteU32LE(output, 36 + data_size);
    WAV_WriteCString(output, "WAVE");
    // fmt
    WAV_WriteCString(output, "fmt ");
    WAV_WriteU32LE(output, 16);
    WAV_WriteU16LE(output, 1);
    WAV_WriteU16LE(output, 2);
    WAV_WriteU32LE(output, sample_rate);
    WAV_WriteU32LE(output, sample_rate * 2 * 2);
    WAV_WriteU16LE(output, 2 * 2);
    WAV_WriteU16LE(output, 16);
    // data
    WAV_WriteCString(output, "data");
    WAV_WriteU32LE(output, samples_written * 2 * 2);

    assert(output.tellp() == 44);

    Close();
}

