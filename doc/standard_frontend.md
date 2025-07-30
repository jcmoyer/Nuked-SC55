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
5. (Windows, requires building from source) ASIO output for lower latency

## Command line options

### `-v, --version`

Prints the version number and build configuration to stdout then exits
immediately.

### `-p, --port <device_name_or_number>`

Sets the MIDI input port.

### `-a, --audio-device <device_name_or_number>`

Sets the audio device to play sound from.

### `-b, --buffer-size <size>[:count]`

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

#### Example

Consider `-b 512:16`: The SC-55mk2 outputs at 66207hz. The emulator would
produce chunks of 512/66207 = 7.7ms of audio at a time. It would be allowed to
queue up to 16 of those chunks, meaning that you could have up to 512\*16/66207
= 123ms of latency.

#### Divergence from upstream

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

#### ASIO

ASIO drivers have a preferred buffer size that may be adjusted separately. The
value you provide here does not necessarily need to match that size, but it's
probably a good idea to make them equal.

### `-f, --format s16|s32|f32`

Sets the output format. Some formats may not be available on all hardware.

#### ASIO

ASIO drivers will request a specific output format. In this case, `-f` will
only control the internal audio format, and it will be converted to the format
the ASIO driver requests when handed off for output.

### `--disable-oversampling`

Disables oversampling, halving output frequency. Normally the emulator produces
two frames at a time. When you set this option, the second one will be dropped.

### `--gain <amount>`

Applies gain to the output. This can be used to increase or decrease the
volume. If `<amount>` ends in `db` the preceding number will be interpreted as
decibels.

Examples:

- `--gain 2`: double volume
- `--gain 0.5`: half volume
- `--gain +6db`: double volume
- `--gain -6db`: half volume

The exact formula used for decibel to scalar conversion is `scale = pow(10, db / 20)`

### `-r, --reset none|gs|gm`

Sends a reset message to the emulator on startup.

This will default to `gs` when using a SC-55mk2 romset in order to work around
a firmware issue that causes incorrect instrument pitch. For all other romsets,
this defaults to `none`.

This behavior differs from upstream which always defaults to `none`.

### `-n, --instances <count>`

Create `count` instances of the emulator. MIDI events will be routed to
emulator N where N is the MIDI event channel mod `count`. Use this to increase
effective polyphony. A `count` of 2 is enough to play most MIDIs without
dropping notes.

### `--no-lcd`

Don't create an LCD window. This is useful if you're using the emulator with
DOOM or a DAW and don't want to spend resources rendering it. When this option
is set you will not be able to control the emulator with your keyboard.

### `--nvram <filename>`

Saves and loads NVRAM to/from disk. JV-880 only. An instance number will be
appended to the filename so that when running multiple instances they do not
clobber each other's NVRAM.

### `-d, --rom-directory <dir>`

Sets the directory to load roms from. If no specific romset flag is passed, the
emulator will pick one based on the filenames in `<dir>`. If this is not set,
the emulator will look for roms in these locations:

1. `<exe_dir>/../share/nuked-sc55`
2. `<exe_dir>`

`<exe_dir>` is the directory containing this executable.

### `--romset <name>`

If provided, this will set the romset to load. Otherwise, the romset will be
autodetected based on the contents of the rom directory.

By default, files in the rom directory will be hashed to determine which ones
should be loaded automatically. This differs from upstream which requires each
romset to have specific filenames. To enable the old behavior, pass
`--legacy-romset-detection`.

### `--legacy-romset-detection`

Behave like upstream when choosing files to load. With this option, you must
rename roms in the rom directory as follows:

