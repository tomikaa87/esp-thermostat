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

#include "Resources.h"

#include "display/Display.h"

#include <span>
#include <string_view>

namespace UI
{

class GraphicsBase
{
public:
    enum class Color
    {
        Black,
        White
    };
};

class OLEDGraphics : public GraphicsBase
{
public:
    using DisplayImpl = Display;

    static constexpr auto Width{ 128 };
    static constexpr auto Height{ 64 };
    static constexpr auto Lines{ 8 };

    OLEDGraphics();

    void drawBitmap(int x, int line, std::span<const uint8_t> bitmap);
    void drawBitmap(int x, int line, std::span<const uint8_t> bitmap, int width, int pageCount);
    void fillArea(int x, int line, int width, int pages, Color color);

    void drawScheduleBar(const std::span<uint8_t, 42>& scheduleBits);
    void drawScheduleBarPositionIndicator(uint8_t scheduleBitIndex);
    
    void drawShortWeekday(int x, int line, int weekday);

    template <std::size_t Width, std::size_t Pages>
    void drawBitmap(int x, int line, const Resources::Assets::MultiPageBitmap<Width, Pages>& bitmap)
    {
        drawBitmap(x, line, bitmap.bitmap, bitmap.width, bitmap.pages);
    }

    template <typename Font>
    void drawChar(
        const char c,
        const Font& font,
        const int yOffset = 0,
        const bool inverted = false
    )
    {
        const auto glyphIndex{ static_cast<std::size_t>(c - 31) };
        const auto* charData{ font.placeholder };

        if (glyphIndex < font.charCount) {
            charData = font.glyphs[glyphIndex];
        }

        DisplayImpl::sendData(charData, font.charWidth, yOffset, inverted);
    }

    template <typename Font>
    int drawText(
        int x,
        const int line,
        const std::string_view& text,
        const Font& font,
        const int yOffset = 0,
        const bool inverted = false
    )
    {
        static constexpr auto CharacterSpacing{ 1 };

        DisplayImpl::setLine(line);

        for (uint8_t i = 0; i < text.length(); ++i) {
            DisplayImpl::setColumn(x);

            x += font.charWidth + CharacterSpacing;

            drawChar(text[i], font, yOffset, inverted);

            // Fill the background between letters
            static const uint8_t BackgroundPattern[CharacterSpacing] = { 0 };
            DisplayImpl::sendData(BackgroundPattern, CharacterSpacing, yOffset, inverted);

            // Stop if the next character won't fit
            if (x > DisplayImpl::Driver::Width - 1) {
                return x;
            }
        }

        return x;
    }
};

using Graphics = OLEDGraphics;

}
