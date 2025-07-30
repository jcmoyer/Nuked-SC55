# Renderer Frontend

The renderer frontend is a command line tool for rendering midi files to wave
files. This is separate from the GUI program packaged by [upstream
Nuked-SC55](https://github.com/nukeykt/Nuked-SC55).

## Command line options

### `-v, --version`

Prints the version number and build configuration to stdout then exits
immediately.

### `-o <filename>`

Writes a wave file to `filename`. Cannot be combined with `--stdout`.

### `--stdout`

Writes the raw sample data to stdout. This is mostly used for testing the
emulator.

### `-f, --format s16|s32|f32`

Sets the output format.

- `s16`: signed 16-bit audio
- `s32`: signed 32-bit audio
- `f32`: 32-bit floating-point audio

### `--disable-oversampling`

Halves output frequency of the emulator. This trades audio quality for space.

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

### `--end cut|release`

Choose how the end of the track is handled:

- `cut` (default): stop rendering at the last MIDI event
- `release`: continue to render audio after the last MIDI event until silence.

### `-r, --reset none|gs|gm`

Sends a reset message to the emulator on startup.

This will default to `gs` when using a SC-55mk2 romset in order to work around
a firmware issue that causes incorrect instrument pitch. For all other romsets,
this defaults to `none`.

### `-n, --instances <count>`

Create `count` instances of the emulator. MIDI events will be routed to
emulator N where N is the MIDI event channel mod `count`. Use this to increase
effective polyphony. A `count` of 2 is enough to play most MIDIs without
dropping notes.

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

If provided, this will set the romset to load. Otherwise, the renderer will
autodetect the romset based on what filenames it finds in the rom directory.

Run `nuked-sc55-render --help` to see a list of accepted romset names.

### `--dump-emidi-loop-points`

If provided, the renderer will print a reference frequency and all EMIDI loop
points to **stderr** when finished.

This flag enables handling of the following events:

- CC 116 (track loop start)
- CC 117 (track loop end)
- CC 118 (global loop start)
- CC 119 (global loop end)

The messages are formatted as follows:

```
rate=66207
track 1 loop start at sample=46192 timestamp=00:00.69
track 1 loop end at sample=4110616 timestamp=01:02.08
...
track 12 loop start at sample=46192 timestamp=00:00.69
track 12 loop end at sample=4110622 timestamp=01:02.08
...
global loop start at sample=0 timestamp=00:00.00
global loop end at sample=1324138 timestamp=00:19.99
```

There can be as many messages as MIDI *tracks* (as opposed to MIDI channels) in
the file. If multiple nested loops are present in a single track, each one will
be printed in the order it was encountered.

It is normal for loop points to differ slightly when rendering with multiple
instances - each instance will be slightly out of sync with the others due to
small timing differences. In this case, any of the sample or timestamp values
are acceptable.

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
