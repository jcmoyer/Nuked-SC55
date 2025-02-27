# Standard Frontend

The standard frontend is an enhanced version of the program packaged by
[upstream Nuked-SC55](https://github.com/nukeykt/Nuked-SC55). The major
differences are:

1. Better performance
2. Ability to run multiple emulators to increase effective polyphony
3. Various QOL enhancements
   - Set rom directory
   - List audio and midi devices and pick them by index or name
   - Launch without a window
4. More audio output formats

# Command line options

## `-p, --port <device_name_or_number>`

Sets the MIDI input port.

## `-a, --audio-device <device_name_or_number>`

Sets the audio device to play sound from.

## `-b, --buffer-size <size>[:count]`

Controls the amount of audio to produce or output at a time. Lower values
reduce latency; higher values reduce playback glitches. Optimal settings are
hardware dependent, so experiment with this option. If no value is provided, it
will default to `512:16` roughly mirroring upstream's intent.

`size` is the number of audio frames that the emulator will produce and the
output will consume in a single chunk. It must be a power of 2.

`count` is the number of `size` pages that can be queued up. It can be any
value greater than zero, but the best value is likely in the range `2..32`.

Having queued chunks is helpful if the emulator produces audio fast enough most
of the time but sometimes falls behind. This can happen for music containing
particularly busy sections. The queued chunks give the emulator some headroom
so that even if it slows down temporarily, the output has enough audio to work
with until the emulator catches back up.

### Example

Consider `-b 512:16`: The SC-55mk2 outputs at 66207hz. The emulator would
produce chunks of 512/66207 = 7.7ms of audio at a time. It would be allowed to
queue up to 16 of those chunks, meaning that you could have up to 512\*16/66207
= 123ms of latency.

### Divergence from upstream

The behavior of this option was changed because the way upstream uses it is
buggy and unintuitive.

Upstream defaults to `-b 512:32`. For this setting, it creates a ringbuffer of
512\*32 *samples*, but the buffer only holds 512\*16 *frames* (each frame
consists of two samples, one for each stereo channel). The emulator will place
one frame at a time into this buffer. Audio will be drained from the buffer for
playback 512/4 = 128 frames at a time. The /4 is arbitrary and cannot be set by
the user.

In theory, the buffer should be able to contain ~123ms of audio, but because
upstream reads from the buffer unconditionally, the read position can overtake
the write position causing dropped frames. This also causes a variable amount
of latency, because at the start of the program the read position will start
advancing - it then takes 123ms to get back to the start of the buffer where
audio has started being written.

This fork corrects the ringbuffer behavior - instead of advancing the read
position unconditionally, it only advances when there is audio to play and
produces silence otherwise. The buffer size you provide to this option is in
*frames* instead of samples, and the numbers you provide are used *without
modification* by both the emulator and output.

### ASIO

ASIO drivers have a preferred buffer size that may be adjusted separately. The
value you provide here does not necessarily need to match that size, but it's
probably a good idea to make them equal.

## `-f, --format s16|s32|f32`

Sets the output format. Some formats may not be available on all hardware.

### ASIO

ASIO drivers will request a specific output format. In this case, `-f` will
only control the internal audio format, and it will be converted to the format
the ASIO driver requests when handed off for output.

## `--disable-oversampling`

Disables oversampling, halving output frequency. Normally the emulator produces
two frames at a time. When you set this option, the second one will be dropped.

## `-r, --reset gs|gm`

Sends a reset message to the emulator on startup. This is necessary to correct
pitch with some roms.

## `-n, --instances <count>`

Create `count` instances of the emulator. MIDI events will be routed to
emulator N where N is the MIDI event channel mod `count`. Use this to increase
effective polyphony. A `count` of 2 is enough to play most MIDIs without
dropping notes.

## `--no-lcd`

Don't create an LCD window. This is useful if you're using the emulator with
DOOM or a DAW and don't want to spend resources rendering it. When this option
is set you will not be able to control the emulator with your keyboard.

## `-d, --rom-directory <dir>`

Sets the directory to load roms from. If no specific romset flag is passed, the
emulator will pick one based on the filenames in `<dir>`. If this is not set,
the emulator will look for roms in these locations:

1. `<exe_dir>/../share/nuked-sc55`
2. `<exe_dir>`

`<exe_dir>` is the directory containing this executable.

## `--mk2`

Use SC-55mk2 ROM set. This overrides autodetect.

## `--st`

Use SC-55st ROM set. This overrides autodetect.

## `--mk1`

Use SC-55mk1 ROM set. This overrides autodetect.

## `--cm300`

Use CM-300/SCC-1 ROM set. This overrides autodetect.

## `--jv880` 

Use JV-880 ROM set. This overrides autodetect.

## `--scb55`

Use SCB-55 ROM set. This overrides autodetect.

## `--rlp3237`

Use RLP-3237 ROM set. This overrides autodetect.
