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

#include "SchedulingScreen.h"
#include "Keypad.h"
#include "graphics.h"
#include "settings.h"
#include "draw_helper.h"
#include "SystemClock.h"
#include "main.h"

#include "display/Display.h"
#include "display/Text.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

SchedulingScreen::SchedulingScreen(const SystemClock& systemClock)
    : _systemClock(systemClock)
{
    memset(_days_data, 0, sizeof(_days_data));
}

void SchedulingScreen::scheduling_screen_init()
{
    const auto localTime = _systemClock.localTime();
    struct tm* t = gmtime(&localTime);
    _day = t->tm_wday;
    _intval_idx = 0;
    memcpy(_days_data, settings.schedule.days, sizeof(schedule_day_data) * 7);
}

void SchedulingScreen::scheduling_screen_draw()
{
    Display::clear();

    draw_day_name();
    draw_interval_display();
    draw_interval_indicator();
    update_schedule_bar();
}

UiResult SchedulingScreen::scheduling_screen_handle_keys(const Keypad::Keys keys)
{
    // 1: advance 15 minutes (long: go back 15 minutes)
    // 2: advance 1 day (long: go back 1 day)
    // 3: save and exit
    // 4: cancel
    // 5: set nighttime mode + advance 15 minutes
    // 6: set daytime mode + advance 15 minutes
    
    if (keys & Keypad::Keys::Plus) {
        set_mode_and_advance(true);
        // if (keys & KEY_LONG_PRESS)
        // 	prev_interval();
        // else
        // 	next_interval();
    } else if (keys & Keypad::Keys::Minus) {
        set_mode_and_advance(false);
        // if (keys & KEY_LONG_PRESS)
        // 	prev_day();
        // else
        // 	next_day();
    } else if (keys & Keypad::Keys::Menu) {
        if (++_menu_press_cnt == 2) {
            _menu_press_cnt = 0;
            if (!(keys & Keypad::Keys::LongPress)) {
                apply_changes();
            }	
            return UiResult::SwitchMainScreen;
        }
    } else if (keys & Keypad::Keys::Boost) {
        next_day();
        // return UI_RESULT_SWITCH_MAIN_SCREEN;
    } else if (keys & Keypad::Keys::Left) {
        prev_interval();
        // set_mode_and_advance(false);
    } else if (keys & Keypad::Keys::Right) {
        next_interval();
        // set_mode_and_advance(true);
    }

    return UiResult::Idle;
}

void SchedulingScreen::draw_day_name()
{
    draw_weekday(0, _day);
}

void SchedulingScreen::draw_interval_display()
{
    uint8_t hours = _intval_idx >> 1;
    uint8_t mins = (_intval_idx & 1) * 30;

    char s[6] = { 0 };
    sprintf(s, "%02u %02u", hours, mins);

    Text::draw7Seg(s, 2, 29);
}

void SchedulingScreen::draw_interval_indicator()
{
    draw_schedule_indicator(_intval_idx);
}

void SchedulingScreen::update_schedule_bar()
{
    draw_schedule_bar(_days_data[_day]);
}

void SchedulingScreen::set_mode_and_advance(bool daytime)
{
    uint8_t bit_idx = _intval_idx & 0b111;
    uint8_t byte_idx = _intval_idx >> 3;
    uint8_t mask = 1 << bit_idx;

    if (daytime)
        _days_data[_day][byte_idx] |= mask;
    else
        _days_data[_day][byte_idx] &= ~mask;

    ++_intval_idx;
    if (_intval_idx > 47)
        _intval_idx = 0;

    draw_interval_indicator();
    draw_interval_display();
    update_schedule_bar();
}

void SchedulingScreen::next_interval()
{
    if (_intval_idx < 47) {
        ++_intval_idx;
        draw_interval_display();
        draw_interval_indicator();
    }
}

void SchedulingScreen::prev_interval()
{
    if (_intval_idx > 0) {
        --_intval_idx;
        draw_interval_display();
        draw_interval_indicator();
    }
}

void SchedulingScreen::next_day()
{
    ++_day;
    if (_day > 6)
        _day = 0;

    _intval_idx = 0;
    scheduling_screen_draw();
}

void SchedulingScreen::prev_day()
{
    --_day;
    if (_day > 6)
        _day = 6;

    _intval_idx = 0;
    scheduling_screen_draw();
}

void SchedulingScreen::apply_changes()
{
    memcpy(settings.schedule.days, _days_data, sizeof(schedule_day_data) * 7);
    settings_save();
}