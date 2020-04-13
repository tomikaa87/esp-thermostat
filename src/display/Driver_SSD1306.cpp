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

#include "Driver_SSD1306.h"
#include "drivers/SimpleI2C.h"

using namespace Driver;
using namespace Drivers; // I2C

bool SSD1306::_poweredOn = false;

#define SSD1306_128_64

enum {
    SSD1306_I2C_ADDRESS = 0x3Cu,
    SSD1306_I2C_DC_FLAG = 0x40u,
    SSD1306_I2C_CO_FLAG = 0x80u
};

enum {
#if defined SSD1306_128_64
    SSD1306_LCDWIDTH = 128u,
    SSD1306_LCDHEIGHT = 64u,
    SSD1306_PAGE_COUNT = 8u,
#elif defined SSD1306_128_32
    SSD1306_LCDWIDTH = 128u,
    SSD1306_LCDHEIGHT = 32u,
    SSD1306_PAGE_COUNT = 4u,
#elif defined SSD1306_96_16
    SSD1306_LCDWIDTH = 96u,
    SSD1306_LCDHEIGHT = 16u,
    SSD1306_PAGE_COUNT = 2u,
#endif
};

enum {
    SSD1306_CMD_SETCONTRAST = 0x81u,
    SSD1306_CMD_DISPLAYALLON_RESUME = 0xA4u,
    SSD1306_CMD_DISPLAYALLON = 0xA5u,
    SSD1306_CMD_NORMALDISPLAY = 0xA6u,
    SSD1306_CMD_INVERTDISPLAY = 0xA7u,
    SSD1306_CMD_DISPLAYOFF = 0xAEu,
    SSD1306_CMD_DISPLAYON = 0xAFu,
    SSD1306_CMD_SETDISPLAYOFFSET = 0xD3u,
    SSD1306_CMD_SETCOMPINS = 0xDAu,
    SSD1306_CMD_SETVCOMDETECT = 0xDBu,
    SSD1306_CMD_SETDISPLAYCLOCKDIV = 0xD5u,
    SSD1306_CMD_SETPRECHARGE = 0xD9u,
    SSD1306_CMD_SETMULTIPLEX = 0xA8u,
    SSD1306_CMD_SETLOWCOLUMN = 0x00u,
    SSD1306_CMD_SETHIGHCOLUMN = 0x10u,
    SSD1306_CMD_SETSTARTLINE = 0x40u,
    SSD1306_CMD_MEMORYMODE = 0x20u,
    SSD1306_CMD_COLUMNADDR = 0x21u,
    SSD1306_CMD_PAGEADDR = 0x22u,
    SSD1306_CMD_COMSCANINC = 0xC0u,
    SSD1306_CMD_COMSCANDEC = 0xC8u,
    SSD1306_CMD_SEGREMAP = 0xA0u,
    SSD1306_CMD_CHARGEPUMP = 0x8Du,
    SSD1306_CMD_EXTERNALVCC = 0x01u,
    SSD1306_CMD_SWITCHCAPVCC = 0x02u,
    SSD1306_CMD_ACTIVATE_SCROLL = 0x2Fu,
    SSD1306_CMD_DEACTIVATE_SCROLL = 0x2Eu,
    SSD1306_CMD_SET_VERTICAL_SCROLL_AREA = 0xA3u,
    SSD1306_CMD_RIGHT_HORIZONTAL_SCROLL = 0x26u,
    SSD1306_CMD_LEFT_HORIZONTAL_SCROLL = 0x27u,
    SSD1306_CMD_VERTICAL_AND_RIGHT_HORIZONTAL_SCROLL = 0x29u,
    SSD1306_CMD_VERTICAL_AND_LEFT_HORIZONTAL_SCROLL = 0x2Au,
    SSD1306_CMD_PAGESTARTADDR = 0xB0u,
};

enum {
    SSD1306_MEM_MODE_PAGE_ADDRESSING = 0b10,
    SSD1306_MEM_MODE_HORIZONTAL_ADDRESSING = 0,
    SSD1306_MEM_MODE_VERTICAL_ADDRESSING = 0b01
};

enum {
    SSD1306_SCROLL_STOP,
    SSD1306_SCROLL_LEFT,
    SSD1306_SCROLL_RIGHT,
    SSD1306_SCROLL_DIAG_LEFT,
    SSD1306_SCROLL_DIAG_RIGHT
};

void SSD1306::init()
{
    setDisplayClockDivRatio(0, 8);
    setMultiplexRatio(SSD1306_LCDHEIGHT - 1);
    setDisplayOffset(0);
    setDisplayStartLine(0);
    setDcDcConvOn(true);
    setMemoryMode(MemoryMode::PageAddressing);
    setSegmentRemap(true);
    setComScanInverted(true);
    setComPadsAltHwConfig(0x12);
    setContrast(0x60);
    setVcomDeselectLevel(0x10);
    setInvertedDisplay(false);
    setDischargePrechargePeriod(2, 2);
}

void SSD1306::fill(const uint8_t pattern)
{
    setColumn(0);

    for (uint8_t line = 0; line < Lines; ++line)
    {
        setLine(line);

        for (uint8_t i = 0; i < SSD1306_LCDWIDTH; ++i)
            sendData(pattern);
    }
}

