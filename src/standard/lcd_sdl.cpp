/*
 * Copyright (C) 2021, 2024 nukeykt
 * Modified by J.C. Moyer
 * Original source file: src/lcd.cpp
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

#include "lcd_sdl.h"
#include "emu.h"
#include "mcu.h"
#include <string>

const int button_map_sc55[][2] = {
    {SDL_SCANCODE_Q, MCU_BUTTON_POWER},
    {SDL_SCANCODE_W, MCU_BUTTON_INST_ALL},
    {SDL_SCANCODE_E, MCU_BUTTON_INST_MUTE},
    {SDL_SCANCODE_R, MCU_BUTTON_PART_L},
    {SDL_SCANCODE_T, MCU_BUTTON_PART_R},
    {SDL_SCANCODE_Y, MCU_BUTTON_INST_L},
    {SDL_SCANCODE_U, MCU_BUTTON_INST_R},
    {SDL_SCANCODE_I, MCU_BUTTON_KEY_SHIFT_L},
    {SDL_SCANCODE_O, MCU_BUTTON_KEY_SHIFT_R},
    {SDL_SCANCODE_P, MCU_BUTTON_LEVEL_L},
    {SDL_SCANCODE_LEFTBRACKET, MCU_BUTTON_LEVEL_R},
    {SDL_SCANCODE_A, MCU_BUTTON_MIDI_CH_L},
    {SDL_SCANCODE_S, MCU_BUTTON_MIDI_CH_R},
    {SDL_SCANCODE_D, MCU_BUTTON_PAN_L},
    {SDL_SCANCODE_F, MCU_BUTTON_PAN_R},
    {SDL_SCANCODE_G, MCU_BUTTON_REVERB_L},
    {SDL_SCANCODE_H, MCU_BUTTON_REVERB_R},
    {SDL_SCANCODE_J, MCU_BUTTON_CHORUS_L},
    {SDL_SCANCODE_K, MCU_BUTTON_CHORUS_R},
    {SDL_SCANCODE_LEFT, MCU_BUTTON_PART_L},
    {SDL_SCANCODE_RIGHT, MCU_BUTTON_PART_R},
};

const int button_map_jv880[][2] = {
    {SDL_SCANCODE_P, MCU_BUTTON_PREVIEW},
    {SDL_SCANCODE_LEFT, MCU_BUTTON_CURSOR_L},
    {SDL_SCANCODE_RIGHT, MCU_BUTTON_CURSOR_R},
    {SDL_SCANCODE_TAB, MCU_BUTTON_DATA},
    {SDL_SCANCODE_Q, MCU_BUTTON_TONE_SELECT},
    {SDL_SCANCODE_A, MCU_BUTTON_PATCH_PERFORM},
    {SDL_SCANCODE_W, MCU_BUTTON_EDIT},
    {SDL_SCANCODE_E, MCU_BUTTON_SYSTEM},
    {SDL_SCANCODE_R, MCU_BUTTON_RHYTHM},
    {SDL_SCANCODE_T, MCU_BUTTON_UTILITY},
    {SDL_SCANCODE_S, MCU_BUTTON_MUTE},
    {SDL_SCANCODE_D, MCU_BUTTON_MONITOR},
    {SDL_SCANCODE_F, MCU_BUTTON_COMPARE},
    {SDL_SCANCODE_G, MCU_BUTTON_ENTER},
};

LCD_SDL_Backend::~LCD_SDL_Backend()
{
    Stop();
}

bool LCD_SDL_Backend::Start(const lcd_t& lcd)
{
    m_lcd = &lcd;

    std::string title = "Nuked SC-55: ";

    title += RomsetName(m_lcd->mcu->romset);

    m_window = SDL_CreateWindow(title.c_str(),
                                SDL_WINDOWPOS_UNDEFINED,
                                SDL_WINDOWPOS_UNDEFINED,
                                (int)m_lcd->width,
                                (int)m_lcd->height,
                                SDL_WINDOW_SHOWN);
    if (!m_window)
        return false;

    m_renderer = SDL_CreateRenderer(m_window, -1, 0);
    if (!m_renderer)
        return false;

    m_texture = SDL_CreateTexture(
        m_renderer, SDL_PIXELFORMAT_BGR888, SDL_TEXTUREACCESS_STREAMING, (int)m_lcd->width, (int)m_lcd->height);
    if (!m_texture)
        return false;

    return true;
}

void LCD_SDL_Backend::Stop()
{
    if (m_texture)
    {
        SDL_DestroyTexture(m_texture);
        m_texture = nullptr;
    }

    if (m_renderer)
    {
        SDL_DestroyRenderer(m_renderer);
        m_renderer = nullptr;
    }

    if (m_window)
    {
        SDL_DestroyWindow(m_window);
        m_window = nullptr;
    }
}

void LCD_SDL_Backend::HandleEvent(const SDL_Event& sdl_event)
{
    switch (sdl_event.type)
    {
    case SDL_KEYDOWN:
    case SDL_KEYUP:
        if (sdl_event.key.windowID != SDL_GetWindowID(m_window))
        {
            return;
        }
        break;
    case SDL_WINDOWEVENT:
        if (sdl_event.window.windowID != SDL_GetWindowID(m_window))
        {
            return;
        }
        break;
    default:
        break;
    }

    if (sdl_event.type == SDL_KEYDOWN)
    {
        if (sdl_event.key.keysym.scancode == SDL_SCANCODE_COMMA)
            MCU_EncoderTrigger(*m_lcd->mcu, 0);
        if (sdl_event.key.keysym.scancode == SDL_SCANCODE_PERIOD)
            MCU_EncoderTrigger(*m_lcd->mcu, 1);
    }

    switch (sdl_event.type)
    {
    case SDL_WINDOWEVENT:
        if (sdl_event.window.event == SDL_WINDOWEVENT_CLOSE)
        {
            m_quit_requested = true;
        }
        break;

    case SDL_KEYDOWN:
    case SDL_KEYUP: {
        if (sdl_event.key.repeat)
            break;

        uint32_t mask           = 0;
        uint32_t button_pressed = m_lcd->mcu->button_pressed;

        auto button_map = m_lcd->mcu->is_jv880 ? button_map_jv880 : button_map_sc55;
        auto button_size =
            (m_lcd->mcu->is_jv880 ? sizeof(button_map_jv880) : sizeof(button_map_sc55)) / sizeof(button_map_sc55[0]);
        for (size_t i = 0; i < button_size; i++)
        {
            if (button_map[i][0] == sdl_event.key.keysym.scancode)
                mask |= (1 << button_map[i][1]);
        }

        if (sdl_event.type == SDL_KEYDOWN)
            button_pressed |= mask;
        else
            button_pressed &= ~mask;

        m_lcd->mcu->button_pressed = button_pressed;

#if 0
                if (sdl_event.key.keysym.scancode >= SDL_SCANCODE_1 && sdl_event.key.keysym.scancode < SDL_SCANCODE_0)
                {
#if 0
                    int kk = sdl_event.key.keysym.scancode - SDL_SCANCODE_1;
                    if (sdl_event.type == SDL_KEYDOWN)
                    {
                        MCU_PostUART(0xc0);
                        MCU_PostUART(118);
                        MCU_PostUART(0x90);
                        MCU_PostUART(0x30 + kk);
                        MCU_PostUART(0x7f);
                    }
                    else
                    {
                        MCU_PostUART(0x90);
                        MCU_PostUART(0x30 + kk);
                        MCU_PostUART(0);
                    }
#endif
                    int kk = sdl_event.key.keysym.scancode - SDL_SCANCODE_1;
                    const int patch = 47;
                    if (sdl_event.type == SDL_KEYDOWN)
                    {
                        static int bend = 0x2000;
                        if (kk == 4)
                        {
                            MCU_PostUART(0x99);
                            MCU_PostUART(0x32);
                            MCU_PostUART(0x7f);
                        }
                        else if (kk == 3)
                        {
                            bend += 0x100;
                            if (bend > 0x3fff)
                                bend = 0x3fff;
                            MCU_PostUART(0xe1);
                            MCU_PostUART(bend & 127);
                            MCU_PostUART((bend >> 7) & 127);
                        }
                        else if (kk == 2)
                        {
                            bend -= 0x100;
                            if (bend < 0)
                                bend = 0;
                            MCU_PostUART(0xe1);
                            MCU_PostUART(bend & 127);
                            MCU_PostUART((bend >> 7) & 127);
                        }
                        else if (kk)
                        {
                            MCU_PostUART(0xc1);
                            MCU_PostUART(patch);
                            MCU_PostUART(0xe1);
                            MCU_PostUART(bend & 127);
                            MCU_PostUART((bend >> 7) & 127);
                            MCU_PostUART(0x91);
                            MCU_PostUART(0x32);
                            MCU_PostUART(0x7f);
                        }
                        else if (kk == 0)
                        {
                            //MCU_PostUART(0xc0);
                            //MCU_PostUART(patch);
                            MCU_PostUART(0xe0);
                            MCU_PostUART(0x00);
                            MCU_PostUART(0x40);
                            MCU_PostUART(0x99);
                            MCU_PostUART(0x37);
                            MCU_PostUART(0x7f);
                        }
                    }
                    else
                    {
                        if (kk == 1)
                        {
                            MCU_PostUART(0x91);
                            MCU_PostUART(0x32);
                            MCU_PostUART(0);
                        }
                        else if (kk == 0)
                        {
                            MCU_PostUART(0x99);
                            MCU_PostUART(0x37);
                            MCU_PostUART(0);
                        }
                        else if (kk == 4)
                        {
                            MCU_PostUART(0x99);
                            MCU_PostUART(0x32);
                            MCU_PostUART(0);
                        }
                    }
                }
#endif
        break;
    }
    }
}

void LCD_SDL_Backend::Render()
{
    SDL_UpdateTexture(m_texture, NULL, m_lcd->buffer, lcd_width_max * 4);
    SDL_RenderCopy(m_renderer, m_texture, NULL, NULL);
    SDL_RenderPresent(m_renderer);
}

bool LCD_SDL_Backend::IsQuitRequested() const
{
    return m_quit_requested;
}
