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
    Created on 2017-01-04
*/

#pragma once

#include <ctime>

#include "Logger.h"
#include "Settings.h"

class ISystemClock;
class TemperatureSensor;

class HeatingController
{
public:
    HeatingController(
        Settings& settings,
        const ISystemClock& systemClock,
        const TemperatureSensor& temperatureSensor
    );

    enum class Mode
    {
        Normal,
        Boost,
        Off
    };

    enum class State
    {
        Off,
        On
    };

    struct NextTransition
    {
        State state = State::Off;
        uint8_t weekday = 0;
        uint8_t hour = 0;
        uint8_t minute = 0;
    };

    using TenthsOfDegrees = int16_t;

    void task();

    Mode mode() const;
    void setMode(Mode mode);

    bool isActive() const;

    bool isBoostActive() const;
    void activateBoost();
    void deactivateBoost();
    void extendBoost();

    std::time_t boostRemaining() const;

    TenthsOfDegrees currentTemp() const;

    TenthsOfDegrees targetTemp() const;
    void setTargetTemp(TenthsOfDegrees temp);
    void incTargetTemp();
    void decTargetTemp();

    TenthsOfDegrees daytimeTemp() const;
    void setDaytimeTemp(TenthsOfDegrees temp);

    TenthsOfDegrees nightTimeTemp() const;
    void setNightTimeTemp(TenthsOfDegrees temp);

    bool hasDaytimeSchedule() const;

    NextTransition nextTransition() const;

    State scheduledStateAt(uint8_t weekday, uint8_t hour, uint8_t min) const;

private:
    Settings& _settings;
    const ISystemClock& _systemClock;
    const TemperatureSensor& _temperatureSensor;
    Logger _log{ "HeatingController" };
    bool _boostActive = false;
    bool _boostDeactivated = false;
    bool _heatingActive = false;
    bool _usingDaytimeSchedule = false;
    bool _customTempSet = false;
    std::time_t _boostEnd = 0;
    TenthsOfDegrees _targetTemp = Limits::MinimumTemperature;
    TenthsOfDegrees _sensorTemp = 0;
    std::time_t _setTempLastChanged = 0;

    void markCustomTempSet();
    void clampTargetTemp();

    void startHeating();
    void stopHeating();

    bool isCustomTempResetNeeded() const;

    void storeTargetTemp();
    void loadStoredTargetTemp();
};