void SSD1306::sendCommand(uint8_t code)
{
    const uint8_t data[] = {
        SSD1306_I2C_CO_FLAG,
        code
    };

    I2C::write(SSD1306_I2C_ADDRESS, data, sizeof(data));
}

void SSD1306::sendCommand(uint8_t code, uint8_t arg)
{
    const uint8_t data[] = {
        SSD1306_I2C_CO_FLAG,
        code,
        SSD1306_I2C_CO_FLAG,
        arg
    };

    I2C::write(SSD1306_I2C_ADDRESS, data, sizeof(data));
}

void SSD1306::sendData(uint8_t data, const uint8_t bitShift, const bool invert)
{
    if (bitShift > 0)
        data <<= bitShift;

    if (invert)
        data = ~data;

    const uint8_t buf[] = {
        SSD1306_I2C_DC_FLAG,
        data
    };

    I2C::write(SSD1306_I2C_ADDRESS, buf, sizeof(buf));
}

void SSD1306::sendData(const uint8_t* data, const uint8_t length, const uint8_t bitShift, const bool invert)
{
    if (bitShift > 7)
        return;

    if (!I2C::start(SSD1306_I2C_ADDRESS, I2C::Operation::Write)) {
        return;
    }

    const uint8_t flag = SSD1306_I2C_DC_FLAG;
    if (!I2C::write(&flag, 1)) {
        return;
    }

    for (uint8_t i = 0; i < length; ++i) {
        uint8_t b = data[i] << bitShift;
        if (invert)
            b = ~b;

        if (!I2C::write(&b, 1)) {
            return;
        }
    }

    I2C::end();
}

bool SSD1306::isPoweredOn()
{
    return _poweredOn;
}

void SSD1306::setPowerOn(const bool on)
{
    _poweredOn = on;
    sendCommand(SSD1306_CMD_DISPLAYOFF | (on ? 1u : 0u));
}

void SSD1306::setComPadsAltHwConfig(const uint8_t value)
{
    sendCommand(SSD1306_CMD_SETCOMPINS, value & 0b00110010);
}

void SSD1306::setComScanInverted(const bool inverted)
{
    sendCommand(SSD1306_CMD_COMSCANINC | (inverted ? 0x08u : 0u));
}

void SSD1306::setContrast(const uint8_t value)
{
    sendCommand(SSD1306_CMD_SETCONTRAST, value);
}

void SSD1306::setDcDcConvOn(const bool on)
{
    sendCommand(SSD1306_CMD_CHARGEPUMP, on ? 0x14u : 0x10u);
}

void SSD1306::setDischargePrechargePeriod(uint8_t precharge, uint8_t discharge)
{
    sendCommand(
        SSD1306_CMD_SETPRECHARGE,
        (precharge & 0x0Fu) | (static_cast<uint8_t>(discharge << 4) & 0x0Fu)
    );
}

void SSD1306::setDisplayAllOn(const bool on)
{
    sendCommand(SSD1306_CMD_DISPLAYOFF | (on ? 1u : 0u));
}

void SSD1306::setDisplayClockDivRatio(const uint8_t div, const uint8_t ratio)
{
    sendCommand(
        SSD1306_CMD_SETDISPLAYCLOCKDIV,
        (div & 0x0Fu) | (static_cast<uint8_t>(ratio << 4) & 0xF0u)
    );
}

void SSD1306::setDisplayStartLine(const uint8_t value)
{
    sendCommand(SSD1306_CMD_SETSTARTLINE | (value & 0x3Fu));
}

void SSD1306::setDisplayOffset(const uint8_t value)
{
    sendCommand(SSD1306_CMD_SETDISPLAYOFFSET, value & 0x3Fu);
}

void SSD1306::setInvertedDisplay(const bool inverted)
{
    sendCommand(SSD1306_CMD_NORMALDISPLAY | (inverted ? 1u : 0u));
}

void SSD1306::setMemoryMode(MemoryMode mode)
{
    sendCommand(SSD1306_CMD_MEMORYMODE, static_cast<uint8_t>(mode));
}

void SSD1306::setMultiplexRatio(const uint8_t value)
{
    sendCommand(SSD1306_CMD_SETMULTIPLEX, value);
}

void SSD1306::setSegmentRemap(const bool reverse)
{
    sendCommand(SSD1306_CMD_SEGREMAP | (reverse ? 1u : 0u));
}

void SSD1306::setVcomDeselectLevel(const uint8_t value)
{
    sendCommand(SSD1306_CMD_SETVCOMDETECT, value & 0b01110000);
}

void SSD1306::setColumn(const uint8_t column)
{
    sendCommand(
        SSD1306_CMD_SETLOWCOLUMN | (column & 0x0Fu),
        SSD1306_CMD_SETHIGHCOLUMN | ((column >> 4) & 0x0Fu)
    );
}

void SSD1306::setLine(const uint8_t line)
{
    sendCommand(SSD1306_CMD_PAGESTARTADDR | (line & 0x0F));
}
