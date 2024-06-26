#include <stdio.h>
#include "mcu.h"
#include "emu.h"
#include "midi.h"
#include "cast.h"
#include <RtMidi.h>

static RtMidiIn *s_midi_in = nullptr;

static FE_Application* midi_frontend = nullptr;

void FE_RouteMIDI(FE_Application& fe, std::span<const uint8_t> bytes);

static void MidiOnReceive(double, std::vector<uint8_t> *message, void *)
{
    FE_RouteMIDI(*midi_frontend, *message);
}

static void MidiOnError(RtMidiError::Type, const std::string &errorText, void *)
{
    fprintf(stderr, "RtMidi: Error has occured: %s\n", errorText.c_str());
    fflush(stderr);
}

bool MIDI_Init(FE_Application& frontend, unsigned int port)
{
    if (s_midi_in)
    {
        fprintf(stderr, "MIDI already running\n");
        return false; // Already running
    }

    midi_frontend = &frontend;

    s_midi_in = new RtMidiIn(RtMidi::UNSPECIFIED, "Nuked SC55", 1024);
    s_midi_in->ignoreTypes(false, false, false); // SysEx disabled by default
    s_midi_in->setCallback(&MidiOnReceive, nullptr); // FIXME: (local bug) Fix the linking error
    s_midi_in->setErrorCallback(&MidiOnError, nullptr);

    unsigned int count = s_midi_in->getPortCount();

    if (count == 0)
    {
        fprintf(stderr, "No midi input\n");
        delete s_midi_in;
        s_midi_in = nullptr;
        return false;
    }

    if (port >= count)
    {
        fprintf(stderr, "Out of range midi port is requested. Defaulting to port 0\n");
        port = 0;
    }

    s_midi_in->openPort(port, "Nuked SC55");

    return true;
}

void MIDI_Quit()
{
    if (s_midi_in)
    {
        s_midi_in->closePort();
        delete s_midi_in;
        s_midi_in = nullptr;
        midi_frontend = nullptr;
    }
}
