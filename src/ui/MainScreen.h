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
    Created on 2017-01-02
*/

#pragma once

#include "Keypad.h"
#include "Logger.h"
#include "Screen.h"

#include <cstdint>

class Settings;
class SystemClock;
class HeatingController;

class MainScreen : public Screen
{
public:
    MainScreen(Settings& settings, const SystemClock& clock, HeatingController& heatingController);

    void activate() override;
    void update() override;
    Action keyPress(Keypad::Keys keys) override;

private:
    Settings& _settings;
    const SystemClock& _clock;
    HeatingController& _heatingController;
    Logger _log{ "MainScreen" };
    uint8_t _indicator = 0;
    bool _boostIndicator = false;
    uint8_t _lastScheduleIndex = 0;

    void draw();
    void drawClock();
    void drawTargetTempBoostIndicator();
    void updateScheduleBar();
    void updateModeIndicator();
    void drawTemperatureDisplay();
};
