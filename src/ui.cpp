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
#include "ui.h"
#include "keypad.h"
#include "settings.h"
#include "clock.h"

#ifndef CONFIG_USE_OLED_SH1106
#include "ssd1306.h"
#else
#include "sh1106.h"
#endif

#include "main_screen.h"
#include "menu_screen.h"
#include "scheduling_screen.h"

#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

// #define ENABLE_DEBUG

static void update_display_active_state();
static bool is_ui_active();

static struct ui_s {
	enum {
		SCR_MAIN = 0,
		SCR_MENU,
		SCR_SCHEDULING
	} screen;

	time_t last_keypress_time;
} ui;

void ui_init()
{
	memset(&ui, 0, sizeof(struct ui_s));

#ifndef CONFIG_USE_OLED_SH1106
	ssd1306_init();
	ssd1306_set_brightness(settings.display.brightness);
#else
	sh1106_init();
	sh1106_set_contrast(settings.display.brightness);
#endif

	main_screen_init();
	main_screen_draw();
}

void ui_update()
{
	switch (ui.screen) {
	case ui_s::SCR_MAIN:
		main_screen_update();
		break;

	case ui_s::SCR_MENU:
		break;

	case ui_s::SCR_SCHEDULING:
		break;
	}

	update_display_active_state();
}

void ui_handle_keys(Keypad::Keys keys)
{
	static auto last_keys = Keypad::Keys::None;
	if (last_keys != keys) {
#ifdef ENABLE_DEBUG
		printf("keys: %04X\r\n", keys);
#endif
		last_keys = keys;
	}

	if (keys == Keypad::Keys::None)
		return;

	ui.last_keypress_time = clock_epoch;

	// If the display is sleeping, use this keypress to wake it up,
	// but don't interact with the UI while it's invisible.
#ifndef CONFIG_USE_OLED_SH1106	
	if (!ssd1306_is_display_enabled()) {
#else
	if (!sh1106_is_display_on()) {
#endif
		return;
	}

	ui_result result = UI_RESULT_IDLE;

	switch (ui.screen) {
	case ui_s::SCR_MAIN:
		result = main_screen_handle_keys(keys);
		break;

	case ui_s::SCR_MENU:
		result = menu_screen_handle_handle_keys(keys);
		break;

	case ui_s::SCR_SCHEDULING:
		result = scheduling_screen_handle_keys(keys);
		break;
	}

#ifdef ENABLE_DEBUG
	printf("ui_result=%d\r\n", result);
#endif

	if (result == UI_RESULT_IDLE)
		return;

	if (result == UI_RESULT_UPDATE) {
		ui_update();
		return;
	}

	// Clear the screen before switching
#ifndef CONFIG_USE_OLED_SH1106
	ssd1306_clear();
#else
	sh1106_clear();
#endif

	switch (result) {
	case UI_RESULT_SWITCH_MAIN_SCREEN:
		ui.screen = ui_s::SCR_MAIN;
		main_screen_draw();
		break;

	case UI_RESULT_SWITCH_MENU_SCREEN:
		ui.screen = ui_s::SCR_MENU;
		menu_screen_init();
		menu_screen_draw();
		break;

	case UI_RESULT_SWITCH_SCHEDULING_SCREEN:
		ui.screen = ui_s::SCR_SCHEDULING;
		scheduling_screen_init();
		scheduling_screen_draw();
		break;
	}

#ifdef ENABLE_DEBUG
	printf("ui.screen=%d\r\n", ui.screen);
#endif
}

static void update_display_active_state()
{
#ifndef CONFIG_USE_OLED_SH1106
	if (is_ui_active()) {
		if (!ssd1306_is_display_enabled()) {
			ssd1306_set_display_enabled(1);
		}
	} else {
		if (ssd1306_is_display_enabled()) {
			ssd1306_set_display_enabled(0);
		}
	}
#else
	if (is_ui_active()) {
		if (!sh1106_is_display_on()) {
			sh1106_set_display_on(true);
		}
	} else {
		if (sh1106_is_display_on()) {
			sh1106_set_display_on(false);
		}
	}
#endif
}

static bool is_ui_active()
{
	if (settings.display.timeout_secs == 0) {
		return true;
	}

	return (clock_epoch - ui.last_keypress_time) < (time_t)(settings.display.timeout_secs);
}