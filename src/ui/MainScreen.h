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
#include "ui_result.h"

#include <cstdint>

class SystemClock;
class HeatingController;

class MainScreen
{
public:
    MainScreen(const SystemClock& clock, HeatingController& heatingController);

    void main_screen_init();
    void main_screen_draw();
    void main_screen_update();
    UiResult main_screen_handle_keys(Keypad::Keys keys);

private:
    const SystemClock& _clock;
    HeatingController& _heatingController;
    Logger _log{ "MainScreen" };
    uint8_t _indicator = 0;
    bool _boostIndicator = false;
    uint8_t _lastScheduleIndex = 0;

    uint8_t last_schedule_index;

    void draw_clock();
    void draw_target_temp_boost_indicator();
    void update_schedule_bar();
    void update_mode_indicator();
    void draw_temperature_display();
};
