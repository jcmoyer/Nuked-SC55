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
#include "lcd.h"
#include "emu.h"
#include "lcd_back.h"
#include "lcd_font.h"
#include <cstring>

void LCD_Enable(lcd_t& lcd, bool enable)
{
    lcd.enable = enable;
}

void LCD_Write(lcd_t& lcd, uint32_t address, uint8_t data)
{
    // No point taking a lock and updating LCD state if there's no one to observe it.
    if (!lcd.backend)
    {
        return;
    }

    std::scoped_lock lock(lcd.mutex);
    if (address == 0)
    {
        if ((data & 0xe0) == 0x20)
        {
            lcd.LCD_DL = (data & 0x10) != 0;
            lcd.LCD_N = (data & 0x8) != 0;
            lcd.LCD_F = (data & 0x4) != 0;
        }
        else if ((data & 0xf8) == 0x8)
        {
            lcd.LCD_D = (data & 0x4) != 0;
            lcd.LCD_C = (data & 0x2) != 0;
            lcd.LCD_B = (data & 0x1) != 0;
        }
        else if ((data & 0xff) == 0x01)
        {
            lcd.LCD_DD_RAM = 0;
            lcd.LCD_ID = 1;
            memset(lcd.LCD_Data, 0x20, sizeof(lcd.LCD_Data));
        }
        else if ((data & 0xff) == 0x02)
        {
            lcd.LCD_DD_RAM = 0;
        }
        else if ((data & 0xfc) == 0x04)
        {
            lcd.LCD_ID = (data & 0x2) != 0;
            lcd.LCD_S = (data & 0x1) != 0;
        }
        else if ((data & 0xc0) == 0x40)
        {
            lcd.LCD_CG_RAM = (data & 0x3f);
            lcd.LCD_RAM_MODE = 0;
        }
        else if ((data & 0x80) == 0x80)
        {
            lcd.LCD_DD_RAM = (data & 0x7f);
            lcd.LCD_RAM_MODE = 1;
        }
        else
        {
            address += 0;
        }
    }
    else
    {
        if (!lcd.LCD_RAM_MODE)
        {
            lcd.LCD_CG[lcd.LCD_CG_RAM] = data & 0x1f;
            if (lcd.LCD_ID)
            {
                lcd.LCD_CG_RAM++;
            }
            else
            {
                lcd.LCD_CG_RAM--;
            }
            lcd.LCD_CG_RAM &= 0x3f;
        }
        else
        {
            if (lcd.LCD_N)
            {
                if (lcd.LCD_DD_RAM & 0x40)
                {
                    if ((lcd.LCD_DD_RAM & 0x3f) < 40)
                        lcd.LCD_Data[(lcd.LCD_DD_RAM & 0x3f) + 40] = data;
                }
                else
                {
                    if ((lcd.LCD_DD_RAM & 0x3f) < 40)
                        lcd.LCD_Data[lcd.LCD_DD_RAM & 0x3f] = data;
                }
            }
            else
            {
                if (lcd.LCD_DD_RAM < 80)
                    lcd.LCD_Data[lcd.LCD_DD_RAM] = data;
            }
            if (lcd.LCD_ID)
            {
                lcd.LCD_DD_RAM++;
            }
            else
            {
                lcd.LCD_DD_RAM--;
            }
            lcd.LCD_DD_RAM &= 0x7f;
        }
    }
    //fprintf(stderr, "%i %.2x ", address, data);
    // if (data >= 0x20 && data <= 'z')
    //     fprintf(stderr, "%c\n", data);
    //else
    //    fprintf(stderr, "\n");
}

void LCD_Init(lcd_t& lcd, mcu_t& mcu)
{
    lcd.mcu = &mcu;
}

bool LCD_Start(lcd_t& lcd)
{
    bool success = true;

    if (lcd.mcu->romset == Romset::JV880)
    {
        lcd.width = 820;
        lcd.height = 100;
        lcd.color1 = 0x000000;
        lcd.color2 = 0x78b500;
    }
    else
    {
        lcd.width = 741;
        lcd.height = 268;
        lcd.color1 = 0x000000;
        lcd.color2 = 0x0050c8;
    }

    if (lcd.backend)
    {
        if (!lcd.backend->Start(lcd))
        {
            success = false;
        }
    }

    return success;
}

void LCD_Stop(lcd_t& lcd)
{
    if (lcd.backend)
    {
        lcd.backend->Stop();
    }
}

