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
    Created on 2020-01-25
*/

#include "DS18B20.h"

#include <Arduino.h>

using namespace Drivers;

int16_t DS18B20::_lastReading = 0;
Logger DS18B20::_log = Logger{ "DS18B20" };

void DS18B20::update()
{
    static bool convert = true;
    
    if (convert) {
        startConversion();
    } else {
        int16_t t = readSensor();

        // t += settings.heatctl.temp_correction * 10;
        _lastReading = t;

        _log.debug("DS18B20::update: %i", t);
    }
    
    convert = !convert;
}

int16_t DS18B20::lastReading()
{
    return _lastReading;
}

void DS18B20::startConversion()
{
    Bus::reset();
    Bus::writeByte(0xCC);
    Bus::writeByte(0x44);
}

int16_t DS18B20::readSensor()
{
    Bus::reset();
    Bus::writeByte(0xCC);
    Bus::writeByte(0xBE);

    uint8_t lsb = Bus::readByte();
    uint8_t msb = Bus::readByte();
    uint16_t value = (msb << 8) + lsb;

    if (value & 0x8000) {
        value = ~value + 1;
    }
    
    int16_t celsius = (value >> (ResolutionBits - 8)) * 100;
    uint16_t frac_part = (value << (4 - (ResolutionBits - 8))) & 0xf;
    frac_part *= 625;
    celsius += frac_part / 100;
    
    if (value & 0x8000)
        celsius *= -1;

    _log.debug("DS18B20::readSensor: %i", celsius);

    return celsius;
}