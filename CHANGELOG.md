# Version 0.6.0 (TBD)

## Breaking changes

- The emulator backend no longer depends on SDL. `Emulator::Init` now accepts
  an LCD backend pointer that can be used to customize the behavior of the LCD
  on a per-application basis. The old SDL backend has been moved to the
  standard frontend.
- `Emulator::LoadRoms` has been renamed to `Emulator::LoadRomsByFilename`. A
  new API `Emulator::LoadRomsByInfo` implements hash based rom loading. See
  `emu.h` for more information.

## Enhancements

- Changed the default rom loader to locate roms by hash instead of by hardcoded
  filename. This means that roms do not need specific filenames; they only need
  to be present in the rom directory. The old behavior can be enabled by
  passing `--legacy-romset-detection` to either frontend.
- Added new command line parameters to override specific roms. These are meant
  for advanced users who have roms with unknown hashes. The parameters are:
  `--override-rom1 <path>`, `--override-rom2 <path>`, `--override-smrom
  <path>`, `--override-waverom1 <path>`, `--override-waverom2 <path>`,
  `--override-waverom3 <path>`. Each parameter takes the rom filename to load.

## General enhancements

- Added an `--nvram <filename>` parameter to both frontends. This is only used
  by the JV-880 and can be used to persist and reload JV-880 settings. (#36)
- Both frontends now send a GS reset by default when using a SC-55mk2 romset in
  order to work around a firmware bug that causes instrument pitch to be
  initialized incorrectly. Upstream defaults to not sending a reset, so this is
  a divergence in behavior. To get the old behavior, pass `--reset none`. (#38)

## Bugfixes

- Fixed crash on startup when the directory containing the executable had
  non-English characters. (#30)

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
