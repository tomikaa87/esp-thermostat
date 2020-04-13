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

#include "MainScreen.h"
#include "MenuScreen.h"
#include "SchedulingScreen.h"

#include <ctime>

class Ui
{
public:
    Ui(const SystemClock& systemClock, Keypad& keypad, HeatingController& heatingController);

    void task();

    void update();
    void handleKeyPress(Keypad::Keys keys);

private:
    const SystemClock& _systemClock;
    Keypad& _keypad;
    HeatingController& _heatingController;
    Logger _log{ "Ui" };
    std::time_t _lastKeyPressTime = 0;

    MainScreen _mainScreen;
    MenuScreen _menuScreen;
    SchedulingScreen _schedulingScreen;

    enum class Screen
    {
        Main,
        Menu,
        Scheduler
    } _screen = Screen::Main;

    void updateActiveState();
    bool isActive() const;
};
