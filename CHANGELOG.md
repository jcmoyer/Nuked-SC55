# Version 0.7.0 (TBD)

- Optimized interrupt handling for a 10-16% overall performance improvement
  depending on compiler.
- Fixed a bug where selecting a specific romset using `--romset` would cause
  the emulator to not load all the roms in that romset.
- Fixed a bug where the renderer output was ~20% slower than the set tempo when
  using a JV880 romset.
- Fixed a subtle timing bug that caused sample output for the JV880 to differ
  slightly from upstream.
- Updated hashes for several romsets thanks to @zilch510. (#56)
- Fixed a bug where `--disable-oversampling` with an mk1 romset would cause the
  renderer to run indefinitely. (#48)

# Version 0.6.1 (2025-07-30)

## Enhancements

- Added a `--gain <amount>` option to both frontends. This can be used to
  increase or decrease the output volume. `<amount>` can be specified in either
  decibels or as a scalar value. See documentation for details. (#46)
- Added basic support for dumping EMIDI loop points. Currently, this includes
  the following control changes:

  - CC 116 (track loop start)
  - CC 117 (track loop end)
  - CC 118 (global loop start)
  - CC 119 (global loop end)

  Pass `--dump-emidi-loop-points` to the renderer to enable this feature. (#47)
- Completed the SCC-1A romset hashes thanks to @Karmeck. (#49)
- Added hashes for mk2 roms with a CTF patch applied from
  [shingo45endo/sc55mk2-ctf-patcher](https://github.com/shingo45endo/sc55mk2-ctf-patcher).
  (#51)
- Improved error reporting when a romset is detected, but it is only partially
  complete. (#51)

## Bugfixes

- Renderer `--end release` option now properly accounts for the MK1's DC
  offset. (#48)

# Version 0.6.0 (2025-06-09)

This release contains bugfixes and a couple quality of life enhancements.

## Enhancements

- Both frontends now default to locating roms by hashing files in the rom
  directory instead of requiring specific filenames. The old behavior can be
  enabled by passing `--legacy-romset-detection` to either program.
- Added more informative messages for loaded and missing roms.
- Added new command line parameters to override specific roms. These are meant
  for advanced users who have roms with unknown hashes. The parameters are:
  `--override-rom1 <path>`, `--override-rom2 <path>`, `--override-smrom
  <path>`, `--override-waverom1 <path>`, `--override-waverom2 <path>`,
  `--override-waverom3 <path>`, `--override-waverom-card <path>`,
  `--override-waverom-exp <path>`. Each parameter takes the rom filename to
  load.
- Added an `--nvram <filename>` parameter to both frontends. This is only used
  by the JV-880 and can be used to persist and reload JV-880 settings. (#36)
- Both frontends now send a GS reset by default when using a SC-55mk2 romset in
  order to work around a firmware bug that causes instrument pitch to be
  initialized incorrectly. Upstream defaults to not sending a reset, so this is
  a divergence in behavior. To get the old behavior, pass `--reset none`. (#38)
- Added the ability to route emulator audio to specific ASIO channels. (#39)

## Bugfixes

- Fixed crash on startup when the directory containing the executable had
  non-English characters. (#30)
- Fixed renderer desync when using multiple instances and one of the instances
  received midi data starting later than tick 0. (#42)

## Breaking changes for developers

- The emulator backend no longer depends on SDL. `Emulator::Init` now accepts
  an LCD backend pointer that can be used to customize the behavior of the LCD
  on a per-application basis. The old SDL backend has been moved to the
  standard frontend.
- `Emulator::LoadRoms` now requires rom data to be loaded into an instance of
  `AllRomsetInfo` by the caller. Several functions have been added to help with
  this. See `emu.h` for more information. A complete example can be found under
  `common/rom_loader.h` and its accompanying source file.

# Version 0.5.0 (2025-03-21)

The main addition in this release is ASIO support for Windows. For legal
reasons, the emulator must be built from source to enable it. See
[BUILDING.md](BUILDING.md) for instructions.

## General enhancements

- Added documentation for both [standard](doc/standard_frontend.md) and
  [renderer](doc/renderer_frontend.md) frontends.
- Added `-v, --version` command line to both frontends to print the binary
  version and build configuration.

## Standard frontend

- Added ASIO support for Windows users who compile from source. See
  [BUILDING.md](BUILDING.md) for instructions. Precompiled ASIO builds cannot
  be provided because of the ASIO SDK license.
- The `-b, --buffer-size <page_size>[:page_count]` flag has been **changed** so
  that it now precisely controls buffer sizes across the program. This is a
  divergence from upstream. The new default values closely match the behavior
  of upstream's default values. The exact differences are documented
  [here](doc/standard_frontend.md#-b---buffer-size-sizecount).
- The program will now always respond to user input even if the emulator starts
  falling behind. This is mostly aimed at developers with the goal of making
  the program easier to terminate when optimizations are disabled for debugging
  purposes.
- Fixed some threading soundness issues inherited from upstream that could
  theoretically cause playback issues. This is more likely to affect platforms
  with weaker memory models than x86. See [upstream
  #72](https://github.com/nukeykt/Nuked-SC55/pull/72) for some discussion about
  this.
- `back.data` is now embedded into the executable and no longer needs to be
  located in a specific directory. (#7, #12, #25)

## Renderer

- The emulator will now run for a bit before receiving any MIDI events even if
  you don't specify a reset with `-r` or `--reset`. This will ensure it is
  ready to accept program changes. (#20)
- The renderer will no longer ask the user to submit a bug report when the
  instance count is set to a number that causes only some instances to have
  MIDI events to render. (#19, #24)

# Version 0.4.2 (2025-02-23)

## Enhancements

- The WAV renderer now accepts MIDI files with tracks that contain extra data
  past their end-of-track event. Extra data is ignored. (#18)

# Version 0.4.1 (2025-02-17)

## Enhancements

- The WAV renderer now prints the list of accepted `--romset` names if you pass
  `--help` or provide an invalid one.

## Bugfixes

- The WAV renderer now writes correct headers when choosing the `f32` format.
  Previously, the WAV files would appear to have a long period of silence at
  the end in some audio players. (#17)

# Version 0.4.0 (2025-02-11)

Initial release
