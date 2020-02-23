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

#pragma once

#include <Arduino.h>
#include <cstdint>

namespace Drivers
{

namespace Detail
{
    namespace OneWireImpl
    {
        uint8_t reset(int pin);

        void writeBit(int pin, uint8_t b);
        void writeByte(int pin, uint8_t b);

        uint8_t readBit(int pin);
        uint8_t readByte(int pin);

        void busLow(int pin);
        void busHigh(int pin);
        void busFloat(int pin);
        uint8_t busRead(int pin);
    }
}

template <int Pin>
class OneWire
{
public:
    OneWire() = delete;

    static uint8_t reset()
    {
        return Detail::OneWireImpl::reset(Pin);
    }

    static void writeBit(const uint8_t b)
    {
        Detail::OneWireImpl::writeBit(Pin, b);
    }

    static void writeByte(const uint8_t b)
    {
        Detail::OneWireImpl::writeByte(Pin, b);
    }

    static uint8_t readBit()
    {
        return Detail::OneWireImpl::readBit(Pin);
    }

    static uint8_t readByte()
    {
        return Detail::OneWireImpl::readByte(Pin);
    }
};

}
