#include "output_asio.h"

#include "asiosys.h"
// asiosys *must* be included before these headers
#include "asio.h"
#include "asiodrivers.h"

#include "audio.h"
#include "audio_sdl.h"
#include "common/command_line.h"
#include "math_util.h"
#include "ringbuffer.h"
#include <atomic>

// number of buffers, one per stereo channel
const size_t N_BUFFERS = 2;

// one per instance
const size_t MAX_STREAMS = 16;

// max number of supported asio output channels
const size_t MAX_CHANNELS = 32;

struct ASIOOutput
{
    AsioDrivers    drivers;
    ASIODriverInfo driver_info;
    ASIOCallbacks  callbacks;

    ASIOBufferInfo   buffer_info[N_BUFFERS]{};
    ASIOChannelInfo  channel_info[MAX_CHANNELS]{};
    SDL_AudioStream* streams[MAX_STREAMS]{};
    size_t           stream_count = 0;

    // Size of a buffer as requested by ASIO driver
    long min_size;
    long max_size;
    long preferred_size;
    long granularity;

    // Size of a buffer as it will be used
    size_t buffer_size_bytes;
    size_t buffer_size_frames;

    // Output frequency the driver is actually using
    ASIOSampleRate actual_freq;

    long input_channel_count;
    long output_channel_count;

    std::atomic<bool> defer_reset;

    ASIOSampleType output_type;

    // Contains interleaved frames received from individual `streams`.
    // This is necessarily 2 * `buffer_size_bytes` long.
    GenericBuffer mix_buffers[2]{};

    // Parameters requested by the user
    ASIO_OutputParameters create_params;

    long left_channel;
    long right_channel;
};

// there isn't a way around using globals here, the ASIO API doesn't accept arbitrary userdata in its callbacks
static ASIOOutput g_output;

// defined in ASIO SDK
// we do actually need to do the loading through this function or else we'll segfault on exit
bool loadAsioDriver(char* name);

static ASIOTime* bufferSwitchTimeInfo(ASIOTime* params, long index, ASIOBool directProcess);
static void      bufferSwitch(long index, ASIOBool processNow);
static void      sampleRateDidChange(ASIOSampleRate sRate);
static long      asioMessage(long selector, long value, void* message, double* opt);

static const char* ErrorToString(ASIOError err)
{
    switch (err)
    {
    case ASE_OK:
        return "ASE_OK";
    case ASE_SUCCESS:
        return "ASE_SUCCESS";
    case ASE_NotPresent:
        return "ASE_NotPresent";
    case ASE_HWMalfunction:
        return "ASE_HWMalfunction";
    case ASE_InvalidParameter:
        return "ASE_InvalidParameter";
    case ASE_InvalidMode:
        return "ASE_InvalidMode";
    case ASE_SPNotAdvancing:
        return "ASE_SPNotAdvancing";
    case ASE_NoClock:
        return "ASE_NoClock";
    case ASE_NoMemory:
        return "ASE_NoMemory";
    default:
        return "Unknown error code";
    }
}

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

// pre: g_output.channel_info populated
bool Out_ASIO_PickOutputChannel(std::string_view name, long& channel_id)
{
    // first try name interpretation
    for (long i = 0; i < g_output.output_channel_count; ++i)
    {
        if (name == g_output.channel_info[i].name)
        {
            channel_id = i;
            return true;
        }
    }

    // maybe the user provided an integer ID instead
    long name_as_long = 0;
    if (common::TryParse(name, name_as_long) && name_as_long < g_output.output_channel_count)
    {
        channel_id = name_as_long;
        return true;
    }

    return false;
}

bool Out_ASIO_QueryOutputs(AudioOutputList& list)
{
    // max number of ASIO drivers supported by this program
    const size_t MAX_NAMES = 32;

    // ASIO limitation
    const size_t MAX_NAME_LEN = 32;

    char  names_buffer[MAX_NAMES * MAX_NAME_LEN];
    char* names[MAX_NAMES];
    for (size_t i = 0; i < MAX_NAMES; ++i)
    {
        names[i] = &names_buffer[i * MAX_NAME_LEN];
    }

    long names_count = g_output.drivers.getDriverNames(names, MAX_NAMES);
    for (long i = 0; i < names_count; ++i)
    {
        list.push_back({.name = names[i], .kind = AudioOutputKind::ASIO});
    }

    return true;
}

