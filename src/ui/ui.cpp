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

#include "ui.h"
#include "Settings.h"
#include "SystemClock.h"
#include "main.h"

#include "display/Display.h"

#include "MainScreen.h"
#include "MenuScreen.h"
#include "SchedulingScreen.h"

#include <algorithm>
#include <iostream>
#include <string.h>
#include <stdio.h>

// #define ENABLE_DEBUG

// TODO rename settings_
Ui::Ui(Settings& settings_, const SystemClock& systemClock, Keypad& keypad, HeatingController& heatingController)
    : _settings(settings_)
    , _systemClock(systemClock)
    , _keypad(keypad)
    , _heatingController(heatingController)
{
    Display::setContrast(_settings.Data.Display.Brightness);

    auto mainScreen = std::unique_ptr<MainScreen>(new MainScreen(_settings, _systemClock, _heatingController));
    _mainScreen = mainScreen.get();
    _currentScreen = _mainScreen;
    mainScreen->activate();
    _screens.push_back(std::move(mainScreen));

    _screens.emplace_back(new MenuScreen(_settings));
    _screens.emplace_back(new SchedulingScreen(_settings, _systemClock));

    _lastKeyPressTime = _systemClock.utcTime();
}

void Ui::task()
{
    const auto pressedKeys = _keypad.scan();
    handleKeyPress(pressedKeys);
}

void Ui::update()
{
    if (_currentScreen) {
        _currentScreen->update();
    } else {
        _log.warning("update: current screen is null");
    }

    updateActiveState();
}

void Ui::handleKeyPress(const Keypad::Keys keys)
{
    if (keys == Keypad::Keys::None)
        return;

    _lastKeyPressTime = _systemClock.utcTime();

    _log.info("keys=%xh, _lastKeyPressTime=%ld", keys, _lastKeyPressTime);

    // If the display is sleeping, use this keypress to wake it up,
    // but don't interact with the UI while it's invisible.
    if (!Display::isPoweredOn()) {
        _log.info("display is off, ignoring key press");
        return;
    }

    const auto action = _currentScreen->keyPress(keys);
    bool screenChanged = true;

    switch (action) {
        case Screen::Action::NoAction:
            screenChanged = false;
            break;

        case Screen::Action::NavigateBack:
            navigateBackward();
            break;

        case Screen::Action::NavigateForward:
            navigateForward(_currentScreen->nextScreen());
            break;
    }

    if (screenChanged) {
        Display::clear();
        _currentScreen->activate();
    }
}

void Ui::updateActiveState()
{
    if (isActive()) {
        if (!Display::isPoweredOn()) {
            _log.debug("powering on the display");
            Display::powerOn();
        }
    } else {
        if (Display::isPoweredOn()) {
            _log.debug("powering off the display");
            Display::powerOff();
        }
    }
}

bool Ui::isActive() const
{
    if (_settings.Data.Display.TimeoutSecs == 0) {
        return true;
    }

    return (_systemClock.utcTime() - _lastKeyPressTime) < static_cast<std::time_t>(_settings.Data.Display.TimeoutSecs);
}

void Ui::navigateForward(const char* name)
{
    if (!name) {
        _log.warning("navigating forward, screen name is null, going to main screen");
        _currentScreen = _mainScreen;
        return;
    }

    _log.debug("navigating forward, next screen: %s", name);

    const auto it = std::find_if(std::begin(_screens), std::end(_screens), [name](const std::unique_ptr<Screen>& scr) {
        return strcmp(scr->name(), name) == 0;
    });

    if (it == std::end(_screens)) {
        _log.warning("screen not found: %s, going to main screen");
        _currentScreen = _mainScreen;
        return;
    }

    _currentScreen = it->get();
}

void Ui::navigateBackward()
{
    _log.debug("navigating back");

    if (_screenStack.empty()) {
        _log.debug("screen stack is empty, navigating to main screen");
        _currentScreen = _mainScreen;
        return;
    }

    _currentScreen = _screenStack.top();
    _screenStack.pop();

    _log.debug("current screen: %s", _currentScreen->name());
}
