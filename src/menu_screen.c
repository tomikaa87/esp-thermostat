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

#include "config.h"
#include "menu_screen.h"
#include "text.h"
#include "keypad.h"
#include "draw_helper.h"
#include "heat_ctl.h"
#include "settings.h"
#include "extras.h"
#include "graphics.h"
#include "text_input.h"

#ifndef CONFIG_USE_OLED_SH1106
#include "ssd1306.h"
#else
#include "sh1106.h"
#endif

#include <stdio.h>

static struct menu_s {
	enum {
		PAGE_FIRST = 0,

		PAGE_HEATCTL_MODE = PAGE_FIRST,
		PAGE_DAYTIME_TEMP,
		PAGE_NIGHTTIME_TEMP,
		PAGE_TEMP_OVERSHOOT,
		PAGE_TEMP_UNDERSHOOT,
		PAGE_BOOST_INTVAL,
		PAGE_CUSTOM_TEMP_TIMEOUT,
		PAGE_DISPLAY_BRIGHTNESS,
		PAGE_DISPLAY_TIMEOUT,
		PAGE_TEMP_CORRECTION,
		PAGE_WIFI,

		PAGE_LAST,

		// These pages cannot be accessed by normal navigation
		PAGE_WIFI_PASSWORD
	} page;

	char wifi_psw[64];

	struct persistent_settings new_settings;
} menu;

static void draw_page_heatctl_mode();
static void draw_page_daytime_temp();
static void draw_page_nighttime_temp();
static void draw_page_temp_overshoot();
static void draw_page_temp_undershoot();
static void draw_page_boost_intval();
static void draw_page_custom_temp_timeout();
static void draw_page_display_brightness();
static void draw_page_display_timeout();
static void draw_page_temp_correction();
static void draw_page_wifi();
static void draw_page_wifi_password();
static void update_page_heatctl_mode();
static void update_page_daytime_temp();
static void update_page_nighttime_temp();
static void update_page_temp_overshoot();
static void update_page_temp_undershoot();
static void update_page_boost_intval();
static void update_page_custom_temp_timeout();
static void update_page_temp_correction();
static void update_page_display_brightness();
static void update_page_display_timeout();
static void update_page_wifi();
static void draw_page_title(const char* text);
static void next_page();
static void previous_page();
static void apply_settings();
static void revert_settings();
static void adjust_value(int8_t amount);

void menu_screen_init()
{
	menu.page = PAGE_FIRST;
	revert_settings();
}

void menu_screen_draw()
{
#ifndef CONFIG_USE_OLED_SH1106
	ssd1306_clear();
#else
	sh1106_clear();
#endif

	switch (menu.page) {
	case PAGE_HEATCTL_MODE:
		draw_page_heatctl_mode();
		break;

	case PAGE_DAYTIME_TEMP:
		draw_page_daytime_temp();
		break;

	case PAGE_NIGHTTIME_TEMP:
		draw_page_nighttime_temp();
		break;

	case PAGE_TEMP_OVERSHOOT:
		draw_page_temp_overshoot();
		break;

	case PAGE_TEMP_UNDERSHOOT:
		draw_page_temp_undershoot();
		break;

	case PAGE_BOOST_INTVAL:
		draw_page_boost_intval();
		break;

	case PAGE_CUSTOM_TEMP_TIMEOUT:
		draw_page_custom_temp_timeout();
		break;

	case PAGE_DISPLAY_BRIGHTNESS:
		draw_page_display_brightness();
		break;

	case PAGE_DISPLAY_TIMEOUT:
		draw_page_display_timeout();
		break;

	case PAGE_TEMP_CORRECTION:
		draw_page_temp_correction();
		break;

	case PAGE_WIFI:
		draw_page_wifi();
		break;

	case PAGE_WIFI_PASSWORD:
		draw_page_wifi_password();
		break;

	case PAGE_LAST:
		break;
	}

	// "<-" previous page indicator
	if (menu.page > PAGE_FIRST) {
		text_draw("<-", 7, 0, 0, false);
	}

	// "->" next page indicator
	if (menu.page < PAGE_LAST - 1) {
		text_draw("->", 7, 115, 0, false);
	}
}

ui_result menu_screen_handle_handle_keys(uint16_t keys)
{
	if (menu.page != PAGE_WIFI_PASSWORD) {
		// 1: increment current value
		// 2: decrement current value
		// 3: save and exit
		// 4: cancel (revert settings)
		// 5: navigate to previous page
		// 6: navigate to next page

		if (keys & KEY_1) {
			adjust_value(1);
		} else if (keys & KEY_2) {
			adjust_value(-1);
		} else if (keys & KEY_3) {
			apply_settings();
			return UI_RESULT_SWITCH_MAIN_SCREEN;
		} else if (keys & KEY_4) {
			revert_settings();
			return UI_RESULT_SWITCH_MAIN_SCREEN;
		} else if (keys & KEY_5) {
			previous_page();
		} else if (keys & KEY_6) {
			next_page();
		}
	} else {
		ti_key_event_t key_event;
		bool valid_key = true;

		if (keys & KEY_1) {
			key_event = TI_KE_UP;
		} else if (keys & KEY_2) {
			key_event = TI_KE_DOWN;
		} else if (keys & KEY_3) {
			key_event = TI_KE_SELECT;
		} else if (keys & KEY_5) {
			key_event = TI_KE_LEFT;
		} else if (keys & KEY_6) {
			key_event = TI_KE_RIGHT;
		} else {
			valid_key = false;
		}

		if (valid_key) {
			const ti_key_event_result_t res = text_input_key_event(key_event);

			if (res != TI_KE_NO_ACTION) {
				menu.page = PAGE_WIFI;
				menu_screen_draw();
			}
		}
	}

	return UI_RESULT_IDLE;
}

