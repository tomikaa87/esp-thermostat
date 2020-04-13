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
    Created on 2020-02-22
*/

#include "EERAM.h"

#include <Arduino.h>
#include <Wire.h>

using namespace Drivers;

namespace ControlBytes
{
    namespace Detail
    {
        static constexpr auto ChipSelect = (EERAM::A1 << 1) | (EERAM::A2 << 2);
    }

    // LSB is omitted since Wire will shift this address and add the direction bit
    static constexpr auto SramAccess        = 0b1010000 | Detail::ChipSelect;
    static constexpr auto ControlRegAccess  = 0b0011000 | Detail::ChipSelect;
}

uint8_t EERAM::read(uint16_t address, uint8_t* const buffer, uint16_t length)
{
    Wire.beginTransmission(ControlBytes::SramAccess);
    Wire.write(static_cast<uint8_t>(address >> 8));
    Wire.write(static_cast<uint8_t>(address & 0xff));
    Wire.endTransmission();

    const auto available = Wire.requestFrom(ControlBytes::SramAccess, length);

    if (available == 0) {
        Serial.println("EERAM: read failed");
        return 0;
    }

    for (auto i = 0; i < available; ++i) {
        buffer[i] = Wire.read();
    }

    return available;
}

void EERAM::write(uint16_t address, const uint8_t* data, uint16_t length)
{
    Wire.beginTransmission(ControlBytes::SramAccess);
    Wire.write(static_cast<uint8_t>(address >> 8));
    Wire.write(static_cast<uint8_t>(address & 0xff));
    Wire.write(data, length);
    Wire.endTransmission();
}

void EERAM::writeControlReg(Register reg, uint8_t value)
{
    Wire.beginTransmission(ControlBytes::ControlRegAccess);
    Wire.write(static_cast<uint8_t>(reg));
    Wire.write(value);
    Wire.endTransmission();

    // T(recall) for 47X16
    delay(25);
}

EERAM::StatusReg EERAM::getStatus()
{
    const auto available = Wire.requestFrom(ControlBytes::ControlRegAccess, 1);
    if (available == 0) {
        Serial.println("EERAM: getStatus failed");
        return{};
    }

    StatusReg sr;
    sr.value = Wire.read();

    return sr;
}

void EERAM::setStatus(StatusReg sr)
{
    writeControlReg(Register::Status, sr.value);
}

void EERAM::setAseEnabled(const bool enabled)
{
    StatusReg sr;
    sr.value = 0;
    sr.ase = enabled ? 1 : 0;
    setStatus(sr);
}

void EERAM::executeCommand(Command cmd)
{
    writeControlReg(Register::Command, static_cast<uint8_t>(cmd));
}