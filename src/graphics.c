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
#include "graphics.h"

#ifndef CONFIG_USE_OLED_SH1106
#include "ssd1306.h"
#else
#include "sh1106.h"
#endif

void graphics_draw_bitmap(
	const uint8_t* bitmap,
	uint8_t width,
	uint8_t x,
	uint8_t page)
{
#ifndef CONFIG_USE_OLED_SH1106
	const uint8_t page_count = SSD1306_PAGE_COUNT;
	const uint8_t display_width = SSD1306_LCDWIDTH;
#else
	const uint8_t page_count = SH1106_PAGE_COUNT;
	const uint8_t display_width = SH1106_LCDWIDTH;
#endif
	
	if (page > page_count || width == 0 || x + width >= display_width)
		return;

#ifndef CONFIG_USE_OLED_SH1106
	ssd1306_page_addressing();
	ssd1306_set_page(page);
	ssd1306_set_start_column(x);
	ssd1306_send_data(bitmap, width, 0);
#else
	sh1106_set_page_addr(page);
	sh1106_set_col_addr(x);
	sh1106_send_data_array(bitmap, width, 0);
#endif
}

void graphics_draw_multipage_bitmap(
	const uint8_t* mp_bitmap,
	uint8_t width,
	uint8_t page_count,
	uint8_t x,
	uint8_t start_page)
{
#ifndef CONFIG_USE_OLED_SH1106
	const uint8_t max_page_count = SSD1306_PAGE_COUNT;
#else
	const uint8_t max_page_count = SH1106_PAGE_COUNT;
#endif

	if (start_page + page_count > max_page_count)
		return;

	const uint8_t* bitmap = mp_bitmap;

	for (uint8_t page = start_page; page < start_page + page_count; ++page) {
		graphics_draw_bitmap(bitmap, width, x, page);
		bitmap += width;
	}
}

const uint8_t graphics_flame_icon_20x3p[20 * 3] = {
	// page 0
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b11100000,
	0b11111000,
	0b00011100,
	0b00001110,
	0b11111111,
	0b11110000,
	0b00000000,
	0b00000000,
	0b10000000,
	0b11000000,
	0b11100000,
	0b11100000,
	0b00000000,
	0b00000000,

	// page 1
	0b11110000,
	0b11111100,
	0b00001110,
	0b00111100,
	0b01110000,
	0b01101110,
	0b11111111,
	0b00000001,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000011,
	0b00000110,
	0b00001100,
	0b00011111,
	0b00000011,
	0b00000000,
	0b00001111,
	0b11111111,
	0b11110000,

	// page 2
	0b00000000,
	0b00000111,
	0b00001111,
	0b00011100,
	0b00111000,
	0b01110000,
	0b01100000,
	0b11000000,
	0b11000000,
	0b11000000,
	0b11000000,
	0b11000000,
	0b11000000,
	0b01100000,
	0b01110000,
	0b00111000,
	0b00011100,
	0b00001111,
	0b00000111,
	0b00000000
};

const uint8_t graphics_off_icon_20x3p[20 * 3] = {
	// page 0
	0b00000000,
	0b00000000,
	0b00000000,
	0b10000000,
	0b11000000,
	0b11100000,
	0b01100000,
	0b00000000,
	0b00000000,
	0b11111110,
	0b11111110,
	0b00000000,
	0b00000000,
	0b01100000,
	0b11100000,
	0b11000000,
	0b10000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// page 1
	0b11111000,
	0b11111110,
	0b00000111,
	0b00000011,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000111,
	0b00000111,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000011,
	0b00000111,
	0b11111110,
	0b11111000,

	// page 2
	0b00000001,
	0b00000111,
	0b00001110,
	0b00011100,
	0b00110000,
	0b01110000,
	0b01100000,
	0b11000000,
	0b11000000,
	0b11000000,
	0b11000000,
	0b11000000,
	0b11000000,
	0b01100000,
	0b01110000,
	0b00110000,
	0b00011100,
	0b00001110,
	0b00000111,
	0b00000001
};

const uint8_t graphics_calendar_icon_20x3p[20 * 3] = {
	// page 0
	0b11000000,
	0b11100000,
	0b01100000,
	0b01100000,
	0b11111000,
	0b11111100,
	0b11111000,
	0b01100000,
	0b01100000,
	0b01100000,
	0b01100000,
	0b01100000,
	0b01100000,
	0b11111000,
	0b11111100,
	0b11111000,
	0b01100000,
	0b01100000,
	0b11100000,
	0b11000000,

	// page 1
	0b11111111,
	0b11111111,
	0b00000000,
	0b01100000,
	0b01100000,
	0b00000001,
	0b00000000,
	0b01101100,
	0b01101100,
	0b00000000,
	0b00000000,
	0b01101100,
	0b01101100,
	0b00000000,
	0b00000001,
	0b01101100,
	0b01101100,
	0b00000000,
	0b11111111,
	0b11111111,

	// page 2
	0b00001111,
	0b00011111,
	0b00011000,
	0b00011011,
	0b00011011,
	0b00011000,
	0b00011000,
	0b00011011,
	0b00011011,
	0b00011000,
	0b00011000,
	0b00011000,
	0b00011000,
	0b00011000,
	0b00011000,
	0b00011000,
	0b00011000,
	0b00011000,
	0b00011111,
	0b00001111
};