/*
    This file is part of esp-thermostat.

    esp-thermostat is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    esp-thermostat is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with esp-thermostat.  If not, see <http://www.gnu.org/licenses/>.

    Author: Tamas Karpati
    Created on 2017-01-02
*/

#include "graphics.h"
#include "display/Display.h"

#include <pgmspace.h>

void graphics_draw_bitmap(
    const uint8_t* bitmap,
    uint8_t width,
    uint8_t x,
    uint8_t line)
{
    if (line > Display::Lines || width == 0 || x + width >= Display::Width)
        return;

    Display::setLine(line);
    Display::setColumn(x);

    char buf[128] = { 0 };
    memcpy_P(buf, bitmap, width);

    Display::sendData(bitmap, width, 0, false);
}

void graphics_draw_multipage_bitmap(
    const uint8_t* mp_bitmap,
    uint8_t width,
    uint8_t lineCount,
    uint8_t x,
    uint8_t startLine)
{
    if (startLine + lineCount > Display::Lines)
        return;

    const uint8_t* bitmap = mp_bitmap;

    for (uint8_t line = startLine; line < startLine + lineCount; ++line) {
        graphics_draw_bitmap(bitmap, width, x, line);
        bitmap += width;
    }
}

const uint8_t graphics_flame_icon_20x3p[20 * 3] PROGMEM = {
    // page 0
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b11100000,
    0b11111000,
    0b00011100,
    0b00001110,
    0b11111111,
    0b11110000,
    0b00000000,
    0b00000000,
    0b10000000,
    0b11000000,
    0b11100000,
    0b11100000,
    0b00000000,
    0b00000000,

    // page 1
    0b11110000,
    0b11111100,
    0b00001110,
    0b00111100,
    0b01110000,
    0b01101110,
    0b11111111,
    0b00000001,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000011,
    0b00000110,
    0b00001100,
    0b00011111,
    0b00000011,
    0b00000000,
    0b00001111,
    0b11111111,
    0b11110000,

    // page 2
    0b00000000,
    0b00000111,
    0b00001111,
    0b00011100,
    0b00111000,
    0b01110000,
    0b01100000,
    0b11000000,
    0b11000000,
    0b11000000,
    0b11000000,
    0b11000000,
    0b11000000,
    0b01100000,
    0b01110000,
    0b00111000,
    0b00011100,
    0b00001111,
    0b00000111,
    0b00000000
};

const uint8_t graphics_off_icon_20x3p[20 * 3] PROGMEM = {
    // page 0
    0b00000000,
    0b00000000,
    0b00000000,
    0b10000000,
    0b11000000,
    0b11100000,
    0b01100000,
    0b00000000,
    0b00000000,
    0b11111110,
    0b11111110,
    0b00000000,
    0b00000000,
    0b01100000,
    0b11100000,
    0b11000000,
    0b10000000,
    0b00000000,
    0b00000000,
    0b00000000,

    // page 1
    0b11111000,
    0b11111110,
    0b00000111,
    0b00000011,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000111,
    0b00000111,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000011,
    0b00000111,
    0b11111110,
    0b11111000,

    // page 2
    0b00000001,
    0b00000111,
    0b00001110,
    0b00011100,
    0b00110000,
    0b01110000,
    0b01100000,
    0b11000000,
    0b11000000,
    0b11000000,
    0b11000000,
    0b11000000,
    0b11000000,
    0b01100000,
    0b01110000,
    0b00110000,
    0b00011100,
    0b00001110,
    0b00000111,
    0b00000001
};

const uint8_t graphics_calendar_icon_20x3p[20 * 3] PROGMEM = {
    // page 0
    0b11000000,
    0b11100000,
    0b01100000,
    0b01100000,
    0b11111000,
    0b11111100,
    0b11111000,
    0b01100000,
    0b01100000,
    0b01100000,
    0b01100000,
    0b01100000,
    0b01100000,
    0b11111000,
    0b11111100,
    0b11111000,
    0b01100000,
    0b01100000,
    0b11100000,
    0b11000000,

    // page 1
    0b11111111,
    0b11111111,
    0b00000000,
    0b01100000,
    0b01100000,
    0b00000001,
    0b00000000,
    0b01101100,
    0b01101100,
    0b00000000,
    0b00000000,
    0b01101100,
    0b01101100,
    0b00000000,
    0b00000001,
    0b01101100,
    0b01101100,
    0b00000000,
    0b11111111,
    0b11111111,

    // page 2
    0b00001111,
    0b00011111,
    0b00011000,
    0b00011011,
    0b00011011,
    0b00011000,
    0b00011000,
    0b00011011,
    0b00011011,
    0b00011000,
    0b00011000,
    0b00011000,
    0b00011000,
    0b00011000,
    0b00011000,
    0b00011000,
    0b00011000,
    0b00011000,
    0b00011111,
    0b00001111
};