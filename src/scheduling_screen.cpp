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

#include "config.h"
#include "scheduling_screen.h"
#include "keypad.h"
#include "text.h"
#include "graphics.h"
#include "settings.h"
#include "draw_helper.h"
#include "clock.h"

#ifndef CONFIG_USE_OLED_SH1106
#include "ssd1306.h"
#else
#include "sh1106.h"
#endif

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

static struct {
    uint8_t day;
    uint8_t intval_idx;
    schedule_day_data days_data[7];
    uint8_t menu_press_cnt;
} sch_screen;

static void draw_day_name();
static void draw_interval_display();
static void draw_interval_indicator();
static void update_schedule_bar();
static void set_mode_and_advance(bool daytime);
static void next_interval();
static void prev_interval();
static void next_day();
static void prev_day();
static void apply_changes();
static void update_day();

void scheduling_screen_init()
{
    struct tm* t = gmtime(&clock_epoch);
    sch_screen.day = t->tm_wday;
    sch_screen.intval_idx = 0;
    memcpy(sch_screen.days_data, settings.schedule.days, sizeof(schedule_day_data) * 7);
}

void scheduling_screen_draw()
{
#ifndef CONFIG_USE_OLED_SH1106
    ssd1306_clear();
#else
    sh1106_clear();
#endif

    draw_day_name();
    draw_interval_display();
    draw_interval_indicator();
    update_schedule_bar();
}

UiResult scheduling_screen_handle_keys(const Keypad::Keys keys)
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
        if (++sch_screen.menu_press_cnt == 2) {
            sch_screen.menu_press_cnt = 0;
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

static void draw_day_name()
{
    draw_weekday(0, sch_screen.day);
}

static void draw_interval_display()
{
    uint8_t hours = sch_screen.intval_idx >> 1;
    uint8_t mins = (sch_screen.intval_idx & 1) * 30;
    
    char s[6] = { 0 };
    sprintf(s, "%02u %02u", hours, mins);
    
    text_draw_7seg_large(s, 2, 29);
}

static void draw_interval_indicator()
{
    draw_schedule_indicator(sch_screen.intval_idx);
}

static void update_schedule_bar()
{
    draw_schedule_bar(sch_screen.days_data[sch_screen.day]);
}

static void set_mode_and_advance(bool daytime)
{
    uint8_t bit_idx = sch_screen.intval_idx & 0b111;
    uint8_t byte_idx = sch_screen.intval_idx >> 3;
    uint8_t mask = 1 << bit_idx;

    if (daytime)
        sch_screen.days_data[sch_screen.day][byte_idx] |= mask;
    else
        sch_screen.days_data[sch_screen.day][byte_idx] &= ~mask;

    ++sch_screen.intval_idx;
    if (sch_screen.intval_idx > 47)
        sch_screen.intval_idx = 0;

    draw_interval_indicator();
    draw_interval_display();
    update_schedule_bar();
}

static void next_interval()
{
    if (sch_screen.intval_idx < 47) {
        ++sch_screen.intval_idx;
        draw_interval_display();
        draw_interval_indicator();
    }
}

static void prev_interval()
{
    if (sch_screen.intval_idx > 0) {
        --sch_screen.intval_idx;
        draw_interval_display();
        draw_interval_indicator();
    }
}

static void next_day()
{
    ++sch_screen.day;
    if (sch_screen.day > 6)
        sch_screen.day = 0;

    sch_screen.intval_idx = 0;
    scheduling_screen_draw();
}

static void prev_day()
{
    --sch_screen.day;
    if (sch_screen.day > 6)
        sch_screen.day = 6;

    sch_screen.intval_idx = 0;
    scheduling_screen_draw();
}

static void apply_changes()
{
    memcpy(settings.schedule.days, sch_screen.days_data, sizeof(schedule_day_data) * 7);
    settings_save();
}