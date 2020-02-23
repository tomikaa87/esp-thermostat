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

#include <Wire.h>

using namespace Drivers;

namespace ControlBytes
{
    namespace Detail
    {
        static constexpr auto ChipSelect = (EERAM::A1 << 2) | (EERAM::A2 << 3);
    }

    static constexpr auto SramRead          = 0b01010001 | Detail::ChipSelect;
    static constexpr auto SramWrite         = 0b01010000 | Detail::ChipSelect;
    static constexpr auto ControlRegRead    = 0b00110001 | Detail::ChipSelect;
    static constexpr auto ControlRegWrite   = 0b00110000 | Detail::ChipSelect;
}

uint8_t EERAM::read(uint16_t address, uint8_t* const buffer, uint16_t length)
{
    Wire.beginTransmission(ControlBytes::SramWrite);
    Wire.write(address);
    Wire.endTransmission();

    const auto available = Wire.requestFrom(ControlBytes::SramRead, length);
    for (auto i = 0; i < available; ++i) {
        buffer[i] = Wire.read();
    }

    return available;
}

void EERAM::write(uint16_t address, const uint8_t* data, uint16_t length)
{
    Wire.beginTransmission(ControlBytes::SramWrite);
    Wire.write(address);
    Wire.write(data, length);
    Wire.endTransmission();
}

uint8_t EERAM::readControlReg(Register reg)
{
    Wire.beginTransmission(ControlBytes::ControlRegWrite);
    Wire.write(static_cast<int>(reg));
    Wire.endTransmission();
    
    const auto available = Wire.requestFrom(ControlBytes::ControlRegRead, 1);
    if (available == 0)
        return 0;

    return Wire.read();
}

void EERAM::writeControlReg(Register reg, uint8_t value)
{
    Wire.beginTransmission(ControlBytes::ControlRegWrite);
    Wire.write(static_cast<int>(reg));
    Wire.write(value);
    Wire.endTransmission();
}

EERAM::StatusReg EERAM::getStatus()
{
    StatusReg sr;
    sr.value = readControlReg(Register::Status);
    return sr;
}

void EERAM::setStatus(StatusReg sr)
{
    writeControlReg(Register::Status, sr.value);
}

void EERAM::executeCommand(Command cmd)
{
    writeControlReg(Register::Command, static_cast<uint8_t>(cmd));
}