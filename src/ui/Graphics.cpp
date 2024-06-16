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

#include <array>
#include <string_view>
#include <tuple>

using namespace UI;

OLEDGraphics::OLEDGraphics()
{
    DisplayImpl::init();
    DisplayImpl::powerOn();
    DisplayImpl::setContrast(0);
}

void OLEDGraphics::drawBitmap(
    const unsigned x,
    const unsigned line,
    const std::span<const uint8_t> bitmap
)
{
    if (line > DisplayImpl::Lines || bitmap.size() == 0 || x + bitmap.size() >= DisplayImpl::Width)
        return;

    DisplayImpl::setLine(line);
    DisplayImpl::setColumn(x);
    DisplayImpl::sendData(bitmap.data(), bitmap.size(), 0, false);
}

void OLEDGraphics::drawBitmap(
    const unsigned x,
    const unsigned startLine,
    const std::span<const uint8_t> bitmap,
    const unsigned width,
    const unsigned pageCount
)
{
    if (startLine + pageCount > DisplayImpl::Lines)
        return;

    auto offset{ 0 };
    for (uint8_t line = startLine; line < startLine + pageCount; ++line) {
        drawBitmap(x, line, bitmap.subspan(offset, width));
        offset += width;
    }
}

void OLEDGraphics::fillArea(
    const unsigned x,
    const unsigned line,
    const unsigned width,
    const unsigned pages,
    const Color color
)
{
    const auto pattern{ static_cast<uint8_t>(color == Color::White ? 0xFFu : 0u) };
    DisplayImpl::fillArea(x, line, width, pages, pattern);
}

void OLEDGraphics::drawScheduleBar(const std::span<uint8_t, 42>& scheduleBits)
{
    static constexpr uint8_t longTick = 0b11110000;
    static constexpr uint8_t shortTick = 0b01110000;
    static constexpr uint8_t setIndicator = 0b00010111;
    static constexpr uint8_t clearedIndicator = 0b00010000;

    DisplayImpl::setLine(6);
    DisplayImpl::setColumn(3);

    uint8_t tickCounter = 0;
    uint8_t longTickCounter = 0;
    uint8_t scheduleByteIdx = 0;
    uint8_t scheduleBitIdx = 255; // Will overflow in the first round
    uint8_t indicatorCounter = 0;
    uint8_t scheduleBitValue = 0;

    for (uint8_t x = 0; x < 121; ++x) {
        uint8_t bitmap;

        if (tickCounter == 0) {
            // Draw ticks
            if (longTickCounter == 0)
                bitmap = longTick;
            else
                bitmap = shortTick;
        } else {
            // Draw rest of the bar with or without the indicators
            if (indicatorCounter < 2 && scheduleBitValue)
                bitmap = setIndicator;
            else
                bitmap = clearedIndicator;
        }

        Display::sendData(bitmap);

        if (++tickCounter == 5) {
            tickCounter = 0;
            if (++longTickCounter == 6)
                longTickCounter = 0;
        }

        ++indicatorCounter;
        if (tickCounter == 1 || tickCounter == 3) {
            indicatorCounter = 0;
            if (++scheduleBitIdx == 8) {
                ++scheduleByteIdx;
                scheduleBitIdx = 0;
            }

            scheduleBitValue = (scheduleBits[scheduleByteIdx] >> scheduleBitIdx) & 1;
        }
    }

    using namespace std::string_view_literals;
    using Label = std::tuple<std::string_view, int>;

    static constexpr auto labels = {
        Label{ "0"sv, 1 },
        Label{ "6"sv, 31 },
        Label{ "12"sv, 58 },
        Label{ "18"sv, 88 },
        Label{ "24"sv, 115 }
    };

    for (const auto& [text, x] : labels) {
        drawText(x, 7, text, Resources::Fonts::Oled);
    }
}

void OLEDGraphics::drawScheduleBarPositionIndicator(const uint8_t scheduleBitIndex)
{
    static constexpr uint8_t indicatorBitmap[] = {
        0b00010000,
        0b00100000,
        0b01111100,
        0b00100000,
        0b00010000
    };

    uint8_t x = 2; // initial offset from left
    x += scheduleBitIndex << 1;	// for every "tick"
    x += scheduleBitIndex >> 1;	// for every padding between "ticks"

    /*
     0:     v
     1:     . v
     2:     . .  v
     3:     . .  . v
     4:     . .  . .  v
     5:     . .  . .  . v
            |||| |||| ||||
            01234567890123

        0 -> 0
        1 -> 2
        2 -> 5
        3 -> 7
        4 -> 10
        5 -> 12
     */

    Display::fillArea(0, 5, 128, 1, 0);

    drawBitmap(x, 5, indicatorBitmap);
}

void Graphics::drawShortWeekday(const unsigned x, const unsigned line, const unsigned weekday)
{
    if (weekday > 6) {
        return;
    }

    using namespace std::string_view_literals;

    static constexpr std::array days{
        "Sun"sv,
        "Mon"sv,
        "Tue"sv,
        "Wed"sv,
        "Thu"sv,
        "Fri"sv,
        "Sat"sv
    };

    drawText(x, line, days[weekday], Resources::Fonts::Oled);
}