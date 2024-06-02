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

#include "Graphics.h"
#include "Resources.h"

#include "display/Display.h"

using namespace UI;

OLEDGraphics::OLEDGraphics()
{
    Display::init();
    Display::powerOn();
    Display::setContrast(0);
}

void OLEDGraphics::drawBitmap(
    const int x,
    const int line,
    const std::span<const uint8_t> bitmap
)
{
    if (line > Display::Lines || bitmap.size() == 0 || x + bitmap.size() >= Display::Width)
        return;

    Display::setLine(line);
    Display::setColumn(x);
    Display::sendData(bitmap.data(), bitmap.size(), 0, false);
}

void OLEDGraphics::drawBitmap(
    const int x,
    const int startLine,
    const std::span<const uint8_t> bitmap,
    const int width,
    const int pageCount
)
{
    if (startLine + pageCount > Display::Lines)
        return;

    auto offset{ 0 };
    for (uint8_t line = startLine; line < startLine + pageCount; ++line) {
        drawBitmap(x, line, bitmap.subspan(offset, width));
        offset += width;
    }
}

void OLEDGraphics::drawChar(const char c, const int yOffset, const bool inverted)
{
    const uint8_t* charData;

    namespace Font = Resources::Fonts::Default;

    // If character is not supported, draw placeholder
    if ((c - 32) >= Font::CharacterCount) {
        charData = Font::PlaceholderData;
    }
    else {
        // Get data for the next character
        charData = Font::Data[c - 32];
    }

    Display::sendData(charData, Font::CharacterWidth, yOffset, inverted);
}

int OLEDGraphics::drawText(
    int x,
    const int line,
    const std::string_view& text,
    const int yOffset,
    const bool inverted
)
{
    namespace Font = Resources::Fonts::Default;

    static constexpr auto CharacterSpacing{ 1 };

    Display::setLine(line);

    for (uint8_t i = 0; i < text.length(); ++i) {
        Display::setColumn(x);

        x += Font::CharacterWidth + CharacterSpacing;

        drawChar(text[i], yOffset, inverted);

        // Fill the background between letters
        static const uint8_t BackgroundPattern[CharacterSpacing] = { 0 };
        Display::sendData(BackgroundPattern, CharacterSpacing, yOffset, inverted);

        // Stop if the next character won't fit
        if (x > Display::Driver::Width - 1) {
            return x;
        }
    }

    return x;
}

void OLEDGraphics::fillArea(
    const int x,
    const int line,
    const int width,
    const int pages,
    const Color color
)
{
    const auto pattern{ static_cast<uint8_t>(color == Color::White ? 0xFFu : 0u) };
    Display::fillArea(x, line, width, pages, pattern);
}


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

const uint8_t graphics_flame_icon_20x3p[20 * 3] = {
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

const uint8_t graphics_off_icon_20x3p[20 * 3] = {
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

const uint8_t graphics_calendar_icon_20x3p[20 * 3] = {
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