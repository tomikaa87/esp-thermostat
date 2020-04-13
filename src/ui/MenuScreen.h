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
    Created on 2017-01-08
*/

#pragma once

#include "Keypad.h"
#include "ui_result.h"
#include "Settings.h"

#include <cstdint>

class MenuScreen
{
public:
    MenuScreen();

    void menu_screen_init();
    void menu_screen_draw();
    UiResult menu_screen_handle_handle_keys(Keypad::Keys keys);

private:
    struct persistent_settings _newSettings;
    char _wifiPsw[64] = { 0 };

    enum class Page
    {
        First = 0,

        HeatCtlMode = First,
        DaytimeTemp,
        NightTimeTemp,
        TempOvershoot,
        TempUndershoot,
        BoostInterval,
        CustomTempTimeout,
        DisplayBrightness,
        DisplayTimeout,
        TempCorrection,
        WiFi,

        Last,

        // These pages cannot be accessed by normal navigation
        WIFI_PASSWORD
    } _page = Page::First;

    void draw_page_heatctl_mode();
    void draw_page_daytime_temp();
    void draw_page_nighttime_temp();
    void draw_page_temp_overshoot();
    void draw_page_temp_undershoot();
    void draw_page_boost_intval();
    void draw_page_custom_temp_timeout();
    void draw_page_display_brightness();
    void draw_page_display_timeout();
    void draw_page_temp_correction();
    void draw_page_wifi();
    void draw_page_wifi_password();
    void update_page_heatctl_mode();
    void update_page_daytime_temp();
    void update_page_nighttime_temp();
    void update_page_temp_overshoot();
    void update_page_temp_undershoot();
    void update_page_boost_intval();
    void update_page_custom_temp_timeout();
    void update_page_temp_correction();
    void update_page_display_brightness();
    void update_page_display_timeout();
    void update_page_wifi();
    void draw_page_title(const char* text);
    void next_page();
    void previous_page();
    void apply_settings();
    void revert_settings();
    void adjust_value(int8_t amount);
};
