/*
 * Copyright (C) 2021, 2024 nukeykt
 *
 *  Redistribution and use of this code or any derivative works are permitted
 *  provided that the following conditions are met:
 *
 *   - Redistributions may not be sold, nor may they be used in a commercial
 *     product or activity.
 *
 *   - Redistributions that are modified from the original source must include the
 *     complete source code, including the source code for all components used by a
 *     binary built from the modified sources. However, as a special exception, the
 *     source code distributed need not include anything that is normally distributed
 *     (in either source or binary form) with the major components (compiler, kernel,
 *     and so on) of the operating system on which the executable runs, unless that
 *     component itself accompanies the executable.
 *
 *   - Redistributions must reproduce the above copyright notice, this list of
 *     conditions and the following disclaimer in the documentation and/or other
 *     materials provided with the distribution.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 */
#include <stdio.h>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <mmsystem.h>
#include <span>
#include <cstdint>
#include "midi.h"

static HMIDIIN midi_handle;
static MIDIHDR midi_buffer;

static uint8_t midi_in_buffer[1024];

static FE_Application* midi_frontend = nullptr;

void FE_RouteMIDI(FE_Application& fe, std::span<const uint8_t> bytes);

void CALLBACK MIDI_Callback(
    HMIDIIN   hMidiIn,
    UINT      wMsg,
    DWORD_PTR dwInstance,
    DWORD_PTR dwParam1,
    DWORD_PTR dwParam2
)
{
    (void)hMidiIn;
    (void)dwInstance;
    (void)dwParam2;

    switch (wMsg)
    {
        case MIM_OPEN:
            break;
        case MIM_DATA:
        {
            uint8_t b1 = dwParam1 & 0xff;
            switch (b1 & 0xf0)
            {
                case 0x80:
                case 0x90:
                case 0xa0:
                case 0xb0:
                case 0xe0:
                    {
                        uint8_t buf[3] = {
                            (uint8_t)b1,
                            (uint8_t)((dwParam1 >> 8) & 0xff),
                            (uint8_t)((dwParam1 >> 16) & 0xff),
                        };
                        FE_RouteMIDI(*midi_frontend, buf);
                    }
                    break;
                case 0xc0:
                case 0xd0:
                    {
                        uint8_t buf[2] = {
                            (uint8_t)b1,
                            (uint8_t)((dwParam1 >> 8) & 0xff),
                        };
                        FE_RouteMIDI(*midi_frontend, buf);
                    }
                    break;
            }
            break;
        }
        case MIM_LONGDATA:
        case MIM_LONGERROR:
        {
            MMRESULT result = midiInUnprepareHeader(midi_handle, &midi_buffer, sizeof(MIDIHDR));
            if (result == MMSYSERR_INVALHANDLE)
            {
                // If this happens, the frontend probably called MIDI_Quit and
                // midi_frontend is no longer valid. We got here because this
                // callback is running in a separate thread and might be called
                // after MIDI_Quit.
                break;
            }

            if (wMsg == MIM_LONGDATA)
            {
                FE_RouteMIDI(*midi_frontend, std::span(midi_in_buffer, midi_buffer.dwBytesRecorded));
            }

            midiInPrepareHeader(midi_handle, &midi_buffer, sizeof(MIDIHDR));
            midiInAddBuffer(midi_handle, &midi_buffer, sizeof(MIDIHDR));

            break;
        }
        default:
            fprintf(stderr, "hmm");
            break;
    }
}

bool MIDI_Init(FE_Application& frontend, unsigned int port)
{
    midi_frontend = &frontend;

    UINT num = midiInGetNumDevs();

    if (num == 0)
    {
        fprintf(stderr, "No midi input\n");
        return 0;
    }

    if (port >= num)
    {
        fprintf(stderr, "Out of range midi port is requested. Defaulting to port 0\n");
        port = 0;
    }

    MIDIINCAPSA caps;
    MMRESULT result;

    result = midiInGetDevCapsA(port, &caps, sizeof(MIDIINCAPSA));
    if (result != MMSYSERR_NOERROR)
    {
        fprintf(stderr, "midiInGetDevCapsA failed\n");
        return false;
    }

    result = midiInOpen(&midi_handle, port, (DWORD_PTR)MIDI_Callback, 0, CALLBACK_FUNCTION);
    if (result != MMSYSERR_NOERROR)
    {
        fprintf(stderr, "midiInOpen failed\n");
        return false;
    }

    fprintf(stderr, "Opened midi port: %s\n", caps.szPname);

    midi_buffer.lpData = (LPSTR)midi_in_buffer;
    midi_buffer.dwBufferLength = sizeof(midi_in_buffer);

    result = midiInPrepareHeader(midi_handle, &midi_buffer, sizeof(MIDIHDR));
    if (result != MMSYSERR_NOERROR)
    {
        fprintf(stderr, "midiInPrepareHeader failed\n");
        return false;
    }

    result = midiInAddBuffer(midi_handle, &midi_buffer, sizeof(MIDIHDR));
    if (result != MMSYSERR_NOERROR)
    {
        fprintf(stderr, "midiInAddBuffer failed\n");
        return false;
    }

    result = midiInStart(midi_handle);
    if (result != MMSYSERR_NOERROR)
    {
        fprintf(stderr, "midiInStart failed\n");
        return false;
    }

    return true;
}

void MIDI_Quit()
{
    if (midi_handle)
    {
        midiInStop(midi_handle);
        midiInClose(midi_handle);
        midi_handle = 0;
    }
    midi_frontend = nullptr;
}
