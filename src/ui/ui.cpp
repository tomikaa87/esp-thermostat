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
#include "settings.h"
#include "SystemClock.h"
#include "main.h"

#include "display/Display.h"

#include "MainScreen.h"
#include "MenuScreen.h"
#include "SchedulingScreen.h"

#include <iostream>
#include <string.h>
#include <stdio.h>

// #define ENABLE_DEBUG

Ui::Ui(const SystemClock& systemClock, Keypad& keypad, HeatingController& heatingController)
    : _systemClock(systemClock)
    , _keypad(keypad)
    , _heatingController(heatingController)
    , _mainScreen(systemClock, heatingController)
    , _schedulingScreen(systemClock)
{
    Display::setContrast(settings.display.brightness);

    _mainScreen.main_screen_init();
    _mainScreen.main_screen_draw();
}

void Ui::task()
{
    const auto pressedKeys = _keypad.scan();
    handleKeyPress(pressedKeys);
}

void Ui::update()
{
    switch (_screen)
    {
        case Screen::Main:
            _mainScreen.main_screen_update();
            break;

        case Screen::Menu:
            break;

        case Screen::Scheduler:
            break;
    }

    updateActiveState();
}

void Ui::handleKeyPress(const Keypad::Keys keys)
{
    if (keys == Keypad::Keys::None)
        return;

    _lastKeyPressTime = _systemClock.utcTime();

    _log.info("keys=%xh, _lastKeyPressTime=%ld, _screen=%d", keys, _lastKeyPressTime, _screen);

    // If the display is sleeping, use this keypress to wake it up,
    // but don't interact with the UI while it's invisible.
    if (!Display::isPoweredOn()) {
        _log.info("display is off, ignoring key press");
        return;
    }

    auto result = UiResult::Idle;

    switch (_screen)
    {
        case Screen::Main:
            result = _mainScreen.main_screen_handle_keys(keys);
            break;

        case Screen::Menu:
            result = _menuScreen.menu_screen_handle_handle_keys(keys);
            break;

        case Screen::Scheduler:
            result = _schedulingScreen.scheduling_screen_handle_keys(keys);
            break;
    }

    if (result == UiResult::Idle)
        return;

    if (result == UiResult::Update) {
        update();
        return;
    }

    // Clear the screen before switching
    Display::clear();

    switch (result)
    {
        case UiResult::SwitchMainScreen:
            _screen = Screen::Main;
            _mainScreen.main_screen_init();
            _mainScreen.main_screen_draw();
            break;

        case UiResult::SwitchMenuScreen:
            _screen = Screen::Menu;
            _menuScreen.menu_screen_init();
            _menuScreen.menu_screen_draw();
            break;

        case UiResult::SwitchSchedulingScreen:
            _screen = Screen::Scheduler;
            _schedulingScreen.scheduling_screen_init();
            _schedulingScreen.scheduling_screen_draw();
            break;

        default:
            break;
    }

#ifdef ENABLE_DEBUG
    printf("ui.screen=%d\r\n", ui.screen);
#endif
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
    if (settings.display.timeout_secs == 0) {
        return true;
    }

    return (_systemClock.utcTime() - _lastKeyPressTime) < (std::time_t)(settings.display.timeout_secs);
}