bool Out_ASIO_OpenDriver(const char* driver_name)
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

    ASIOError err;

    err = ASIOInit(&g_output.driver_info);
    if (err != ASE_OK)
    {
        fprintf(stderr, "ASIOInit failed with %s: %s\n", ErrorToString(err), g_output.driver_info.errorMessage);
        return false;
    }

    return true;
}

void Out_ASIO_CloseDriver()
{
    ASIOExit();
}

bool Out_ASIO_QueryChannels(const char* driver_name, ASIO_OutputChannelList& list)
{
    list.clear();

    if (!Out_ASIO_OpenDriver(driver_name))
    {
        return false;
    }

    ASIOError err;

    err = ASIOGetChannels(&g_output.input_channel_count, &g_output.output_channel_count);
    if (err != ASE_OK)
    {
        Out_ASIO_CloseDriver();
        return false;
    }

    if ((size_t)g_output.output_channel_count > MAX_CHANNELS)
    {
        g_output.output_channel_count = MAX_CHANNELS;
    }

    list.reserve((size_t)g_output.output_channel_count);

    for (long i = 0; i < g_output.output_channel_count; ++i)
    {
        g_output.channel_info[i].channel = i;
        g_output.channel_info[i].isInput = ASIOFalse;

        err = ASIOGetChannelInfo(&g_output.channel_info[i]);
        if (err != ASE_OK)
        {
            Out_ASIO_CloseDriver();
            return false;
        }

        list.push_back({.id = i, .name = g_output.channel_info[i].name});
    }

    Out_ASIO_CloseDriver();
    return true;
}

