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

#include "UIController.h"
#include "Settings.h"
#include "SystemClock.h"
#include "TemperatureSensor.h"
#include "main.h"

#include "display/Display.h"

#include <CoreApplication.h>

#include <algorithm>
#include <iostream>
#include <string.h>
#include <stdio.h>
#include <type_traits>

// #define ENABLE_DEBUG

using namespace UI;

UIController::UIController(
    CoreApplication& application,
    Settings& settings
)
    : _app{ application }
    , _settings{ settings }
    , _screens{ RegisteredScreens::constructScreens(_model, _graphics) }
    , _clockController{ _model.clock, _app.systemClock() }
{
    _log.info_P(PSTR("initializing Display, brightness: %d"), _settings.system.display.brightness);
    Display::init();
    Display::setContrast(_settings.system.display.brightness);

    _lastKeyPressTime = _app.systemClock().utcTime();

    if (loadScreen(ScreenID::Main)) {
        invokeActivate(*_currentScreen);
    }
}

void UIController::task(const uint32_t deltaMillis)
{
    _clockController.task(deltaMillis);

    const auto pressedKeys = _keypad.scan();
    handleKeyPress(pressedKeys);

    _lastUpdateMillis += deltaMillis;

    if (_lastUpdateMillis >= 500) {
        _lastUpdateMillis = 0;
        update();
    }
}

void UIController::update()
{
    // _log.debug("update");

    if (_currentScreen) {
        invokeUpdate(*_currentScreen);
    } else {
        _log.warning_P(PSTR("update: current screen is null"));
    }

    updateActiveState();
}

void UIController::handleKeyPress(const Keypad::Keys keys)
{
    if (keys == Keypad::Keys::None) {
        return;
    }

    _lastKeyPressTime = _app.systemClock().utcTime();

    // _log.info_P(PSTR("keys=%xh, _lastKeyPressTime=%ld"), keys, _lastKeyPressTime);

    // If the display is sleeping, use this keypress to wake it up,
    // but don't interact with the UI while it's invisible.
    if (!Display::isPoweredOn()) {
        _log.info_P(PSTR("display is off, ignoring key press"));
        return;
    }

    const auto screenChanged = std::visit(
        [this]<typename Result>(const Result& result) -> bool {
            if constexpr (std::is_same_v<Result, Screen::Navigate>) {
                return loadScreen(result.id);
            }

            return false;
        },
        invokeHandleKeyPress(*_currentScreen, keys)
    );

    if (screenChanged) {
        _log.debug("handleKeyPress::screenChanged");
        Display::clear();
        invokeActivate(*_currentScreen);
    }
}

void UIController::updateActiveState()
{
    if (isActive()) {
        if (!Display::isPoweredOn()) {
            _log.debug_P(PSTR("powering on the display, brightness: %d"), _settings.system.display.brightness);
            Display::powerOn();
            Display::setContrast(_settings.system.display.brightness);
        }
    } else {
        if (Display::isPoweredOn()) {
            _log.debug_P(PSTR("powering off the display"));
            Display::powerOff();
        }
    }
}

bool UIController::isActive() const
{
    if (_settings.system.display.timeoutSecs == 0) {
        return true;
    }

    return (_app.systemClock().utcTime() - _lastKeyPressTime) < static_cast<std::time_t>(_settings.system.display.timeoutSecs);
}

bool UIController::loadScreen(const int id)
{
    if (_currentScreen) {
        const auto currentScreenId{
            std::visit(
                []<typename ScreenType>(const ScreenType& s) {
                    static_assert(IsScreen<ScreenType>);
                    return s.id();
                },
                *_currentScreen
            )
        };

        if (currentScreenId == id) {
            return false;
        }
    }

    for (auto& screen : _screens) {
        if (id == getId(screen)) {
            _currentScreen = &screen;
            return true;
        }
    }

    return false;
}

int UIController::getId(RegisteredScreens::Screen& screen)
{
    return std::visit(
        []<typename ScreenType>(const ScreenType& s) {
            static_assert(IsScreen<ScreenType>);
            return s.id();
        },
        screen
    );
}

void UIController::invokeActivate(RegisteredScreens::Screen& screen)
{
    std::visit(
        []<typename ScreenType>(ScreenType& s) {
            static_assert(IsScreen<ScreenType>);
            s.activate();
        },
        screen
    );
}

void UIController::invokeUpdate(RegisteredScreens::Screen& screen)
{
    std::visit(
        []<typename ScreenType>(ScreenType& s) {
            static_assert(IsScreen<ScreenType>);
            s.update();
        },
        screen
    );
}

Screen::Result UIController::invokeHandleKeyPress(
    RegisteredScreens::Screen& screen,
    const Keypad::Keys keys
)
{
    return std::visit(
        [&]<typename ScreenType>(ScreenType& s) {
            static_assert(IsScreen<ScreenType>);
            return s.handleKeyPress(keys);
        },
        screen
    );
}
