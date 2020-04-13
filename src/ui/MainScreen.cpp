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

#include "MainScreen.h"
#include "graphics.h"
#include "SystemClock.h"
#include "HeatingController.h"
#include "draw_helper.h"
#include "Keypad.h"
#include "extras.h"

// FIXME
#include "main.h"

#include "display/Text.h"

#include <stdio.h>
#include <string.h>
#include <time.h>

#include "Peripherals.h"

MainScreen::MainScreen(const SystemClock& clock, HeatingController& heatingController)
    : _clock(clock)
    , _heatingController(heatingController)
{}

void MainScreen::main_screen_init()
{
    _indicator = 0;
    _boostIndicator = 0;
}

void MainScreen::main_screen_draw()
{
    _lastScheduleIndex = 255;

    update_schedule_bar();
    main_screen_update();
    draw_mode_indicator(static_cast<mode_indicator_t>(_indicator));
}

void MainScreen::main_screen_update()
{
    draw_temperature_display();
    draw_clock();
    update_mode_indicator();
    draw_target_temp_boost_indicator();
    update_schedule_bar();
}

UiResult MainScreen::main_screen_handle_keys(Keypad::Keys keys)
{
    // 1: increase temperature (long: repeat)
    // 2: decrease temperature (long: repeat)
    // 3: menu
    // 4: boost start, extend x minutes (long: stop)
    // 5: daytime manual override -> back to automatic
    // 6: nighttime manual override -> back to automatic

    if (keys & Keypad::Keys::Plus) {
        _heatingController.incTargetTemp();
    } else if (keys & Keypad::Keys::Minus) {
        _heatingController.decTargetTemp();
    } else if (keys & Keypad::Keys::Menu) {
        // Avoid entering the menu while exiting
        // from another screen with long press
        if (!(keys & Keypad::Keys::LongPress)) {
            return UiResult::SwitchMenuScreen;
        }
    } else if (keys & Keypad::Keys::Boost) {
        if (keys & Keypad::Keys::LongPress) {
            if (_heatingController.isBoostActive()) {
                _heatingController.deactivateBoost();
            }
        } else {
            if (!_heatingController.isBoostActive())
                _heatingController.activateBoost();
            else
                _heatingController.extendBoost();
        }
    // } else if (keys & KEY_LEFT) {
    // 	heatctl_deactivate_boost();
    } else if (keys & Keypad::Keys::Right) {
        return UiResult::SwitchSchedulingScreen;
    }

    return UiResult::Update;
}

void MainScreen::draw_clock()
{
    const auto localTime = _clock.localTime();
    const struct tm* t = gmtime(&localTime);

    char time_fmt[10] = { 6 };
    sprintf(time_fmt, "%02d:%02d", t->tm_hour, t->tm_min);

    Text::draw(time_fmt, 0, 0, 0, false);
    draw_weekday(33, t->tm_wday);
}

void MainScreen::draw_target_temp_boost_indicator()
{
    char s[15] = "";

    if (!_heatingController.isBoostActive()) {
        uint16_t temp = _heatingController.targetTemp();
        sprintf(s, "     %2d.%d C", temp / 10, temp % 10);
    } else {
        time_t secs = _heatingController.boostRemaining();
        uint16_t minutes = secs / 60;
        secs -= minutes * 60;

        sprintf(s, " BST %3u:%02ld", minutes, secs);
    }

    Text::draw(s, 0, 60, 0, false);
}

void MainScreen::update_schedule_bar()
{
    const auto localTime = _clock.localTime();
    const struct tm* t = gmtime(&localTime);

    draw_schedule_bar(settings.schedule.days[t->tm_wday]);

    uint8_t idx = calculate_schedule_intval_idx(t->tm_hour, t->tm_min);

    if (idx != _lastScheduleIndex) {
        _lastScheduleIndex = idx;
        draw_schedule_indicator(idx);
    }
}

void MainScreen::update_mode_indicator()
{
    switch (_heatingController.mode())
    {
    case HeatingController::Mode::Boost:
    case HeatingController::Mode::Normal:
        if (_heatingController.isActive()) {
            if (_indicator != DH_MODE_HEATING)
                _indicator = DH_MODE_HEATING;
            else
                return;
        } else {
            if (_indicator != DH_NO_INDICATOR)
                _indicator = DH_NO_INDICATOR;
            else
                return;
        }
        break;

    case HeatingController::Mode::Off:
        if (_indicator != DH_MODE_OFF) {
            _indicator = DH_MODE_OFF;
            break;
        } else {
            return;
        }
    }

    draw_mode_indicator(static_cast<mode_indicator_t>(_indicator));
}

void MainScreen::draw_temperature_display()
{
    const auto reading = Peripherals::Sensors::MainTemperature::lastReading();
    draw_temperature_value(10, reading / 100,
        (reading % 100) / 10);
}
