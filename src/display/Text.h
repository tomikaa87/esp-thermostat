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
    Created on 2020-01-22
*/

#pragma once

#include <cstdint>

namespace Text
{
    void draw(char c, uint8_t line, uint8_t x, uint8_t yOffset, bool invert);
    uint8_t draw(const char* s, uint8_t line, uint8_t x, uint8_t yOffset, bool invert);
    void draw7Seg(const char* number, uint8_t line, uint8_t x);
}