bool Out_ASIO_Create(const char* driver_name, const ASIO_OutputParameters& params)
{
    if (!Out_ASIO_OpenDriver(driver_name))
    {
        return false;
    }

    ASIOError err;

    fprintf(stderr,
            "asioVersion:   %ld\n"
            "driverVersion: %ld\n"
            "name:          %s\n"
            "errorMessage:  %s\n",
            g_output.driver_info.asioVersion,
            g_output.driver_info.driverVersion,
            g_output.driver_info.name,
            g_output.driver_info.errorMessage);

    err = ASIOGetBufferSize(&g_output.min_size, &g_output.max_size, &g_output.preferred_size, &g_output.granularity);
    if (err != ASE_OK)
    {
        fprintf(stderr, "ASIOGetBufferSize failed with %s\n", ErrorToString(err));
        ASIOExit();
        return false;
    }

    fprintf(stderr,
            "ASIO buffer info: min=%ld, max=%ld, preferred=%ld, granularity=%ld\n",
            g_output.min_size,
            g_output.max_size,
            g_output.preferred_size,
            g_output.granularity);

    fprintf(stderr, "User requested buffer size is %d\n", params.common.buffer_size);

    // ASIO4ALL can't handle the sample rate the emulator uses, so we'll need
    // to use a more common one and resample
    err = ASIOSetSampleRate((ASIOSampleRate)params.common.frequency);
    if (err != ASE_OK)
    {
        fprintf(stderr,
                "ASIOSetSampleRate(%d) failed with %s; trying to continue anyways\n",
                params.common.frequency,
                ErrorToString(err));
    }

    err = ASIOGetSampleRate(&g_output.actual_freq);
    if (err != ASE_OK)
    {
        fprintf(stderr, "ASIOGetSampleRate failed with %s\n", ErrorToString(err));
        ASIOExit();
        return false;
    }

    fprintf(stderr, "ASIO: sample rate is %d\n", (int)g_output.actual_freq);

    err = ASIOGetChannels(&g_output.input_channel_count, &g_output.output_channel_count);
    if (err != ASE_OK)
    {
        fprintf(stderr, "ASIOGetChannels failed with %s\n", ErrorToString(err));
        ASIOExit();
        return false;
    }

    fprintf(
        stderr, "Available channels: %ld in, %ld out\n", g_output.input_channel_count, g_output.output_channel_count);

    if ((size_t)g_output.output_channel_count > MAX_CHANNELS)
    {
        fprintf(stderr, "WARNING: more than %zu output channels; truncating to %zu\n", MAX_CHANNELS, MAX_CHANNELS);
        g_output.output_channel_count = MAX_CHANNELS;
    }

    for (long i = 0; i < g_output.output_channel_count; ++i)
    {
        g_output.channel_info[i].channel = i;
        g_output.channel_info[i].isInput = ASIOFalse;

        err = ASIOGetChannelInfo(&g_output.channel_info[i]);
        if (err != ASE_OK)
        {
            fprintf(stderr, "ASIOGetChannelInfo failed with %s\n", ErrorToString(err));
            ASIOExit();
            return false;
        }
    }

    if ((size_t)g_output.output_channel_count < N_BUFFERS)
    {
        fprintf(stderr, "%zu channels required; aborting\n", N_BUFFERS);
        ASIOExit();
        return false;
    }

    if (!Out_ASIO_PickOutputChannel(params.left_channel, g_output.left_channel))
    {
        fprintf(stderr, "L channel defaulting to 0\n");
        g_output.left_channel = 0;
    }

    if (!Out_ASIO_PickOutputChannel(params.right_channel, g_output.right_channel))
    {
        fprintf(stderr, "R channel defaulting to 1\n");
        g_output.right_channel = 1;
    }

    fprintf(stderr, "ASIO output channels:\n");

    for (long i = 0; i < g_output.output_channel_count; ++i)
    {
        fprintf(stderr, "  %ld: %-32s %s ", i, g_output.channel_info[i].name,
                SampleTypeToString(g_output.channel_info[i].type));

        if (i == g_output.left_channel)
        {
            fprintf(stderr, "(left)\n");
        }
        else if (i == g_output.right_channel)
        {
            fprintf(stderr, "(right)\n");
        }
        else
        {
            fprintf(stderr, "\n");
        }
    }

    if ((size_t)g_output.left_channel >= (size_t)g_output.output_channel_count)
    {
        fprintf(stderr, "Left channel out of range; aborting\n");
        ASIOExit();
        return false;
    }

    if ((size_t)g_output.right_channel >= (size_t)g_output.output_channel_count)
    {
        fprintf(stderr, "Right channel out of range; aborting\n");
        ASIOExit();
        return false;
    }

    if (g_output.left_channel == g_output.right_channel)
    {
        fprintf(stderr, "Left and right channels are both %ld; aborting\n", g_output.left_channel);
        ASIOExit();
        return false;
    }

    if (g_output.channel_info[g_output.left_channel].type != g_output.channel_info[g_output.right_channel].type)
    {
        fprintf(stderr,
                "Left and right channels %ld and %ld have different output types; aborting\n",
                g_output.left_channel,
                g_output.right_channel);
        ASIOExit();
        return false;
    }

    g_output.buffer_info[0].isInput    = ASIOFalse;
    g_output.buffer_info[0].channelNum = g_output.left_channel;
    g_output.buffer_info[0].buffers[0] = nullptr;
    g_output.buffer_info[0].buffers[1] = nullptr;

    g_output.buffer_info[1].isInput    = ASIOFalse;
    g_output.buffer_info[1].channelNum = g_output.right_channel;
    g_output.buffer_info[1].buffers[0] = nullptr;
    g_output.buffer_info[1].buffers[1] = nullptr;

    g_output.callbacks.bufferSwitch         = bufferSwitch;
    g_output.callbacks.bufferSwitchTimeInfo = bufferSwitchTimeInfo;
    g_output.callbacks.sampleRateDidChange  = sampleRateDidChange;
    g_output.callbacks.asioMessage          = asioMessage;

    g_output.buffer_size_frames = params.common.buffer_size;
    g_output.output_type        = g_output.channel_info[g_output.left_channel].type;
    g_output.buffer_size_bytes  = g_output.buffer_size_frames * Out_ASIO_GetFormatSampleSizeBytes();

    err = ASIOCreateBuffers(g_output.buffer_info, N_BUFFERS, (long)g_output.buffer_size_frames, &g_output.callbacks);
    if (err != ASE_OK)
    {
        fprintf(stderr, "ASIOCreateBuffers failed with %s\n", ErrorToString(err));
        ASIOExit();
        return false;
    }

    // *2 because an ASIO buffer only represents one channel, but our mix buffer will hold 2 channels
    const size_t mb_size = 2 * g_output.buffer_size_bytes;

    if (!g_output.mix_buffers[0].Init(mb_size) || !g_output.mix_buffers[1].Init(mb_size))
    {
        fprintf(stderr, "Failed to allocate mix buffer for ASIO output.\n");
        ASIOExit();
        return false;
    }

    g_output.create_params = params;

    return true;
}

