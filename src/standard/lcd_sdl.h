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
#pragma once

#include "lcd.h"
#include <SDL.h>

class LCD_SDL_Backend : public LCD_Backend
{
public:
    LCD_SDL_Backend() = default;
    ~LCD_SDL_Backend();

    bool Start(const lcd_t& lcd);
    void Stop();

    void HandleEvent(const SDL_Event& ev);
    void Render();

    bool IsQuitRequested() const;

private:
    const lcd_t*  m_lcd      = nullptr;
    SDL_Window*   m_window   = nullptr;
    SDL_Renderer* m_renderer = nullptr;
    SDL_Texture*  m_texture  = nullptr;

    // When the user closes the window this becomes true
    bool m_quit_requested = false;
};
