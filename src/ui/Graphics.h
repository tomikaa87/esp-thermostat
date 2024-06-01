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

#pragma once

#include <span>
#include <string_view>

namespace UI
{

class Graphics
{
public:
    enum class Color
    {
        Black,
        White
    };
};

class OLEDGraphics : public Graphics
{
public:
    static constexpr auto Width{ 128 };
    static constexpr auto Height{ 64 };
    static constexpr auto Lines{ 8 };

    OLEDGraphics();

    void drawBitmap(int x, int line, std::span<const uint8_t> bitmap);
    void drawBitmap(int x, int line, std::span<const uint8_t> bitmap, int width, int pageCount);
    int drawText(int x, int line, const std::string_view& text, int yOffset, bool inverted);
    void drawChar(char c, int yOffset, bool inverted);
    void fillArea(int x, int line, int width, int pages, Color color);
};

using DefaultGraphics = OLEDGraphics;

}

#include <stdint.h>

extern const uint8_t graphics_flame_icon_20x3p[];
extern const uint8_t graphics_off_icon_20x3p[];
extern const uint8_t graphics_calendar_icon_20x3p[];

void graphics_draw_bitmap(
    const uint8_t* bitmap,
    uint8_t width,
    uint8_t x,
    uint8_t line);

void graphics_draw_multipage_bitmap(
    const uint8_t* mp_bitmap,
    uint8_t width,
    uint8_t page_count,
    uint8_t x,
    uint8_t start_page);

