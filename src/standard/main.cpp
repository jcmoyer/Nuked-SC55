/*
 * Copyright (C) 2024-2026 J.C. Moyer
 *
 * This file is part of Nuked-SC55.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#include "application.h"

#include "common/path_util.h"

#ifdef _WIN32
#include <Windows.h>
#endif

#ifdef _WIN32
// On Windows we install a Ctrl-C handler to make sure that the event loop always receives an SDL_QUIT event. This
// is what normally happens on other platforms but only some Windows environments (for instance, a mingw64 shell).
// If the program is run from cmd or Windows explorer, SDL_QUIT is never sent and the program hangs.
BOOL WINAPI CtrlCHandler(DWORD dwCtrlType)
{
    (void)dwCtrlType;
    SDL_Event quit_event{};
    quit_event.type = SDL_QUIT;
    SDL_PushEvent(&quit_event);
    return TRUE;
}
#endif

bool GlobalInit()
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0)
    {
        fprintf(stderr, "FATAL ERROR: Failed to initialize SDL: %s.\n", SDL_GetError());
        fflush(stderr);
        return false;
    }

#ifdef _WIN32
    SetConsoleCtrlHandler(CtrlCHandler, TRUE);
#endif

    return true;
}

void GlobalQuit()
{
    SDL_Quit();
}

void PrintUsage()
{
    constexpr const char* USAGE_STR = R"(Usage: %s [options]

General options:
  -?, -h, --help                                Display this information.
  -v, --version                                 Display version information.

Audio options:
  -p, --port         <device_name_or_number>    Set MIDI input port.
  -a, --audio-device <device_name_or_number>    Set output audio device.
  -b, --buffer-size  <size>[:count]             Set buffer size, number of buffers.
  -f, --format       s16|s32|f32                Set output format.
  --disable-oversampling                        Halves output frequency.
  --gain <amount>                               Apply gain to the output.

Emulator options:
  -r, --reset     none|gs|gm                    Reset system in GS or GM mode.
  -n, --instances <count>                       Set number of emulator instances.
  --no-lcd                                      Run without LCDs.
  --nvram <filename>                            Saves and loads NVRAM to/from disk. JV-880 only.

ROM management options:
  -d, --rom-directory <dir>                     Sets the directory to load roms from.
  --romset <name>                               Sets the romset to load.
  --legacy-romset-detection                     Load roms using specific filenames like upstream.

)";

#if NUKED_ENABLE_ASIO
    constexpr const char* EXTRA_ASIO_STR = R"(ASIO options:
  --asio-sample-rate <freq>                     Request frequency from the ASIO driver.
  --asio-left-channel <channel_name_or_number>  Set left channel for ASIO output.
  --asio-right-channel <channel_name_or_number> Set right channel for ASIO output.

)";
#endif

    std::string name = common::GetProcessPath().stem().generic_string();
    fprintf(stderr, USAGE_STR, name.c_str());
    common::PrintRomsets(stderr);
#if NUKED_ENABLE_ASIO
    fprintf(stderr, EXTRA_ASIO_STR);
#endif
    MIDI_PrintDevices();
    PrintAudioDevices(stderr);
}

int main(int argc, char* argv[])
{
    CliParameters params;
    CliParseError result = ParseCommandLine(argc, argv, params);
    if (result != CliParseError::Success)
    {
        fprintf(stderr, "error: %s\n", ParseErrorStr(result));
        PrintUsage();
        return 1;
    }

    if (params.help)
    {
        PrintUsage();
        return 0;
    }

    if (params.version)
    {
        // we'll explicitly use stdout for this - often tools want to parse
        // version information and we want to be able to support that use case
        // without requiring stream redirection
        Cfg_WriteVersionInfo(stdout);
        return 0;
    }

    FixupParameters(params);

    if (!GlobalInit())
    {
        fprintf(stderr, "FATAL ERROR: Failed to initialize global state\n");
        return 1;
    }

    // It is important that the application gets its own scope so it can be
    // destroyed before SDL is deinitialized. This is required for destructors
    // to be able to clean up SDL objects safely.
    {
        Application app;

        if (!app.Initialize(params))
        {
            fprintf(stderr, "FATAL ERROR: Failed to initialize application\n");
            return 1;
        }

        app.Run();
    }

    GlobalQuit();

    return 0;
}
