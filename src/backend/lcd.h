/*
 * Copyright (C) 2021, 2024 nukeykt
 * Modified by J.C. Moyer
 * Original source file: src/lcd.h
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

#include <atomic>
#include <cstdint>
#include <mutex>

struct mcu_t;
struct lcd_t;

static const int lcd_width_max = 1024;
static const int lcd_height_max = 1024;

class LCD_Backend
{
public:
    virtual ~LCD_Backend() = default;

    // Called on LCD_Start. The backend should fully initialize itself here.
    virtual bool Start(const lcd_t& lcd) = 0;

    // Called on LCD_Stop. The backend can choose to clean up resources here or keep them around in case the LCD is
    // started again.
    virtual void Stop() = 0;

    // Called on LCD_Render. The backend should display a frame to the user.
    virtual void Render() = 0;
};

struct lcd_t {
    mcu_t* mcu = nullptr;

    size_t width = 0;
    size_t height = 0;

    uint32_t color1 = 0;
    uint32_t color2 = 0;

    // all the variables in this group are updated by the MCU via LCD_Write
    uint32_t LCD_DL = 0, LCD_N = 0, LCD_F = 0, LCD_D = 0, LCD_C = 0, LCD_B = 0, LCD_ID = 0, LCD_S = 0;
    uint32_t LCD_DD_RAM = 0, LCD_AC = 0, LCD_CG_RAM = 0;
    uint32_t LCD_RAM_MODE = 0;
    uint8_t LCD_Data[80]{};
    uint8_t LCD_CG[64]{};

    // updated by MCU via LCD_Enable
    std::atomic<bool> enable = 0;

    uint32_t buffer[lcd_height_max][lcd_width_max]{};

    std::mutex mutex;

    LCD_Backend* backend = nullptr;
};


void LCD_Init(lcd_t& lcd, mcu_t& mcu);
bool LCD_Start(lcd_t& lcd);
void LCD_Stop(lcd_t& lcd);
void LCD_Write(lcd_t& lcd, uint32_t address, uint8_t data);
void LCD_Enable(lcd_t& lcd, bool enable);
void LCD_Render(lcd_t& lcd);
