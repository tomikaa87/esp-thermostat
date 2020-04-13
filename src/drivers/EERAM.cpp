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
#include "drivers/SimpleI2C.h"

using namespace Drivers;

Logger EERAM::_log = Logger{ "EERAM" };

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

bool EERAM::read(const uint16_t address, uint8_t* const buffer, const uint16_t length)
{
    const uint8_t addrBuf[] = {
        static_cast<uint8_t>(address >> 8),
        static_cast<uint8_t>(address & 0xff)
    };

    if (!I2C::write(ControlBytes::SramAccess, addrBuf, sizeof(addrBuf))) {
        _log.error("read error: cannot start transfer");
        return false;
    }

    if (!I2C::read(ControlBytes::SramAccess, buffer, length)) {
        _log.error("read error: cannot read data");
        return false;
    }

    return true;
}

bool EERAM::write(uint16_t address, const uint8_t* data, uint16_t length)
{
    if (!I2C::start(ControlBytes::SramAccess, I2C::Operation::Write)) {
        _log.error("write error: cannot start transfer");
        return false;
    }

    uint8_t addrBuf[] = {
        static_cast<uint8_t>(address >> 8),
        static_cast<uint8_t>(address & 0xff)
    };

    if (!I2C::write(addrBuf, sizeof(addrBuf))) {
        _log.error("write error: cannot write register address");
        return false;
    }

    if (!I2C::write(data, length)) {
        _log.error("write error: cannot write data");
        return false;
    }

    I2C::end();

    return true;
}

void EERAM::writeControlReg(Register reg, uint8_t value)
{
    if (!I2C::start(ControlBytes::ControlRegAccess, I2C::Operation::Write)) {
        _log.error("control register write error: cannot start transfer");
        return;
    }

    if (!I2C::write(reinterpret_cast<const uint8_t*>(&reg), 1)) {
        _log.error("control register write error: cannot write register address");
        return;
    }

    if (!I2C::write(&value, 1)) {
        _log.error("control register write error: cannot write value");
        return;
    }

    I2C::end();

    // T(recall) for 47X16
    delay(25);
}

EERAM::StatusReg EERAM::getStatus()
{
    StatusReg sr;
    if (!I2C::read(ControlBytes::ControlRegAccess, reinterpret_cast<uint8_t*>(&sr.value), sizeof(sr))) {
        _log.error("get status error: cannot read control register");
    }

    return sr;
}

void EERAM::setStatus(StatusReg sr)
{
    writeControlReg(Register::Status, sr.value);
}

void EERAM::setAseEnabled(const bool enabled)
{
    _log.debug("setting ASE to %s", enabled ? "enabled" : "disabled");

    StatusReg sr;
    sr.value = 0;
    sr.ase = enabled ? 1 : 0;
    setStatus(sr);
}

void EERAM::executeCommand(Command cmd)
{
    writeControlReg(Register::Command, static_cast<uint8_t>(cmd));
}