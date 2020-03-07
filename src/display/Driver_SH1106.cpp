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

#include "Driver_SH1106.h"
#include "Wire.h"

using namespace Driver;

bool SH1106::_poweredOn = false;

enum {
    SH1106_I2C_ADDRESS = 0x3Cu,
    SH1106_I2C_DC_FLAG = 0x40u,
    SH1106_I2C_CO_FLAG = 0x80u
};

enum {
    SH1106_LCDWIDTH = 128u,
    SH1106_LCDHEIGHT = 64u,
    SH1106_LCDOFFSET = 2u,
    SH1106_PAGE_COUNT = 8u
};

enum {
    SH1106_CMD_SET_COL_ADDR_LOW = 0x00u,
    SH1106_CMD_SET_COL_ADDR_HIGH = 0x10u,
    SH1106_CMD_SET_CHARGE_PUMP_VOLTAGE = 0x30u,
    SH1106_CMD_SET_DISPLAY_START_LINE = 0x40u,
    SH1106_CMD_SET_CONTRAST_CONTROL = 0x81u,
    SH1106_CMD_SET_SEGMENT_REMAP = 0xA0u,
    SH1106_CMD_SET_ENTIRE_DISPLAY_OFF_ON = 0xA4u,
    SH1106_CMD_SET_NORMAL_REVERSE_DISPLAY = 0xA6u,
    SH1106_CMD_SET_MULTIPLEX_RATIO = 0xA8u,
    SH1106_CMD_SET_DC_DC_OFF_ON = 0xADu,
    SH1106_CMD_SET_DISPLAY_OFF_ON = 0xAEu,
    SH1106_CMD_SET_PAGE_ADDR = 0xB0u,
    SH1106_CMD_SET_COM_OUT_SCAN_DIR = 0xC0u,
    SH1106_CMD_SET_DISPLAY_OFFSET = 0xD3u,
    SH1106_CMD_SET_DISPLAY_CLOCK_DIV_RATIO = 0xD5u,
    SH1106_CMD_SET_PRECHARGE_PERIOD = 0xD9u,
    SH1106_CMD_SET_COM_PADS_HW_CONFIG = 0xDAu,
    SH1106_CMD_SET_VCOM_DESELECT_LEVEL = 0xDBu,
    SH1106_CMD_READ_MODIFY_WRITE = 0xE0u,
    SH1106_CMD_NOP = 0xE3u,
    SH1106_CMD_END_READ_MODIFY_WRITE = 0xEEu
};

void SH1106::init()
{
    setChargePumpVoltage(ChargePumpVoltage::_8_0V);
    setDisplayStartLine(0);
    setContrast(0x80);
    setSegmentRemap(true);
    setInvertedDisplay(false);
    setMultiplexRatio(63);
    setDcDcConvOn(true);
    setComScanInverted(true);
    setDisplayOffset(0);
    setDisplayClockDivRatio(0, 0x0F);
    setDischargePrechargePeriod(2, 2);
    setComPadsAltHwConfig(true);
    setVcomDeselectLevel(0x35);
}

void SH1106::fill(const uint8_t pattern)
{
    setColumn(0);

    for (uint8_t line = 0; line < Lines; ++line)
    {
        setLine(line);

        for (uint8_t i = 0; i < 132; ++i)
            sendData(pattern);
    }
}

void SH1106::sendCommand(uint8_t code)
{
    Wire.beginTransmission(SH1106_I2C_ADDRESS);

    Wire.write(SH1106_I2C_CO_FLAG);
    Wire.write(&code, 1);

    Wire.endTransmission();
}

void SH1106::sendCommand(uint8_t code, uint8_t arg)
{
    Wire.beginTransmission(SH1106_I2C_ADDRESS);

    Wire.write(SH1106_I2C_CO_FLAG);
    Wire.write(&code, 1);

    Wire.write(SH1106_I2C_CO_FLAG);
    Wire.write(&arg, 1);

    Wire.endTransmission();
}

void SH1106::sendData(uint8_t data, const uint8_t bitShift, const bool invert)
{
    Wire.beginTransmission(SH1106_I2C_ADDRESS);

    Wire.write(SH1106_I2C_DC_FLAG);

    if (bitShift > 0)
        data <<= bitShift;

    if (invert)
        data = ~data;

    Wire.write(&data, 1);

    Wire.endTransmission();
}