void LCD_FontRenderStandard(lcd_t& lcd, uint8_t* LCD_CG, int32_t x, int32_t y, uint8_t ch, bool overlay = false)
{
    uint8_t* f;
    if (ch >= 16)
        f = &lcd_font[ch - 16][0];
    else
        f = &LCD_CG[(ch & 7) * 8];
    for (int i = 0; i < 7; i++)
    {
        for (int j = 0; j < 5; j++)
        {
            uint32_t col;
            if (f[i] & (1<<(4-j)))
            {
                col = lcd.color1;
            }
            else
            {
                col = lcd.color2;
            }
            int xx = x + i * 6;
            int yy = y + j * 6;
            for (int ii = 0; ii < 5; ii++)
            {
                for (int jj = 0; jj < 5; jj++)
                {
                    if (overlay)
                        lcd.buffer[xx+ii][yy+jj] &= col;
                    else
                        lcd.buffer[xx+ii][yy+jj] = col;
                }
            }
        }
    }
}

void LCD_FontRenderLevel(lcd_t& lcd, uint8_t* LCD_CG, int32_t x, int32_t y, uint8_t ch, uint8_t width = 5)
{
    uint8_t* f;
    if (ch >= 16)
        f = &lcd_font[ch - 16][0];
    else
        f = &LCD_CG[(ch & 7) * 8];
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < width; j++)
        {
            uint32_t col;
            if (f[i] & (1<<(4-j)))
            {
                col = lcd.color1;
            }
            else
            {
                col = lcd.color2;
            }
            int xx = x + i * 11;
            int yy = y + j * 26;
            for (int ii = 0; ii < 9; ii++)
            {
                for (int jj = 0; jj < 24; jj++)
                {
                    lcd.buffer[xx+ii][yy+jj] = col;
                }
            }
        }
    }
}

static const uint8_t LR[2][12][11] =
{
    {
        {1,1,0,0,0,0,0,0,0,0,0,},
        {1,1,0,0,0,0,0,0,0,0,0,},
        {1,1,0,0,0,0,0,0,0,0,0,},
        {1,1,0,0,0,0,0,0,0,0,0,},
        {1,1,0,0,0,0,0,0,0,0,0,},
        {1,1,0,0,0,0,0,0,0,0,0,},
        {1,1,0,0,0,0,0,0,0,0,0,},
        {1,1,0,0,0,0,0,0,0,0,0,},
        {1,1,0,0,0,0,0,0,0,0,0,},
        {1,1,0,0,0,0,0,0,0,0,0,},
        {1,1,1,1,1,1,1,1,1,1,1,},
        {1,1,1,1,1,1,1,1,1,1,1,},
    },
    {
        {1,1,1,1,1,1,1,1,1,0,0,},
        {1,1,1,1,1,1,1,1,1,1,0,},
        {1,1,0,0,0,0,0,0,1,1,0,},
        {1,1,0,0,0,0,0,0,1,1,0,},
        {1,1,0,0,0,0,0,0,1,1,0,},
        {1,1,1,1,1,1,1,1,1,1,0,},
        {1,1,1,1,1,1,1,1,1,0,0,},
        {1,1,0,0,0,0,0,1,1,0,0,},
        {1,1,0,0,0,0,0,0,1,1,0,},
        {1,1,0,0,0,0,0,0,1,1,0,},
        {1,1,0,0,0,0,0,0,0,1,1,},
        {1,1,0,0,0,0,0,0,0,1,1,},
    }
};

static const int LR_xy[2][2] = {
    { 70, 264 },
    { 232, 264 }
};


void LCD_FontRenderLR(lcd_t& lcd, uint8_t* LCD_CG, uint8_t ch)
{
    uint8_t* f;
    if (ch >= 16)
        f = &lcd_font[ch - 16][0];
    else
        f = &LCD_CG[(ch & 7) * 8];
    uint32_t col;
    if (f[0] & 1)
    {
        col = lcd.color1;
    }
    else
    {
        col = lcd.color2;
    }
    for (int letter = 0; letter < 2; letter++)
    {
        for (int i = 0; i < 12; i++)
        {
            for (int j = 0; j < 11; j++)
            {
                if (LR[letter][i][j])
                    lcd.buffer[i+LR_xy[letter][0]][j+LR_xy[letter][1]] = col;
            }
        }
    }
}

