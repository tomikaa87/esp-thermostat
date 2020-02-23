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

#include "drivers/DS18B20.h"
#include "drivers/EERAM.h"

#include <Arduino.h>

namespace Peripherals
{
    namespace Sensors
    {
        using MainTemperature = Drivers::DS18B20;
    }

    namespace Clock
    {

    }

    namespace Relay
    {

    }

    namespace Storage
    {
        using EERAM = Drivers::EERAM;
    }
}
