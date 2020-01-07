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
#include "draw_helper.h"
#include "graphics.h"
#include "text.h"
#include "extras.h"

#ifndef CONFIG_USE_OLED_SH1106
#include "ssd1306.h"
#else
#include "sh1106.h"
#endif

#include <stdio.h>

void draw_weekday(uint8_t x, uint8_t wday)
{
	if (wday > 6)
		return;

	static const char days[7][4] = {
		"SUN",
		"MON",
		"TUE",
		"WED",
		"THU",
		"FRI",
		"SAT"
	};

	text_draw(days[wday], 0, x, 0, false);
}

void draw_mode_indicator(mode_indicator_t indicator)
{
	switch (indicator) {
	case DH_MODE_HEATING:
		graphics_draw_multipage_bitmap(graphics_flame_icon_20x3p, 20, 3, 92, 2);
		break;

	case DH_MODE_OFF:
		graphics_draw_multipage_bitmap(graphics_off_icon_20x3p, 20, 3, 92, 2);
		break;

	default:
#ifndef CONFIG_USE_OLED_SH1106
		ssd1306_fill_area(92, 2, 20, 3, 0);
#else
		sh1106_fill_area(92, 2, 20, 3, 0);
#endif
		break;
	}
}

void draw_schedule_bar(schedule_day_data sday)
{
	static const uint8_t long_tick = 0b11110000;
	static const uint8_t short_tick = 0b01110000;
	static const uint8_t bar_indicator = 0b00010111;
	static const uint8_t bar_no_indicator = 0b00010000;

#ifndef CONFIG_USE_OLED_SH1106
	ssd1306_page_addressing();
	ssd1306_set_page(6);
	ssd1306_set_start_column(3);
#else
	sh1106_set_page_addr(6);
	sh1106_set_col_addr(3);
#endif

	uint8_t tick_counter = 0;
	uint8_t long_tick_counter = 0;
	uint8_t schedule_byte_idx = 0;
	uint8_t schedule_bit_idx = 255; // Will overflow in the first round
	uint8_t indicator_counter = 0;
	uint8_t schedule_bit = 0;

	for (uint8_t x = 0; x < 121; ++x) {
		uint8_t bitmap;

		if (tick_counter == 0) {
			// Draw ticks
			if (long_tick_counter == 0)
				bitmap = long_tick;
			else
				bitmap = short_tick;
		} else {
			// Draw rest of the bar with or without the indicators
			if (indicator_counter < 2 && schedule_bit)
				bitmap = bar_indicator;
			else
				bitmap = bar_no_indicator;
		}

#ifndef CONFIG_USE_OLED_SH1106
		ssd1306_send_data(&bitmap, 1, 0, false);
#else
		sh1106_send_data(bitmap);
#endif

		if (++tick_counter == 5) {
			tick_counter = 0;
			if (++long_tick_counter == 6)
				long_tick_counter = 0;
		}

		++indicator_counter;
		if (tick_counter == 1 || tick_counter == 3) {
			indicator_counter = 0;
			if (++schedule_bit_idx == 8) {
				++schedule_byte_idx;
				schedule_bit_idx = 0;
			}

			schedule_bit = (sday[schedule_byte_idx] >> schedule_bit_idx) & 1;
		}
	}

	text_draw("0", 7, 1, 1, false);
	text_draw("6", 7, 31, 1, false);
	text_draw("12", 7, 58, 1, false);
	text_draw("18", 7, 88, 1, false);
	text_draw("24", 7, 115, 1, false);
}

void draw_schedule_indicator(uint8_t sch_intval_idx)
{
	static const uint8_t indicator_bitmap[] = {
		0b00010000,
		0b00100000,
		0b01111100,
		0b00100000,
		0b00010000
	};

	uint8_t x = 2; // initial offset from left
	x += sch_intval_idx << 1;	// for every "tick"
	x += sch_intval_idx >> 1;	// for every padding between "ticks"

	/*
	 0:     v
	 1:     . v
	 2:     . .  v
	 3:     . .  . v
	 4:     . .  . .  v
	 5:     . .  . .  . v
	        |||| |||| ||||
	        01234567890123

		0 -> 0
		1 -> 2
		2 -> 5
		3 -> 7
		4 -> 10
		5 -> 12
	 */

#ifndef CONFIG_USE_OLED_SH1106
	ssd1306_fill_area(0, 5, 128, 1, 0);
#else
	sh1106_fill_area(0, 5, 128, 1, 0);
#endif

	graphics_draw_bitmap(indicator_bitmap, sizeof(indicator_bitmap), x, 5);
}

void draw_temperature_value(uint8_t x, int8_t int_part, int8_t frac_part)
{
	if (int_part < 0 || frac_part < 0)
		text_draw_7seg_large("-", 2, x - 13);
	else
		text_draw_7seg_large(" ", 2, x - 13);

	// Draw integral part of the value
	char s[3] = { 0 };
	sprintf(s, "%02d", int_part >= 0 ? int_part : -int_part);
	text_draw_7seg_large(s, 2, x);

	// Draw the decimal point
	static const uint8_t dp_bitmap[] = { 0b01100000, 0b01100000 };
#ifndef CONFIG_USE_OLED_SH1106
	ssd1306_set_page(4);
	ssd1306_set_start_column(x + 28);
	ssd1306_send_data(dp_bitmap, sizeof(dp_bitmap), 0, false);
#else
	sh1106_set_page_addr(4);
	sh1106_set_col_addr(x + 28);
	sh1106_send_data_array(dp_bitmap, sizeof(dp_bitmap), 0, false);
#endif

	// Draw fractional part of the value
	sprintf(s, "%d", frac_part >= 0 ? frac_part : -frac_part);
	text_draw_7seg_large(s, 2, x + 32);

	// Draw the degree symbol
	static const uint8_t ds_bitmap[6] = {
		0b00011110,
		0b00101101,
		0b00110011,
		0b00110011,
		0b00101101,
		0b00011110
	};
#ifndef CONFIG_USE_OLED_SH1106	
	ssd1306_set_page(2);
	ssd1306_set_start_column(x + 48);
	ssd1306_send_data(ds_bitmap, sizeof(ds_bitmap), 1, false);
#else
	sh1106_set_page_addr(2);
	sh1106_set_col_addr(x + 48);
	sh1106_send_data_array(ds_bitmap, sizeof(ds_bitmap), 1, false);
#endif

	// Draw temperature unit
	text_draw_7seg_large("C ", 2, x + 56);
}