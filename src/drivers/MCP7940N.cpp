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
    Created on 2020-02-24
*/

#include "MCP7940N.h"
#include "drivers/SimpleI2C.h"

using namespace Drivers;

Logger MCP7940N::_log = Logger{ "MCP7940N" };

bool MCP7940N::isOscillatorRunning()
{
    uint8_t value = read(Register::RTCWKDAY);
    return (value & (1 << 5)) > 0;
}

void MCP7940N::setOscillatorEnabled(bool enabled)
{
    uint8_t value = read(Register::RTCSEC);
    value = enabled ? value | (1 << 7) : value & ~(1 << 7);
    write(Register::RTCSEC, value);
}

bool MCP7940N::isOscillatorEnabled()
{
    uint8_t value = read(Register::RTCSEC);
    return (value & (1 << 7)) > 0;
}

void MCP7940N::setExternalOscillatorEnabled(bool enabled)
{
    uint8_t value = read(Register::CONTROL);
    value = enabled ? value | (1 << 3) : value & ~(1 << 3);
    write(Register::CONTROL, value);
}

void MCP7940N::clearPowerFailFlag()
{
    uint8_t value = read(Register::RTCWKDAY);
    value &= ~(1 << 4);
    write(Register::RTCWKDAY, value);
}

bool MCP7940N::getPowerFailFlag()
{
    uint8_t value = read(Register::RTCWKDAY);
    return (value & (1 << 4)) > 0;
}

void MCP7940N::setBatteryEnabled(const bool enabled)
{
    uint8_t value = read(Register::RTCWKDAY);
    value = enabled ? value | (1 << 3) : value & ~(1 << 3);
    write(Register::RTCWKDAY, value);
}

bool MCP7940N::isBatteryEnabled()
{
    uint8_t value = read(Register::RTCWKDAY);
    return (value & (1 << 3)) > 0;
}

void MCP7940N::set12HoursEnabled(bool enabled)
{

}

bool MCP7940N::is12HoursEnabled()
{

}

void MCP7940N::setDateTime(const DateTime& dt)
{
    setOscillatorEnabled(false);

    uint8_t data[7] = { 0 };
    if (read(Register::RTCSEC, data, sizeof(data)) != sizeof(data)) {
        setOscillatorEnabled(true);
        return;
    }

    // RTSEC, keep ST bit
    data[0] = (data[0] & (1 << 7)) | (toBcd(dt.seconds) & 0x7f);

    // RTCMIN
    data[1] = toBcd(dt.minutes) & 0x7f;
    
    // RTCHOUR
    if (dt.is12Hours) {
        data[2] = (1 << 6) | (dt.pm ? 1 << 5 : 0) | (toBcd(dt.hours) & 0x1f);
    } else {
        data[2] = toBcd(dt.hours) & 0x3f;
    }

    // RTCWKDAY, keep PWRFAIL and VBATEN bits
    data[3] = (data[3] & (0b11 << 3)) | (toBcd(dt.weekday) & 0x07);

    // RTCDATE
    data[4] = toBcd(dt.date) & 0x3f;

    // RTCMTH
    data[5] = toBcd(dt.month) & 0x1f;

    // RTCYEAR
    data[6] = toBcd(dt.year);

    write(Register::RTCSEC, data, sizeof(data));

    setOscillatorEnabled(true);
}

MCP7940N::DateTime MCP7940N::getDateTime()
{
    uint8_t data[7] = { 0 };
    if (read(Register::RTCSEC, data, sizeof(data)) != sizeof(data)) {
        setOscillatorEnabled(true);
        return{};
    }

    DateTime dt;
    dt.seconds = fromBcd(data[0] & 0x7f);
    dt.minutes = fromBcd(data[1] & 0x7f);

    if (data[2] & (1 << 6)) {
        // 12-hours mode
        dt.is12Hours = true;
        dt.pm = (data[2] & (1 << 5)) > 0;
        dt.hours = fromBcd(data[2] & 0x1f);
    } else {
        dt.is12Hours = false;
        dt.pm = false;
        dt.hours = fromBcd(data[2] & 0x3f);
    }

    dt.weekday = data[3] & 0x07;
    dt.date = fromBcd(data[4] & 0x3f);
    dt.month = fromBcd(data[5] & 0x1f);
    dt.leapYear = (data[5] & (1 << 5)) > 0;
    dt.year = fromBcd(data[6]);

    return dt;
}