static void draw_page_heatctl_mode()
{
	draw_page_title("MODE");
	update_page_heatctl_mode();
}

static void draw_page_daytime_temp()
{
	draw_page_title("DAYTIME T.");
	update_page_daytime_temp();
}

static void draw_page_nighttime_temp()
{
	draw_page_title("NIGHTTIME T.");
	update_page_nighttime_temp();
}

static void draw_page_temp_overshoot()
{
	draw_page_title("T. OVERSHOOT");
	update_page_temp_overshoot();
}

static void draw_page_temp_undershoot()
{
	draw_page_title("T. UNDERSHOOT");
	update_page_temp_undershoot();
}

static void draw_page_boost_intval()
{
	draw_page_title("BOOST INT. (MIN)");
	update_page_boost_intval();
}

static void draw_page_custom_temp_timeout()
{
	draw_page_title("CUS. TMP. TOUT. (MIN)");
	update_page_custom_temp_timeout();
}

static void draw_page_display_brightness()
{
	draw_page_title("DISP. BRIGHT.");
	update_page_display_brightness();
}

static void draw_page_display_timeout()
{
	draw_page_title("DISP. TIMEOUT");
	update_page_display_timeout();
}

static void draw_page_temp_correction()
{
	draw_page_title("TEMP. CORR.");
	update_page_temp_correction();
}

static void draw_page_wifi()
{
	draw_page_title("WIFI CONN.");
	update_page_wifi();
}

static void draw_page_wifi_password()
{
	text_input_init(menu.wifi_psw, sizeof(menu.wifi_psw), "WiFi password:");
}

static void update_page_heatctl_mode()
{
	// FIXME: proper mode name must be shown

	char num[3] = { 0 };
	sprintf(num, "%2d", menu.new_settings.heatctl.mode);

#ifndef CONFIG_USE_OLED_SH1106
	ssd1306_fill_area(0, 3, 128, 3, 0);
#else
	sh1106_fill_area(0, 3, 128, 3, 0);
#endif

	switch (menu.new_settings.heatctl.mode) {
	case HC_MODE_NORMAL:
		graphics_draw_multipage_bitmap(graphics_calendar_icon_20x3p, 20, 3, 20, 2);
		text_draw("NORMAL", 3, 50, 0, false);
		text_draw("(SCHEDULE)", 4, 50, 0, false);
		break;

	case HC_MODE_OFF:
		graphics_draw_multipage_bitmap(graphics_off_icon_20x3p, 20, 3, 20, 2);
		text_draw("OFF", 3, 50, 0, false);
		break;
	}
}

static void update_page_daytime_temp()
{
	draw_temperature_value(20,
		menu.new_settings.heatctl.day_temp / 10,
		menu.new_settings.heatctl.day_temp % 10);
}

static void update_page_nighttime_temp()
{
	draw_temperature_value(20,
		menu.new_settings.heatctl.night_temp / 10,
		menu.new_settings.heatctl.night_temp % 10);
}

static void update_page_temp_overshoot()
{
	draw_temperature_value(20,
		menu.new_settings.heatctl.overshoot / 10,
		menu.new_settings.heatctl.overshoot % 10);
}

static void update_page_temp_undershoot()
{
	draw_temperature_value(20,
		menu.new_settings.heatctl.undershoot / 10,
		menu.new_settings.heatctl.undershoot % 10);
}

static void update_page_boost_intval()
{
	char num[3] = { 0 };
	sprintf(num, "%2d", menu.new_settings.heatctl.boost_intval);
	text_draw_7seg_large(num, 2, 20);
}

static void update_page_custom_temp_timeout()
{
	char num[5] = { 0 };
	sprintf(num, "%4u", menu.new_settings.heatctl.custom_temp_timeout);
	text_draw_7seg_large(num, 2, 20);
}

static void update_page_temp_correction()
{
	draw_temperature_value(20,
		menu.new_settings.heatctl.temp_correction / 10,
		menu.new_settings.heatctl.temp_correction % 10);
}

static void update_page_display_brightness()
{
	char num[4] = { 0 };
	sprintf(num, "%3d", menu.new_settings.display.brightness);
	text_draw_7seg_large(num, 2, 20);

#ifndef CONFIG_USE_OLED_SH1106
	ssd1306_set_brightness(menu.new_settings.display.brightness);
#else
	sh1106_set_contrast(menu.new_settings.display.brightness);
#endif
}

