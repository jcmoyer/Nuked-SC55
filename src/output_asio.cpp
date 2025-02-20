#include "output_asio.h"

#include "asio_config.h"

#include "asiosys.h"
// asiosys *must* be included before these headers
#include "asio.h"
#include "asiodrivers.h"

#include "ringbuffer.h"
#include <atomic>

// number of buffers, one per stereo channel
const size_t N_BUFFERS = 2;

// one per instance
const size_t MAX_STREAMS = 16;

struct GlobalAsioState
{
    AsioDrivers    drivers;
    ASIODriverInfo driver_info;
    ASIOCallbacks  callbacks;

    ASIOBufferInfo   buffer_info[N_BUFFERS]{};
    ASIOChannelInfo  channel_info[N_BUFFERS]{};
    SDL_AudioStream* streams[MAX_STREAMS]{};
    size_t           stream_count = 0;

    long min_size;
    long max_size;
    long preferred_size;
    long granularity;

    long input_channel_count;
    long output_channel_count;

    std::atomic<bool> defer_reset;

    ASIOSampleType output_type;

    int huge[0x400000]{};

    // TODO: use this instead of huge
    GenericBuffer mix_buffer;
};

// defined in ASIO SDK
// we do actually need to do the loading through this function or else we'll segfault on exit
bool loadAsioDriver(char *name);

static ASIOTime* bufferSwitchTimeInfo(ASIOTime* params, long index, ASIOBool directProcess);
static void      bufferSwitch(long index, ASIOBool processNow);
static void      sampleRateDidChange(ASIOSampleRate sRate);
static long      asioMessage(long selector, long value, void* message, double* opt);

static const char* SampleTypeToString(ASIOSampleType type)
{
    switch (type)
    {
    case ASIOSTInt16MSB:
        return "ASIOSTInt16MSB";
    case ASIOSTInt24MSB:
        return "ASIOSTInt24MSB";
    case ASIOSTInt32MSB:
        return "ASIOSTInt32MSB";
    case ASIOSTFloat32MSB:
        return "ASIOSTFloat32MSB";
    case ASIOSTFloat64MSB:
        return "SIOSTFloat64MSB";
    case ASIOSTInt32MSB16:
        return "ASIOSTInt32MSB16";
    case ASIOSTInt32MSB18:
        return "ASIOSTInt32MSB18";
    case ASIOSTInt32MSB20:
        return "ASIOSTInt32MSB20";
    case ASIOSTInt32MSB24:
        return "ASIOSTInt32MSB24";
    case ASIOSTInt16LSB:
        return "ASIOSTInt16LSB";
    case ASIOSTInt24LSB:
        return "ASIOSTInt24LSB";
    case ASIOSTInt32LSB:
        return "ASIOSTInt32LSB";
    case ASIOSTFloat32LSB:
        return "ASIOSTFloat32LSB";
    case ASIOSTFloat64LSB:
        return "ASIOSTFloat64LSB";
    case ASIOSTInt32LSB16:
        return "ASIOSTInt32LSB16";
    case ASIOSTInt32LSB18:
        return "ASIOSTInt32LSB18";
    case ASIOSTInt32LSB20:
        return "ASIOSTInt32LSB20";
    case ASIOSTInt32LSB24:
        return "ASIOSTInt32LSB24";
    case ASIOSTDSDInt8LSB1:
        return "ASIOSTDSDInt8LSB1";
    case ASIOSTDSDInt8MSB1:
        return "ASIOSTDSDInt8MSB1";
    case ASIOSTDSDInt8NER8:
        return "ASIOSTDSDInt8NER8";
    default:
        return "Unknown sample type";
    }
}

// there isn't a way around using globals here, the ASIO API doesn't accept
// arbitrary userdata in its callbacks
GlobalAsioState g_asio_state;

bool Out_ASIO_QueryOutputs(AudioOutputList& list)
{
    const size_t MAX_NAMES    = 32;
    const size_t MAX_NAME_LEN = 32;

    // TODO: wat. do we seriously need to allocate all this?
    char* names[MAX_NAMES];
    for (size_t i = 0; i < MAX_NAMES; ++i)
    {
        names[i] = (char*)malloc(MAX_NAME_LEN);
    }

    long names_count = g_asio_state.drivers.getDriverNames(names, MAX_NAMES);
    for (long i = 0; i < names_count; ++i)
    {
        list.push_back({.name = names[i], .kind = AudioOutputKind::ASIO});
    }

    for (size_t i = 0; i < MAX_NAMES; ++i)
    {
        free(names[i]);
    }

    return true;
}