void LCD_Render(lcd_t& lcd)
{
    if (!lcd.backend)
    {
        return;
    }

    if (!lcd.mcu->is_cm300 && !lcd.mcu->is_st && !lcd.mcu->is_scb55)
    {
        if (!lcd.mutex.try_lock())
        {
            // if the MCU is currently updating something, just drop the frame
            return;
        }

        // This is the only shared mutable state we need to complete rendering. Since rendering is relatively expensive,
        // we'll quickly take a copy, release the lock, and use it for this frame.
        uint32_t LCD_C      = lcd.LCD_C;
        uint32_t LCD_DD_RAM = lcd.LCD_DD_RAM;

        uint8_t LCD_CG[sizeof(lcd.LCD_CG)];
        memcpy(LCD_CG, lcd.LCD_CG, sizeof(LCD_CG));

        uint8_t LCD_Data[sizeof(lcd.LCD_Data)];
        memcpy(LCD_Data, lcd.LCD_Data, sizeof(LCD_Data));

        lcd.mutex.unlock();

        if (!lcd.enable && !lcd.mcu->is_jv880)
        {
            memset(lcd.buffer, 0, sizeof(lcd.buffer));
        }
        else
        {
            if (lcd.mcu->is_jv880)
            {
                for (size_t i = 0; i < lcd.height; i++) {
                    for (size_t j = 0; j < lcd.width; j++) {
                        lcd.buffer[i][j] = 0xFF03be51;
                    }
                }
            }
            else
            {
                for (size_t i = 0; i < lcd.height; i++) {
                    for (size_t j = 0; j < lcd.width; j++) {
                        lcd.buffer[i][j] = back_palette[back_data[i * lcd.width + j]];
                    }
                }
            }

            if (lcd.mcu->is_jv880)
            {
                for (int i = 0; i < 2; i++)
                {
                    for (int j = 0; j < 24; j++)
                    {
                        uint8_t ch = LCD_Data[i * 40 + j];
                        LCD_FontRenderStandard(lcd, LCD_CG, 4 + i * 50, 4 + j * 34, ch);
                    }
                }
                
                // cursor
                int j = LCD_DD_RAM % 0x40;
                int i = LCD_DD_RAM / 0x40;
                if (i < 2 && j < 24 && LCD_C)
                    LCD_FontRenderStandard(lcd, LCD_CG, 4 + i * 50, 4 + j * 34, '_', true);
            }
            else
            {
                for (int i = 0; i < 3; i++)
                {
                    uint8_t ch = LCD_Data[0 + i];
                    LCD_FontRenderStandard(lcd, LCD_CG, 11, 34 + i * 35, ch);
                }
                for (int i = 0; i < 16; i++)
                {
                    uint8_t ch = LCD_Data[3 + i];
                    LCD_FontRenderStandard(lcd, LCD_CG, 11, 153 + i * 35, ch);
                }
                for (int i = 0; i < 3; i++)
                {
                    uint8_t ch = LCD_Data[40 + i];
                    LCD_FontRenderStandard(lcd, LCD_CG, 75, 34 + i * 35, ch);
                }
                for (int i = 0; i < 3; i++)
                {
                    uint8_t ch = LCD_Data[43 + i];
                    LCD_FontRenderStandard(lcd, LCD_CG, 75, 153 + i * 35, ch);
                }
                for (int i = 0; i < 3; i++)
                {
                    uint8_t ch = LCD_Data[49 + i];
                    LCD_FontRenderStandard(lcd, LCD_CG, 139, 34 + i * 35, ch);
                }
                for (int i = 0; i < 3; i++)
                {
                    uint8_t ch = LCD_Data[46 + i];
                    LCD_FontRenderStandard(lcd, LCD_CG, 139, 153 + i * 35, ch);
                }
                for (int i = 0; i < 3; i++)
                {
                    uint8_t ch = LCD_Data[52 + i];
                    LCD_FontRenderStandard(lcd, LCD_CG, 203, 34 + i * 35, ch);
                }
                for (int i = 0; i < 3; i++)
                {
                    uint8_t ch = LCD_Data[55 + i];
                    LCD_FontRenderStandard(lcd, LCD_CG, 203, 153 + i * 35, ch);
                }

                LCD_FontRenderLR(lcd, LCD_CG, LCD_Data[58]);

                for (int i = 0; i < 2; i++)
                {
                    for (int j = 0; j < 4; j++)
                    {
                        uint8_t ch = LCD_Data[20 + j + i * 40];
                        LCD_FontRenderLevel(lcd, LCD_CG, 71 + i * 88, 293 + j * 130, ch, j == 3 ? 1 : 5);
                    }
                }
            }
        }

        lcd.backend->Render();
    }
}

