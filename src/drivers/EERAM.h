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

#pragma once

#include "Logger.h"

#include <cstdint>

namespace Drivers
{

class EERAM
{
public:
    static constexpr auto A1 = 0;
    static constexpr auto A2 = 0;

    enum class Register : uint8_t
    {
        Status = 0,
        Command = 0x55
    };

    enum class Command : uint8_t
    {
        SoftwareStore = 0b00110011,
        SoftwareRecall = 0b11011101
    };

    union StatusReg
    {
        struct
        {
            uint8_t event : 1;
            uint8_t ase : 1;
            uint8_t bp : 3;
            uint8_t _reserved : 2;
            uint8_t am : 1;
        } __attribute__((packed));
        uint8_t value;
    } __attribute__((packed));

    EERAM() = delete;

    static bool read(uint16_t address, uint8_t* buffer, uint16_t length);
    static bool write(uint16_t address, const uint8_t* data, uint16_t length);

    static void writeControlReg(Register reg, uint8_t value);

    static StatusReg getStatus();
    static void setStatus(StatusReg sr);

    static void setAseEnabled(bool enabled);

    static void executeCommand(Command cmd);

private:
    static Logger _log;
};

}