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

#include <cstdlib>
#include <span>
#include <string_view>

#include <Arduino.h>

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

    void drawBitmap(unsigned x, unsigned line, std::span<const uint8_t> bitmap);
    void drawBitmap(unsigned x, unsigned line, std::span<const uint8_t> bitmap, unsigned width, unsigned pageCount);
    void fillArea(unsigned x, unsigned line, unsigned width, unsigned pages, Color color);

    void drawScheduleBar(const std::span<uint8_t, 42>& scheduleBits);
    void drawScheduleBarPositionIndicator(uint8_t scheduleBitIndex);

    void drawShortWeekday(unsigned x, unsigned line, unsigned weekday);

    template <std::size_t Width, std::size_t Pages>
    void drawBitmap(unsigned x, unsigned line, const Resources::Assets::MultiPageBitmap<Width, Pages>& bitmap)
    {
        drawBitmap(x, line, bitmap.bitmap, bitmap.width, bitmap.pages);
    }

    template <typename Font>
    void drawChar(
        const char c,
        const Font& font,
        const unsigned yOffset = 0,
        const bool inverted = false
    )
    {
        const auto glyphIndex{ static_cast<std::size_t>(c - 32) };
        const auto* charData{ font.placeholder };

        if (glyphIndex < font.charCount) {
            charData = font.glyphs[glyphIndex];
        }

        DisplayImpl::sendData(charData, font.charWidth, yOffset, inverted);
    }

    template <typename Font>
    unsigned drawText(
        unsigned x,
        const unsigned line,
        const std::string_view& text,
        const Font& font,
        const unsigned yOffset = 0,
        const bool inverted = false
    )
    {
        static constexpr auto CharacterSpacing{ 1 };
        static constexpr std::array<uint8_t, CharacterSpacing> BackgroundPattern{{}};

        DisplayImpl::setLine(line);

        for (uint8_t i = 0; i < text.length(); ++i) {
            DisplayImpl::setColumn(x);

            x += font.charWidth + CharacterSpacing;

            drawChar(text[i], font, yOffset, inverted);

            // Fill the background between letters
            DisplayImpl::sendData(BackgroundPattern.data(), BackgroundPattern.size(), yOffset, inverted);

            // Stop if the next character won't fit
            if (x > DisplayImpl::Driver::Width - 1) {
                return x;
            }
        }

        return x;
    }

    template <typename LargeNumberFont>
    unsigned drawLargeNumber(
        unsigned x,
        const unsigned line,
        int number,
        const LargeNumberFont& font,
        const bool inverted = false
    )
    {
        static constexpr auto CharacterSpacing{ 1 };
        static constexpr std::array<uint8_t, CharacterSpacing> BackgroundPattern{};

        const bool negative{ number < 0 };

        Serial.printf("number=%d, x=%u, line=%u, font.charWidth=%u, font.charPages=%u\r\n",
            number, x, line, font.charWidth, font.charPages
        );

        // By storing the individual digits, calculating character positions
        // is much easier for left-aligned drawing
        std::array<uint8_t, 10> digits{{}};

        for (int i = static_cast<int>(digits.size()) - 1; i >= 0; --i) {
            digits[i] = std::abs(number % 10);
            number /= 10;
            Serial.printf("digits[%u]=%u, number=%d\r\n", i, digits[i], number);
        }

        if (negative) {
            for (auto page = 0u; page < font.charPages; ++page) {
                DisplayImpl::setColumn(x);
                DisplayImpl::setLine(page + line);
                DisplayImpl::sendData(font.negativeSignGlyph[page], font.charWidth, inverted);
                DisplayImpl::sendData(BackgroundPattern.data(), BackgroundPattern.size(), inverted);
            }

            x += font.charWidth + CharacterSpacing;
        }

        bool skipZeros{ true };
        for (const auto digit : digits) {
            Serial.printf("digit=%u\r\n", digit);

            if (digit == 0 && skipZeros) {
                continue;
            }

            skipZeros = false;

            for (auto page = 0u; page < font.charPages; ++page) {
                Serial.printf("page=%u\r\n", page);

                DisplayImpl::setColumn(x);
                DisplayImpl::setLine(page + line);
                DisplayImpl::sendData(font.glyphs[digit][page], font.charWidth, inverted);
                DisplayImpl::sendData(BackgroundPattern.data(), BackgroundPattern.size(), inverted);
            }

            x += font.charWidth + CharacterSpacing;
        }

        return x;
    }
};

using Graphics = OLEDGraphics;

}
