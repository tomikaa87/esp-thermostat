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
    Created on 2017-01-07
*/

#include "DrawHelper.h"
#include "Graphics.h"
#include "Extras.h"

#include "display/Display.h"
#include "display/Text.h"

#include <stdio.h>

void draw_temperature_value(uint8_t x, int8_t int_part, int8_t frac_part)
{
    if (int_part < 0 || frac_part < 0)
        Text::draw7Seg("-", 2, x - 13);
    else
        Text::draw7Seg(" ", 2, x - 13);

    // Draw integral part of the value
    char s[4] = { 0 };
    sprintf(s, "%02d", int_part >= 0 ? int_part : -int_part);
    Text::draw7Seg(s, 2, x);

    // Draw the decimal point
    static const uint8_t dp_bitmap[] = { 0b01100000, 0b01100000 };
    Display::setLine(4);
    Display::setColumn(x + 28);
    Display::sendData(dp_bitmap, sizeof(dp_bitmap), 0, false);

    // Draw fractional part of the value
    sprintf(s, "%d", frac_part >= 0 ? frac_part : -frac_part);
    Text::draw7Seg(s, 2, x + 32);

    // Draw the degree symbol
    static const uint8_t ds_bitmap[6] = {
        0b00011110,
        0b00101101,
        0b00110011,
        0b00110011,
        0b00101101,
        0b00011110
    };

    Display::setLine(2);
    Display::setColumn(x + 48);
    Display::sendData(ds_bitmap, sizeof(ds_bitmap), 1, false);

    // Draw temperature unit
    Text::draw7Seg("C ", 2, x + 56);
}