bool Out_ASIO_Start(const char* driver_name)
{
    // for some reason the api wants a non-const pointer
    char internal_driver_name[256]{};
    if (strcpy_s(internal_driver_name, sizeof(internal_driver_name), driver_name) != 0)
    {
        fprintf(stderr, "Driver name too long: `%s`\n", driver_name);
        return false;
    }

    if (!loadAsioDriver(internal_driver_name))
    {
        fprintf(stderr, "Failed to load ASIO driver `%s`\n", internal_driver_name);
        return false;
    }

    if (ASIOInit(&g_asio_state.driver_info) != ASE_OK)
    {
        fprintf(stderr, "ASIOInit failed\n");
        return false;
    }

    fprintf(stderr,
            "asioVersion:   %ld\n"
            "driverVersion: %ld\n"
            "name:          %s\n"
            "errorMessage:  %s\n",
            g_asio_state.driver_info.asioVersion,
            g_asio_state.driver_info.driverVersion,
            g_asio_state.driver_info.name,
            g_asio_state.driver_info.errorMessage);

    if (ASIOGetBufferSize(
            &g_asio_state.min_size, &g_asio_state.max_size, &g_asio_state.preferred_size, &g_asio_state.granularity) !=
        ASE_OK)
    {
        fprintf(stderr, "ASIOGetBufferSize failed\n");
        return false;
    }

    fprintf(stderr,
            "ASIOGetBufferSize (min: %ld, max: %ld, preferred: %ld, granularity: %ld);\n",
            g_asio_state.min_size,
            g_asio_state.max_size,
            g_asio_state.preferred_size,
            g_asio_state.granularity);

    // ASIO4ALL can't handle the sample rate the emulator uses, so we'll need
    // to use a more common one and resample
    if (ASIOSetSampleRate(44100) != ASE_OK)
    {
        fprintf(stderr, "ASIOSetSampleRate(44100) failed; trying to continue anyways\n");
    }

    if (ASIOGetChannels(&g_asio_state.input_channel_count, &g_asio_state.output_channel_count))
    {
        fprintf(stderr, "ASIOGetChannels failed\n");
        return false;
    }

    fprintf(stderr,
            "Available channels: %ld in, %ld out\n",
            g_asio_state.input_channel_count,
            g_asio_state.output_channel_count);

    if ((size_t)g_asio_state.output_channel_count < N_BUFFERS)
    {
        fprintf(stderr, "%" PRIu64 " channels required; aborting\n", N_BUFFERS);
        return false;
    }

    for (size_t i = 0; i < N_BUFFERS; ++i)
    {
        g_asio_state.buffer_info[i].isInput    = ASIOFalse;
        g_asio_state.buffer_info[i].channelNum = (long)i;
        g_asio_state.buffer_info[i].buffers[0] = nullptr;
        g_asio_state.buffer_info[i].buffers[1] = nullptr;
    }

    g_asio_state.callbacks.bufferSwitch         = bufferSwitch;
    g_asio_state.callbacks.bufferSwitchTimeInfo = bufferSwitchTimeInfo;
    g_asio_state.callbacks.sampleRateDidChange  = sampleRateDidChange;
    g_asio_state.callbacks.asioMessage          = asioMessage;

    // TODO error handling
    ASIOCreateBuffers(g_asio_state.buffer_info, N_BUFFERS, g_asio_state.preferred_size, &g_asio_state.callbacks);

    for (size_t i = 0; i < N_BUFFERS; ++i)
    {
        g_asio_state.channel_info[i].channel = g_asio_state.buffer_info[i].channelNum;
        ASIOGetChannelInfo(&g_asio_state.channel_info[i]);

        fprintf(stderr,
                "ASIO channel %" PRIu64 ": %s: %s\n",
                i,
                g_asio_state.channel_info[i].name,
                SampleTypeToString(g_asio_state.channel_info[i].type));
    }

    g_asio_state.output_type = g_asio_state.channel_info[0].type;

    for (size_t i = 1; i < N_BUFFERS; ++i)
    {
        if (g_asio_state.output_type != g_asio_state.channel_info[i].type)
        {
            fprintf(stderr, "ASIO channel %" PRIu64 " has a different output type!\n", i);
            return false;
        }
    }

    ASIOStart();

    return true;
}

