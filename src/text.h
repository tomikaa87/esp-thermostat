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
    Created on 2017-01-01
*/

#ifndef TEXT_H
#define	TEXT_H

#include <stdbool.h>
#include <stdint.h>
    
uint8_t text_draw(const char* s, uint8_t line, uint8_t x, uint8_t y_offset, bool invert);
void text_draw_char(char c, uint8_t line, uint8_t x, uint8_t y_offset, bool invert);
void text_draw_7seg_large(const char* number, uint8_t line, uint8_t x);

#endif	/* TEXT_H */