static void update_page_display_timeout()
{
	char num[4] = { 0 };
	sprintf(num, "%3d", menu.new_settings.display.timeout_secs);
	text_draw_7seg_large(num, 2, 20);
}

static void update_page_wifi()
{
	text_draw("ENABLED: YES", 2, 0, 0, false);
	text_draw("NETWORK:", 4, 0, 0, false);
	text_draw("<SSID>", 5, 0, 0, false);
}

static void draw_page_title(const char* text)
{
	text_draw(text, 0, 0, 0, false);
}

static void next_page()
{
	if (menu.page < PAGE_LAST - 1) {
		++menu.page;
		menu_screen_draw();
	}
}

static void previous_page()
{
	if (menu.page > PAGE_FIRST) {
		--menu.page;
		menu_screen_draw();
	}
}

static void apply_settings()
{
	settings = menu.new_settings;
	settings_save();
}

static void revert_settings()
{
	menu.new_settings = settings;

#ifndef CONFIG_USE_OLED_SH1106
	ssd1306_set_brightness(settings.display.brightness);
#else
	sh1106_set_contrast(settings.display.brightness);
#endif
}

static void adjust_value(int8_t amount)
{
	switch (menu.page) {
	case PAGE_HEATCTL_MODE:
		if (menu.new_settings.heatctl.mode == 0 && amount > 0) {
			menu.new_settings.heatctl.mode = 2;
		} else if (menu.new_settings.heatctl.mode == 2 && amount < 0) {
			menu.new_settings.heatctl.mode = 0;
		}
		update_page_heatctl_mode();
		break;

	case PAGE_DAYTIME_TEMP:
		menu.new_settings.heatctl.day_temp += amount;
		CLAMP_VALUE(menu.new_settings.heatctl.day_temp,
			SETTINGS_LIMIT_HEATCTL_DAY_TEMP_MIN,
			SETTINGS_LIMIT_HEATCTL_DAY_TEMP_MAX);
		update_page_daytime_temp();
		break;

	case PAGE_NIGHTTIME_TEMP:
		menu.new_settings.heatctl.night_temp += amount;
		CLAMP_VALUE(menu.new_settings.heatctl.night_temp,
			SETTINGS_LIMIT_HEATCTL_NIGHT_TEMP_MIN,
			SETTINGS_LIMIT_HEATCTL_NIGHT_TEMP_MAX);
		update_page_nighttime_temp();
		break;

	case PAGE_TEMP_OVERSHOOT:
		menu.new_settings.heatctl.overshoot += amount;
		CLAMP_VALUE(menu.new_settings.heatctl.overshoot,
			SETTINGS_LIMIT_HEATCTL_OVERSHOOT_MIN,
			SETTINGS_LIMIT_HEATCTL_OVERSHOOT_MAX);
		update_page_temp_overshoot();
		break;

	case PAGE_TEMP_UNDERSHOOT:
		menu.new_settings.heatctl.undershoot += amount;
		CLAMP_VALUE(menu.new_settings.heatctl.undershoot,
			SETTINGS_LIMIT_HEATCTL_OVERSHOOT_MIN,
			SETTINGS_LIMIT_HEATCTL_OVERSHOOT_MAX);
		update_page_temp_undershoot();
		break;

	case PAGE_BOOST_INTVAL:
		menu.new_settings.heatctl.boost_intval += amount;
		CLAMP_VALUE(menu.new_settings.heatctl.boost_intval,
			SETTINGS_LIMIT_HEATCTL_BOOST_INTVAL_MIN,
			SETTINGS_LIMIT_HEATCTL_BOOST_INTVAL_MAX);
		update_page_boost_intval();
		break;

	case PAGE_CUSTOM_TEMP_TIMEOUT:
		menu.new_settings.heatctl.custom_temp_timeout += amount;
		CLAMP_VALUE(menu.new_settings.heatctl.custom_temp_timeout,
			SETTINGS_LIMIT_HEATCTL_CUSTOM_TEMP_TIMEOUT_MIN,
			SETTINGS_LIMIT_HEATCTL_CUSTOM_TEMP_TIMEOUT_MAX);
		update_page_custom_temp_timeout();
		break;

	case PAGE_DISPLAY_BRIGHTNESS:
		menu.new_settings.display.brightness += amount;
		update_page_display_brightness();
		break;

	case PAGE_DISPLAY_TIMEOUT:
		menu.new_settings.display.timeout_secs += amount;
		update_page_display_timeout();
		break;

	case PAGE_TEMP_CORRECTION:
		menu.new_settings.heatctl.temp_correction += amount;
		CLAMP_VALUE(menu.new_settings.heatctl.temp_correction,
			SETTINGS_LIMIT_HEATCTL_TEMP_CORR_MIN,
			SETTINGS_LIMIT_HEATCTL_TEMP_CORR_MAX);
		update_page_temp_correction();
		break;

	// TODO dummy implementation
	case PAGE_WIFI:
		menu.page = PAGE_WIFI_PASSWORD;
		draw_page_wifi_password();
		break;

	case PAGE_LAST:
	case PAGE_WIFI_PASSWORD:
		break;
	}
}