void MCP7940N::setAlarm(AlarmModule module, const Alarm& alarm)
{

}

MCP7940N::Alarm MCP7940N::getAlarm(AlarmModule module)
{

}

void MCP7940N::setOutputConfig(OutputConfig config)
{
    uint8_t value = read(Register::CONTROL);
    value &= ~(0b111 << 4);
    value |= static_cast<uint8_t>(config) << 4;
    write(Register::CONTROL, value);
}

MCP7940N::OutputConfig MCP7940N::getOutputConfig()
{
    uint8_t value = read(Register::CONTROL);
    return static_cast<OutputConfig>((value >> 4) & 0b111);
}

void MCP7940N::setSquareWaveOutputFrequency(SquareWaveFrequency frequency)
{
    uint8_t value = read(Register::CONTROL);
    value &= ~(0b11);
    value |= static_cast<uint8_t>(frequency) & 0b11;
}

void MCP7940N::writeGpo(bool high)
{
    uint8_t value = read(Register::CONTROL);
    value = high ? value | (1 << 7) : value & ~(1 << 7);
    write(Register::CONTROL, value);
}

void MCP7940N::setDigitalTrimming(int8_t ppm)
{
    write(Register::OSCTRIM, static_cast<uint8_t>(ppm));
}

int8_t MCP7940N::getDigitalTrimming()
{
    return static_cast<int8_t>(read(Register::OSCTRIM));
}

void MCP7940N::setCoarseTrimmingEnabled(bool enabled)
{
    uint8_t value = read(Register::CONTROL);
    value = enabled ? value | (1 << 2) : value & ~(1 << 2);
    write(Register::CONTROL, value);
}

MCP7940N::PowerFailTimestamp MCP7940N::getPowerDownTimestamp()
{

}

MCP7940N::PowerFailTimestamp MCP7940N::getPowerUpTimestamp()
{

}

bool MCP7940N::writeSram(uint8_t address, uint8_t value)
{

}

uint8_t MCP7940N::writeSram(uint8_t address, const uint8_t* buffer, uint8_t length)
{

}

uint8_t MCP7940N::readSram(uint8_t address)
{

}

uint8_t MCP7940N::readSram(uint8_t address, uint8_t* buffer, uint8_t length)
{

}

uint8_t MCP7940N::fromBcd(uint8_t value)
{
    return (value & 0x0F) + 10 * (value >> 4);
}

uint8_t MCP7940N::toBcd(uint8_t value)
{
    return value % 10 | (value / 10) << 4;
}

uint8_t MCP7940N::write(uint8_t address, const uint8_t* buffer, uint8_t length)
{
    {
        const auto block = _log.debugBlock("write(%02xh,%ph,%u):", address, buffer, length);
        for (auto i = 0; i < length; ++i) {
            _log.debug(" %02xh", buffer[i]);
        }
    }

    if (!I2C::start(ControlByte, I2C::Operation::Write)) {
        _log.error("write error: cannot start transfer");
        return 0;
    }

    if (!I2C::write(&address, 1)) {
        _log.error("write error: cannot write address");
        return 0;
    }

    if (!I2C::write(buffer, length)) {
        _log.error("write error: cannot write data");
        return 0;
    }

    I2C::end();

    return length;
}

uint8_t MCP7940N::write(Register reg, const uint8_t* buffer, uint8_t length)
{
    return write(static_cast<uint8_t>(reg), buffer, length);
}

bool MCP7940N::write(const Register reg, uint8_t value)
{
    return write(reg, &value, 1) == 1;
}

uint8_t MCP7940N::read(uint8_t address, uint8_t* buffer, uint8_t length)
{
    _log.debug("read(%02xh,%ph,%u)", address, buffer, length);

    if (!I2C::write(ControlByte, &address, 1)) {
        _log.error("read error: cannot write address");
        return 0;
    }

    if (!I2C::read(ControlByte, buffer, length)) {
        _log.error("read error: cannot read data");
        return 0;
    }

    I2C::end();

    {
        const auto block = _log.debugBlock("read(): data:");
        for (auto i = 0; i < length; ++i) {
            _log.debug(" %02xh", buffer[i]);
        }
    }

    return length;
}

uint8_t MCP7940N::read(Register reg, uint8_t* buffer, uint8_t length)
{
    return read(static_cast<uint8_t>(reg), buffer, length);
}

uint8_t MCP7940N::read(const Register reg)
{
    uint8_t value = 0;
    return read(reg, &value, 1) == 1 ? value : 0;
}