void Out_ASIO_Destroy()
{
    ASIOStop();
    ASIODisposeBuffers();
    ASIOExit();
}

bool Out_ASIO_Start()
{
    ASIOError err = ASIOStart();
    if (err != ASE_OK)
    {
        fprintf(stderr, "ASIOStart failed with %s\n", ErrorToString(err));
        return false;
    }

    return true;
}

void Out_ASIO_AddSource(SDL_AudioStream* stream)
{
    if (g_output.stream_count == MAX_STREAMS)
    {
        fprintf(stderr, "PANIC: attempted to add more than %zu ASIO streams\n", MAX_STREAMS);
        exit(1);
    }
    g_output.streams[g_output.stream_count] = stream;
    ++g_output.stream_count;
}

int Out_ASIO_GetFrequency()
{
    return (int)g_output.actual_freq;
}

SDL_AudioFormat Out_ASIO_GetFormat()
{
    switch (g_output.output_type)
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

size_t Out_ASIO_GetFormatSampleSizeBytes()
{
    return SDL_AUDIO_BITSIZE(Out_ASIO_GetFormat()) / 8;
}

size_t Out_ASIO_GetFormatFrameSizeBytes()
{
    return 2 * Out_ASIO_GetFormatSampleSizeBytes();
}

void Out_ASIO_Stop()
{
    ASIOStop();
}

bool Out_ASIO_IsResetRequested()
{
    return g_output.defer_reset;
}

bool Out_ASIO_Reset()
{
    g_output.defer_reset = false;
    Out_ASIO_Destroy();

    if (!Out_ASIO_Create(g_output.driver_info.name, g_output.create_params))
    {
        fprintf(stderr, "ASIO reset: failed to re-initialize ASIO\n");
        return false;
    }

    if (!Out_ASIO_Start())
    {
        fprintf(stderr, "ASIO reset: failed to restart ASIO playback\n");
        return false;
    }

    return true;
}

size_t Out_ASIO_GetBufferSize()
{
    return g_output.create_params.common.buffer_size;
}

// `src` contains `count` pairs of 16-bit words LRLRLRLR (here count = 4)
// `dst_a` will receive LLLL
// `dst_b` will receive RRRR
inline void Deinterleave16(void* dst_a, void* dst_b, const void* src, size_t count)
{
    for (size_t i = 0; i < count; ++i)
    {
        memcpy((uint8_t*)dst_a + 2 * i, (uint8_t*)src + 4 * i + 0, 2);
        memcpy((uint8_t*)dst_b + 2 * i, (uint8_t*)src + 4 * i + 2, 2);
    }
}

// same as above but for 32-bit words
inline void Deinterleave32(void* dst_a, void* dst_b, const void* src, size_t count)
{
    for (size_t i = 0; i < count; ++i)
    {
        memcpy((uint8_t*)dst_a + 4 * i, (uint8_t*)src + 8 * i + 0, 4);
        memcpy((uint8_t*)dst_b + 4 * i, (uint8_t*)src + 8 * i + 4, 4);
    }
}

inline void Deinterleave(void* dst_a, void* dst_b, const void* src, size_t count, size_t word_size)
{
    switch (word_size)
    {
    case 2:
        Deinterleave16(dst_a, dst_b, src, count);
        break;
    case 4:
        Deinterleave32(dst_a, dst_b, src, count);
        break;
    default:
        fprintf(stderr, "PANIC: Deinterleave not implemented for word size %zu\n", word_size);
        exit(1);
    }
}

template <typename FrameT>
inline void MixBuffer(GenericBuffer& dst, const GenericBuffer& src)
{
    auto dst_span = std::span<FrameT>((FrameT*)dst.DataFirst(), dst.GetByteLength() / sizeof(FrameT));
    auto src_span = std::span<FrameT>((FrameT*)src.DataFirst(), src.GetByteLength() / sizeof(FrameT));
    assert(src_span.size() == dst_span.size());
    for (size_t samp = 0; samp < dst_span.size(); ++samp)
    {
        MixFrame(dst_span[samp], src_span[samp]);
    }
}

inline void MixBuffer(GenericBuffer& dst, const GenericBuffer& src, SDL_AudioFormat format)
{
    switch (format)
    {
    case AUDIO_S16SYS:
        MixBuffer<AudioFrame<int16_t>>(dst, src);
        break;
    case AUDIO_S32SYS:
        MixBuffer<AudioFrame<int32_t>>(dst, src);
        break;
    case AUDIO_F32SYS:
        MixBuffer<AudioFrame<float>>(dst, src);
        break;
    default:
        fprintf(
            stderr, "PANIC: MixBuffer called for unsupported format %s (%x)\n", SDLAudioFormatToString(format), format);
        exit(1);
    }
}

static ASIOTime* bufferSwitchTimeInfo(ASIOTime* params, long index, ASIOBool directProcess)
{
    (void)params;
    (void)directProcess;

    size_t renderable_frames = g_output.buffer_size_frames;
    for (size_t i = 0; i < g_output.stream_count; ++i)
    {
        renderable_frames =
            Min(renderable_frames, (size_t)SDL_AudioStreamAvailable(g_output.streams[i]) / sizeof(AudioFrame<int32_t>));
    }

    if (renderable_frames < g_output.buffer_size_frames || g_output.stream_count == 0)
    {
        memset(g_output.buffer_info[0].buffers[index], 0, g_output.buffer_size_bytes);
        memset(g_output.buffer_info[1].buffers[index], 0, g_output.buffer_size_bytes);
        return 0;
    }

    SDL_AudioStreamGet(
        g_output.streams[0], g_output.mix_buffers[1].DataFirst(), (int)g_output.mix_buffers[1].GetByteLength());

    for (size_t i = 1; i < g_output.stream_count; ++i)
    {
        // read from stream into staging buffer
        SDL_AudioStreamGet(
            g_output.streams[i], g_output.mix_buffers[0].DataFirst(), (int)g_output.mix_buffers[0].GetByteLength());

        // mix staging buffer into final buffer
        MixBuffer(g_output.mix_buffers[1], g_output.mix_buffers[0], Out_ASIO_GetFormat());
    }

    // unpack final buffer and send it to ASIO driver
    Deinterleave(g_output.buffer_info[0].buffers[index],
                 g_output.buffer_info[1].buffers[index],
                 g_output.mix_buffers[1].DataFirst(),
                 g_output.buffer_size_frames,
                 Out_ASIO_GetFormatSampleSizeBytes());

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
    // TODO: host needs to be notified so it can update the SDL stream to use the new frequency...
    g_output.actual_freq = sRate;
    fprintf(stderr, "ASIO: driver changed sample rate to %f - this is currently unimplemented!\n", sRate);
}

static long asioMessage(long selector, long value, void* message, double* opt)
{
    (void)message;
    (void)opt;

    switch (selector)
    {
    case kAsioSelectorSupported:
        return value == kAsioSelectorSupported || value == kAsioEngineVersion || value == kAsioResetRequest;
    case kAsioEngineVersion:
        return 2;
    case kAsioResetRequest:
        g_output.defer_reset = true;
        return 1;
    default:
        return 0;
    }
}
