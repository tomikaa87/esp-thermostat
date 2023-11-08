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
    Created on 2017-01-07
*/

#pragma once

#include "Keypad.h"
#include "Screen.h"
#include "Settings.h"

#include <HeatingZoneController.h>

#include <cstdint>

class ISystemClock;

class SchedulingScreen : public Screen
{
public:
    SchedulingScreen(Settings& settings, const ISystemClock& systemClock);

    void activate() override;
    void update() override;
    Action keyPress(Keypad::Keys keys) override;

private:
    Settings& _settings;
    const ISystemClock& _systemClock;

    uint8_t _day = 0;
    uint8_t _intvalIdx = 0;
    HeatingZoneController::ScheduleData _daysData[7];
    uint8_t _menuPressCnt = 0;

    void draw();
    void drawDayName();
    void drawIntervalDisplay();
    void drawIntervalIndicator();
    void updateScheduleBar();
    void setModeAndAdvance(bool daytime);
    void nextInterval();
    void prevInterval();
    void nextDay();
    void prevDay();
    void applyChanges();
};
