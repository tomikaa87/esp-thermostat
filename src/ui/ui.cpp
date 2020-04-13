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
#include "clock.h"
#include "main.h"

#include "display/Display.h"

#include "main_screen.h"
#include "menu_screen.h"
#include "scheduling_screen.h"

#include <iostream>
#include <string.h>
#include <stdio.h>

// #define ENABLE_DEBUG

Ui::Ui(Keypad& keypad)
    : _keypad(keypad)
{
    Display::setContrast(settings.display.brightness);

    // main_screen_init();
    // main_screen_draw();
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
            // main_screen_update();
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

    // _lastKeyPressTime = Globals::clock.utcTime();

    // If the display is sleeping, use this keypress to wake it up,
    // but don't interact with the UI while it's invisible.
    if (!Display::isPoweredOn()) {
        std::cout << "Ui::handleKeyPress: display is off, ignoring" << std::endl;
        return;
    }

    auto result = UiResult::Idle;

    switch (_screen)
    {
        case Screen::Main:
            // result = main_screen_handle_keys(keys);
            break;

        case Screen::Menu:
            result = menu_screen_handle_handle_keys(keys);
            break;

        case Screen::Scheduler:
            result = scheduling_screen_handle_keys(keys);
            break;
    }

#ifdef ENABLE_DEBUG
    printf("ui_result=%d\r\n", result);
#endif

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
            // main_screen_draw();
            break;

        case UiResult::SwitchMenuScreen:
            _screen = Screen::Menu;
            menu_screen_init();
            menu_screen_draw();
            break;

        case UiResult::SwitchSchedulingScreen:
            _screen = Screen::Scheduler;
            scheduling_screen_init();
            scheduling_screen_draw();
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
            Display::powerOn();
        }
    } else {
        if (Display::isPoweredOn()) {
            Display::powerOff();
        }
    }
}

bool Ui::isActive() const
{
    if (settings.display.timeout_secs == 0) {
        return true;
    }

    return false; //(Globals::clock.utcTime() - _lastKeyPressTime) < (std::time_t)(settings.display.timeout_secs);
}