void Out_ASIO_AddStream(SDL_AudioStream* stream)
{
    if (g_asio_state.stream_count == MAX_STREAMS)
    {
        fprintf(stderr, "PANIC: attemped to add more than %" PRIu64 " ASIO streams\n", MAX_STREAMS);
        exit(1);
    }
    g_asio_state.streams[g_asio_state.stream_count] = stream;
    ++g_asio_state.stream_count;
}

double Out_ASIO_GetFrequency()
{
    ASIOSampleRate rate;
    ASIOGetSampleRate(&rate);
    return rate;
}

SDL_AudioFormat Out_ASIO_GetFormat()
{
    switch (g_asio_state.output_type)
    {
    case ASIOSTInt16LSB:
        return AUDIO_S16LSB;
    case ASIOSTInt32LSB:
        return AUDIO_S32LSB;
    case ASIOSTFloat32LSB:
        return AUDIO_F32LSB;
    case ASIOSTInt16MSB:
        return AUDIO_S16MSB;
    case ASIOSTInt32MSB:
        return AUDIO_S32MSB;
    case ASIOSTFloat32MSB:
        return AUDIO_F32MSB;
    default:
        fprintf(stderr, "PANIC: ASIO format conversion not implemented\n");
        exit(1);
    }
}

void Out_ASIO_Stop()
{
    ASIOExit();
}

bool Out_ASIO_IsResetRequested()
{
    return g_asio_state.defer_reset;
}

void Out_ASIO_Reset()
{
    ASIOStop();
    Out_ASIO_Start(g_asio_state.driver_info.name);
    g_asio_state.defer_reset = false;
}

static ASIOTime* bufferSwitchTimeInfo(ASIOTime* params, long index, ASIOBool directProcess)
{
    (void)params;
    (void)directProcess;

    size_t renderable_frames = (size_t)g_asio_state.preferred_size;
    for (size_t i = 0; i < g_asio_state.stream_count; ++i)
    {
        // TODO /8 varies with asio output format
        renderable_frames = Min(renderable_frames, (size_t)SDL_AudioStreamAvailable(g_asio_state.streams[i]) / 8);
    }

    memset(g_asio_state.buffer_info[0].buffers[index], 0, g_asio_state.preferred_size * 4);
    memset(g_asio_state.buffer_info[1].buffers[index], 0, g_asio_state.preferred_size * 4);

    if (renderable_frames >= (size_t)g_asio_state.preferred_size)
    {
        for (size_t i = 0; i < g_asio_state.stream_count; ++i)
        {
            SDL_AudioStreamGet(g_asio_state.streams[i], g_asio_state.huge, g_asio_state.preferred_size * 2 * 4);

            for (int j = 0; j < g_asio_state.preferred_size; ++j)
            {
                ((int32_t*)(g_asio_state.buffer_info[0].buffers[index]))[j] = g_asio_state.huge[2 * j + 0];
                ((int32_t*)(g_asio_state.buffer_info[1].buffers[index]))[j] = g_asio_state.huge[2 * j + 1];
            }
        }
    }

    // TODO: supposed to query if this optimization is available
    ASIOOutputReady();

    return 0;
}

static void bufferSwitch(long index, ASIOBool processNow)
{
    ASIOTime timeInfo;
    memset(&timeInfo, 0, sizeof(timeInfo));
    bufferSwitchTimeInfo(&timeInfo, index, processNow);
}

static void sampleRateDidChange(ASIOSampleRate sRate)
{
    fprintf(stderr, "ASIO: sample rate changed to %f\n", sRate);
}

static long asioMessage(long selector, long value, void* message, double* opt)
{
    (void)value;
    (void)message;
    (void)opt;

    switch (selector)
    {
    case kAsioEngineVersion:
        return 2;
    case kAsioSupportsTimeCode:
        return 0;
    case kAsioResetRequest:
        g_asio_state.defer_reset = true;
        return 1;
    default:
        return 1;
    }
}