void SH1106::sendData(const uint8_t* data, const uint8_t length, const uint8_t bitShift, const bool invert)
{
    if (bitShift > 7)
        return;

    uint8_t buffer[17];
    auto bytesRemaining = length;
    uint8_t dataIndex = 0;
    static const uint8_t ChunkSize = 16;

    while (bytesRemaining > 0) {
        const auto count =
            bytesRemaining >= ChunkSize ? ChunkSize : bytesRemaining;
        bytesRemaining -= count;

        buffer[0] = SH1106_I2C_DC_FLAG;
        for (uint8_t i = 1; i <= count; ++i) {
            buffer[i] = data[dataIndex++] << bitShift;
            if (invert)
                buffer[i] = ~buffer[i];
        }

        Wire.beginTransmission(SH1106_I2C_ADDRESS);
        Wire.write(buffer, count + 1);
        Wire.endTransmission();
    }
}

bool SH1106::isPoweredOn()
{
    return _poweredOn;
}

void SH1106::setPowerOn(const bool on)
{
    _poweredOn = on;
    sendCommand(SH1106_CMD_SET_DISPLAY_OFF_ON | (on ? 1u : 0u));
}

void SH1106::setChargePumpVoltage(const ChargePumpVoltage voltage)
{
    sendCommand(SH1106_CMD_SET_CHARGE_PUMP_VOLTAGE | (static_cast<uint8_t>(voltage) & 0x03u));
}

void SH1106::setComPadsAltHwConfig(const bool alternative)
{
    sendCommand(
        SH1106_CMD_SET_COM_PADS_HW_CONFIG,
        alternative ? 0x12u : 0x02u
    );
}

void SH1106::setComScanInverted(const bool inverted)
{
    sendCommand(SH1106_CMD_SET_COM_OUT_SCAN_DIR | (inverted ? 0x08u : 0u));
}

void SH1106::setContrast(const uint8_t value)
{
    sendCommand(SH1106_CMD_SET_CONTRAST_CONTROL, value);
}

void SH1106::setDcDcConvOn(const bool on)
{
    sendCommand(SH1106_CMD_SET_DC_DC_OFF_ON, 0x8Au | (on ? 1u : 0u));
}

void SH1106::setDischargePrechargePeriod(const uint8_t precharge, const uint8_t discharge)
{
    sendCommand(
        SH1106_CMD_SET_PRECHARGE_PERIOD,
        (precharge & 0x0Fu) | (static_cast<uint8_t>(discharge << 4) & 0x0Fu)
    );
}

void SH1106::setDisplayClockDivRatio(const uint8_t div, const uint8_t ratio)
{
    sendCommand(
        SH1106_CMD_SET_DISPLAY_CLOCK_DIV_RATIO,
        (div & 0x0Fu) | (static_cast<uint8_t>(ratio << 4) & 0xF0u)
    );
}

void SH1106::setDisplayStartLine(const uint8_t value)
{
    sendCommand(SH1106_CMD_SET_DISPLAY_START_LINE | (value & 0x3Fu));
}

void SH1106::setDisplayOffset(const uint8_t value)
{
    sendCommand(SH1106_CMD_SET_DISPLAY_OFFSET, value & 0x3Fu);
}

void SH1106::setInvertedDisplay(const bool inverted)
{
    sendCommand(SH1106_CMD_SET_NORMAL_REVERSE_DISPLAY | (inverted ? 1u : 0u));
}

void SH1106::setMultiplexRatio(const uint8_t value)
{
    sendCommand(SH1106_CMD_SET_MULTIPLEX_RATIO, value & 0x3Fu);
}

void SH1106::setSegmentRemap(const bool reverse)
{
    sendCommand(SH1106_CMD_SET_SEGMENT_REMAP | (reverse ? 1u : 0u));
}

void SH1106::setVcomDeselectLevel(const uint8_t value)
{
    sendCommand(SH1106_CMD_SET_VCOM_DESELECT_LEVEL, value);
}

void SH1106::setColumn(const uint8_t column)
{
    const auto addr = column + SH1106_LCDOFFSET;

    sendCommand(
        SH1106_CMD_SET_COL_ADDR_LOW | (addr & 0x0Fu),
        SH1106_CMD_SET_COL_ADDR_HIGH | ((addr >> 4) & 0x0Fu)
    );
}

void SH1106::setLine(const uint8_t line)
{
    sendCommand(SH1106_CMD_SET_PAGE_ADDR | (line & 0x0F));
}