```
SC-55mk2/SC-155mk2 (v1.01):
R15199858 (H8/532 mcu) -> rom1.bin
R00233567 (H8/532 extra code) -> rom2.bin
R15199880 (M37450M2 mcu) -> rom_sm.bin
R15209359 (WAVE 16M) -> waverom1.bin
R15279813 (WAVE 8M) -> waverom2.bin

SC-55st (v1.01):
R15199858 (H8/532 mcu) -> rom1.bin
R00561413 (H8/532 extra code) -> rom2_st.bin
R15199880 (M37450M2 mcu) -> rom_sm.bin
R15209359 (WAVE 16M) -> waverom1.bin
R15279813 (WAVE 8M) -> waverom2.bin

SC-55 (v1.00):
R15199748 (H8/532 mcu) -> sc55_rom1.bin
R1544925800 (H8/532 extra code) -> sc55_rom2.bin
R15209276 (WAVE A) -> sc55_waverom1.bin
R15209277 (WAVE B) -> sc55_waverom2.bin
R15209281 (WAVE C) -> sc55_waverom3.bin

SC-55 (v1.21):
R15199778 (H8/532 mcu) -> sc55_rom1.bin
R15209363 (H8/532 extra code) -> sc55_rom2.bin
R15209276 (WAVE A) -> sc55_waverom1.bin
R15209277 (WAVE B) -> sc55_waverom2.bin
R15209281 (WAVE C) -> sc55_waverom3.bin

SC-55 (v2.0):
R15199799 (H8/532 mcu) -> sc55_rom1.bin
R15209387 (H8/532 extra code) -> sc55_rom2.bin
R15209276 (WAVE A) -> sc55_waverom1.bin
R15209277 (WAVE B) -> sc55_waverom2.bin
R15209281 (WAVE C) -> sc55_waverom3.bin

CM-300/SCC-1 (v1.10):
R15199774 (H8/532 mcu) -> cm300_rom1.bin
R15279809 (H8/532 extra code) -> cm300_rom2.bin
R15279806 (WAVE A) -> cm300_waverom1.bin
R15279807 (WAVE B) -> cm300_waverom2.bin
R15279808 (WAVE C) -> cm300_waverom3.bin

CM-300/SCC-1 (v1.20):
R15199774 (H8/532 mcu) -> cm300_rom1.bin
R15279812 (H8/532 extra code) -> cm300_rom2.bin
R15279806 (WAVE A) -> cm300_waverom1.bin
R15279807 (WAVE B) -> cm300_waverom2.bin
R15279808 (WAVE C) -> cm300_waverom3.bin

SCC-1A:
R00128523 (H8/532 mcu) -> cm300_rom1.bin
R00128567 (H8/532 extra code) -> cm300_rom2.bin
R15279806 (WAVE A) -> cm300_waverom1.bin
R15279807 (WAVE B) -> cm300_waverom2.bin
R15279808 (WAVE C) -> cm300_waverom3.bin

JV-880 (v1.0.0):
R15199810 (H8/532 mcu) -> jv880_rom1.bin
R15209386 (H8/532 extra code) -> jv880_rom2.bin
R15209312 (WAVE A) -> jv880_waverom1.bin
R15209313 (WAVE B) -> jv880_waverom2.bin
PCM Cards -> jv880_waverom_pcmcard.bin (optional)
Expansion PCBs -> jv880_waverom_expansion.bin (optional)

SCB-55/RLP-3194:
R15199827 (H8/532 mcu) -> scb55_rom1.bin
R15279828 (H8/532 extra code) -> scb55_rom2.bin
R15209359 (WAVE 16M) -> scb55_waverom1.bin
R15279813 (WAVE 8M) -> scb55_waverom2.bin

RLP-3237:
R15199827 (H8/532 mcu) -> rlp3237_rom1.bin
R15209486 (H8/532 extra code) -> rlp3237_rom2.bin
R15279824 (WAVE 16M) -> rlp3237_waverom1.bin

SC-155 (rev 1):
R15199799 (H8/532 mcu) -> sc155_rom1.bin
R15209361 (H8/532 extra code) -> sc155_rom2.bin
R15209276 (WAVE A) -> sc155_waverom1.bin
R15209277 (WAVE B) -> sc155_waverom2.bin
R15209281 (WAVE C) -> sc155_waverom3.bin

SC-155 (rev 2):
R15199799 (H8/532 mcu) -> sc155_rom1.bin
R15209400 (H8/532 extra code) -> sc155_rom2.bin
R15209276 (WAVE A) -> sc155_waverom1.bin
R15209277 (WAVE B) -> sc155_waverom2.bin
R15209281 (WAVE C) -> sc155_waverom3.bin
```

## ASIO specific parameters

The following options are only enabled in ASIO builds.

### `--asio-sample-rate <rate>`

Requests the ASIO driver to use `<rate>` frequency. If no value is provided,
nuked-sc55 will request the native frequency for your selected romset.

The emulator natively produces audio at 64000hz or 66207hz depending on the
romset. Some ASIO drivers cannot support these frequencies so resampling to
`<rate>` is necessary.

### `--asio-left-channel <channel_name_or_number>`

Routes audio from the emulator's left channel to ASIO channel
`<channel_name_or_number>`.

### `--asio-right-channel <channel_name_or_number>`

Routes audio from the emulator's right channel to ASIO channel
`<channel_name_or_number>`.

## Advanced parameters

### `--override-* <path>`

Overrides the path for a specific rom. This bypasses the default methods of
locating roms.

Each romset consists of multiple roms that are individually loaded into
different locations within the emulator. These rom locations are named:

- `rom1`
- `rom2`
- `smrom`
- `waverom1`
- `waverom2`
- `waverom3`
- `waverom-card`
- `waverom-exp`

A romset does not necessarily use all of these rom locations. For example, the
mk2 will only use `rom1`, `rom2`, `smrom`, `waverom1`, and `waverom2`.

To override a specific rom path you can replace the `*` in `--override-*
<path>` with the name of the rom location you would like to load instead, e.g.
`--override-rom2 ctf-patched-rom2.bin`. This is useful in case you have a
patched rom that the emulator does not recognize.
