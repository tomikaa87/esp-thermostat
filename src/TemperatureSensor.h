/*
    This file is part of esp-iot-base.

    esp-iot-base is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    esp-iot-base is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with esp-iot-base.  If not, see <http://www.gnu.org/licenses/>.

    Author: Tamas Karpati
    Created on 2020-10-05
*/

#pragma once

#include <cstdint>

class Settings;

class TemperatureSensor
{
public:
    static constexpr auto UpdateIntervalMs = 2500;

    explicit TemperatureSensor(const Settings& settings);

    void task();

    int16_t read() const;

private:
    const Settings& _settings;
    uint32_t _lastUpdate = 0;
};