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
#include "ui_result.h"
#include "Settings.h"

#include <cstdint>

class Clock;

class SchedulingScreen
{
public:
    SchedulingScreen(const Clock& clock);

    void scheduling_screen_init();
    void scheduling_screen_draw();
    UiResult scheduling_screen_handle_keys(Keypad::Keys keys);

private:
    const Clock& _clock;

    uint8_t _day = 0;
    uint8_t _intval_idx = 0;
    schedule_day_data _days_data[7];
    uint8_t _menu_press_cnt = 0;

    void draw_day_name();
    void draw_interval_display();
    void draw_interval_indicator();
    void update_schedule_bar();
    void set_mode_and_advance(bool daytime);
    void next_interval();
    void prev_interval();
    void next_day();
    void prev_day();
    void apply_changes();
};
