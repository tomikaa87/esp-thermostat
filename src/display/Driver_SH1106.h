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

namespace Driver
{

class SH1106
{
public:
    enum class ChargePumpVoltage
    {
        _6_4V,
        _7_4V,
        _8_0V,
        _9_0V
    };

    static constexpr uint8_t Height = 64;
    static constexpr uint8_t Width = 128;
    static constexpr uint8_t Lines = 8;

    static void init();

    static void fill(uint8_t pattern);

    static void sendCommand(uint8_t code);
    static void sendCommand(uint8_t code, uint8_t arg);

    static void sendData(uint8_t data, uint8_t bitShift = 0, bool invert = false);
    static void sendData(const uint8_t* data, uint8_t length, uint8_t bitShift = 0, bool invert = false);

    static bool isPoweredOn();
    static void setPowerOn(bool on);
    static void setChargePumpVoltage(ChargePumpVoltage voltage);
    static void setComPadsAltHwConfig(bool alternative);
    static void setComScanInverted(bool inverted);
    static void setContrast(uint8_t value);
    static void setDcDcConvOn(bool on);
    static void setDischargePrechargePeriod(uint8_t precharge, uint8_t discharge);
    static void setDisplayClockDivRatio(uint8_t div, uint8_t ratio);
    static void setDisplayStartLine(uint8_t value);
    static void setDisplayOffset(uint8_t value);
    static void setInvertedDisplay(bool inverted);
    static void setMultiplexRatio(uint8_t value);
    static void setSegmentRemap(bool reverse);
    static void setVcomDeselectLevel(uint8_t value);

    static void setColumn(uint8_t column);
    static void setLine(uint8_t line);

private:
    static bool _poweredOn;
};

}
