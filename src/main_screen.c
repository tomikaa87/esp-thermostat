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

#include "config.h"
#include "main_screen.h"
#include "text.h"
#include "ds18x20.h"
#include "graphics.h"
#include "clock.h"
#include "heat_ctl.h"
#include "draw_helper.h"
#include "keypad.h"
#include "extras.h"

#include <stdio.h>
#include <string.h>
#include <time.h>

static struct {
	unsigned indicator: 2;
	unsigned boost_indicator: 1;
	unsigned: 0;

	uint8_t last_schedule_index;
} state;

static void draw_temperature_display()
{
	draw_temperature_value(10, ds18x20_last_reading / 100,
		(ds18x20_last_reading % 100) / 10);
}

static void update_mode_indicator()
{
	switch (heatctl_mode())
	{
	case HC_MODE_BOOST:
	case HC_MODE_NORMAL:
		if (heatctl_is_active()) {
			if (state.indicator != DH_MODE_HEATING)
				state.indicator = DH_MODE_HEATING;
			else
				return;
		} else {
			if (state.indicator != DH_NO_INDICATOR)
				state.indicator = DH_NO_INDICATOR;
			else
				return;
		}
		break;

	case HC_MODE_OFF:
		if (state.indicator != DH_MODE_OFF) {
			state.indicator = DH_MODE_OFF;
			break;
		} else {
			return;
		}
	}

	draw_mode_indicator(state.indicator);
}

static void update_schedule_bar()
{
	const struct tm* t = gmtime(&clock_epoch);

	draw_schedule_bar(settings.schedule.days[t->tm_wday]);

	uint8_t idx = calculate_schedule_intval_idx(t->tm_hour, t->tm_min);

	if (idx != state.last_schedule_index) {
		state.last_schedule_index = idx;
		draw_schedule_indicator(idx);
	}
}

static void draw_target_temp_boost_indicator()
{
	char s[15] = "";

	if (!heatctl_is_boost_active()) {
		uint16_t temp = heatctl_target_temp();
		sprintf(s, "     %2d.%d C", temp / 10, temp % 10);
	} else {
		time_t secs = heatctl_boost_remaining_secs();
		uint16_t minutes = secs / 60;
		secs -= minutes * 60;

		sprintf(s, " BST %3u:%02ld", minutes, secs);
	}

	text_draw(s, 0, 60, 0, false);
}

static void draw_clock()
{
	const struct tm* t = gmtime(&clock_epoch);

	char time_fmt[10] = { 6 };
	sprintf(time_fmt, "%02d:%02d", t->tm_hour, t->tm_min);

	text_draw(time_fmt, 0, 0, 0, false);
	draw_weekday(33, t->tm_wday);
}

void main_screen_init()
{
	state.indicator = 0;
	state.boost_indicator = 0;
}

void main_screen_draw()
{
	state.last_schedule_index = 255;

	update_schedule_bar();
	main_screen_update();
	draw_mode_indicator(state.indicator);
}

void main_screen_update()
{
	draw_temperature_display();
	draw_clock();
	update_mode_indicator();
	draw_target_temp_boost_indicator();
	update_schedule_bar();
}

ui_result main_screen_handle_keys(uint16_t keys)
{
	// 1: increase temperature (long: repeat)
	// 2: decrease temperature (long: repeat)
	// 3: menu
	// 4: boost start, extend x minutes (long: stop)
	// 5: daytime manual override -> back to automatic
	// 6: nighttime manual override -> back to automatic

	if (keys & KEY_PLUS) {
		heatctl_inc_target_temp();
	} else if (keys & KEY_MINUS) {
		heatctl_dec_target_temp();
	} else if (keys & KEY_MENU) {
		// Avoid entering the menu while exiting
		// from another screen with long press
		if (!(keys & KEY_LONG_PRESS)) {
			return UI_RESULT_SWITCH_MENU_SCREEN;
		}
	} else if (keys & KEY_BOOST) {
		if (keys & KEY_LONG_PRESS) {
			if (heatctl_is_boost_active()) {
				heatctl_deactivate_boost();
			}
		} else {
			if (!heatctl_is_boost_active())
				heatctl_activate_boost();
			else
				heatctl_extend_boost();
		}
	// } else if (keys & KEY_LEFT) {
	// 	heatctl_deactivate_boost();
	} else if (keys & KEY_RIGHT) {
		return UI_RESULT_SWITCH_SCHEDULING_SCREEN;
	}

	return UI_RESULT_UPDATE;
}