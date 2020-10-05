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

#include "HeatingController.h"
#include "Keypad.h"
#include "Logger.h"

#include "Screen.h"
#include "MainScreen.h"
#include "MenuScreen.h"
#include "SchedulingScreen.h"

#include <ctime>
#include <memory>
#include <stack>

class ISystemClock;
class Settings;
class TemperatureSensor;

class Ui
{
public:
    Ui(
        Settings& settings,
        const ISystemClock& systemClock,
        Keypad& keypad,
        HeatingController& heatingController,
        const TemperatureSensor& temperatureSensor
    );

    void task();

    void update();
    void handleKeyPress(Keypad::Keys keys);

private:
    Settings& _settings;
    const ISystemClock& _systemClock;
    Keypad& _keypad;
    HeatingController& _heatingController;
    const TemperatureSensor& _temperatureSensor;
    Logger _log{ "Ui" };
    std::time_t _lastKeyPressTime = 0;

    std::stack<Screen*> _screenStack;
    std::vector<std::unique_ptr<Screen>> _screens;

    Screen* _currentScreen = nullptr;
    Screen* _mainScreen = nullptr;

    void updateActiveState();
    bool isActive() const;

    void navigateForward(const char* name);
    void navigateBackward();
};
