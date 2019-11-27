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
    Created on 2016-10-10
*/

#include "ssd1306.h"

//#define SSD1306_DEBUG
//#define SSD1306_VERBOSE_ERRORS

#include <stdbool.h>
#include <stdio.h>

#define ssd1306_screen_buffer_size (SSD1306_LCDHEIGHT * SSD1306_LCDWIDTH / 8)

bool ssd1306_display_on;
static ssd1306_i2c_send_func_t i2c_send = NULL;

void ssd1306_set_i2c_send_func(ssd1306_i2c_send_func_t func)
{
    i2c_send = func;
}

void ssd1306_init()
{
	ssd1306_send_command(SSD1306_CMD_DISPLAYOFF);

	ssd1306_send_command(SSD1306_CMD_SETDISPLAYCLOCKDIV);
	ssd1306_send_command(0x80);

	ssd1306_send_command(SSD1306_CMD_SETMULTIPLEX);
	ssd1306_send_command(SSD1306_LCDHEIGHT - 1);

	ssd1306_send_command(SSD1306_CMD_SETDISPLAYOFFSET);
	ssd1306_send_command(0x0);

	ssd1306_send_command(SSD1306_CMD_SETSTARTLINE | 0x0);

	ssd1306_send_command(SSD1306_CMD_CHARGEPUMP);
	ssd1306_send_command(0x14);

	ssd1306_send_command(SSD1306_CMD_MEMORYMODE);
	ssd1306_send_command(SSD1306_MEM_MODE_HORIZONTAL_ADDRESSING);

	ssd1306_send_command(SSD1306_CMD_SEGREMAP | 0x1);
	ssd1306_send_command(SSD1306_CMD_COMSCANDEC);

	ssd1306_send_command(SSD1306_CMD_SETCOMPINS);
	ssd1306_send_command(0x12);

	ssd1306_dim(SSD1306_DIM_NORMAL);

	ssd1306_send_command(SSD1306_CMD_SETVCOMDETECT);
	ssd1306_send_command(0x10);

	ssd1306_send_command(SSD1306_CMD_DISPLAYALLON_RESUME);
	ssd1306_send_command(SSD1306_CMD_NORMALDISPLAY);
	ssd1306_send_command(SSD1306_CMD_DEACTIVATE_SCROLL);
	ssd1306_send_command(SSD1306_CMD_DISPLAYON);

	ssd1306_clear();

#if defined SSD1306_DEBUG
	printf("ssd1306_init: filling buffer with test data\r\n");
	for (uint16_t i = 0; i < sizeof(ssd1306_screen_buffer); ++i)
		ssd1306_screen_buffer[i] = 0xAA;
#endif
	
	ssd1306_display_on = 1;
}

void ssd1306_set_invert(uint8_t enabled)
{
	ssd1306_send_command(enabled ? SSD1306_CMD_INVERTDISPLAY : SSD1306_CMD_NORMALDISPLAY);
}

void ssd1306_scroll(ssd1306_scroll_t scroll, uint8_t start, uint8_t stop)
{
	switch (scroll) {
	case SSD1306_SCROLL_STOP:
		ssd1306_send_command(SSD1306_CMD_DEACTIVATE_SCROLL);
		return;

	case SSD1306_SCROLL_LEFT:
		ssd1306_send_command(SSD1306_CMD_LEFT_HORIZONTAL_SCROLL);
		ssd1306_send_command(0);
		ssd1306_send_command(start);
		ssd1306_send_command(0);
		ssd1306_send_command(stop);
		ssd1306_send_command(0);
		ssd1306_send_command(0xFF);
		ssd1306_send_command(SSD1306_CMD_ACTIVATE_SCROLL);
		break;

	case SSD1306_SCROLL_RIGHT:
		ssd1306_send_command(SSD1306_CMD_RIGHT_HORIZONTAL_SCROLL);
		ssd1306_send_command(0);
		ssd1306_send_command(start);
		ssd1306_send_command(0);
		ssd1306_send_command(stop);
		ssd1306_send_command(0);
		ssd1306_send_command(0xFF);
		ssd1306_send_command(SSD1306_CMD_ACTIVATE_SCROLL);
		break;

	case SSD1306_SCROLL_DIAG_LEFT:
		ssd1306_send_command(SSD1306_CMD_SET_VERTICAL_SCROLL_AREA);
		ssd1306_send_command(0);
		ssd1306_send_command(SSD1306_LCDHEIGHT);
		ssd1306_send_command(SSD1306_CMD_VERTICAL_AND_LEFT_HORIZONTAL_SCROLL);
		ssd1306_send_command(0);
		ssd1306_send_command(start);
		ssd1306_send_command(0);
		ssd1306_send_command(stop);
		ssd1306_send_command(1);
		ssd1306_send_command(SSD1306_CMD_ACTIVATE_SCROLL);
		break;

	case SSD1306_SCROLL_DIAG_RIGHT:
		ssd1306_send_command(SSD1306_CMD_SET_VERTICAL_SCROLL_AREA);
		ssd1306_send_command(0);
		ssd1306_send_command(SSD1306_LCDHEIGHT);
		ssd1306_send_command(SSD1306_CMD_VERTICAL_AND_RIGHT_HORIZONTAL_SCROLL);
		ssd1306_send_command(0);
		ssd1306_send_command(start);
		ssd1306_send_command(0);
		ssd1306_send_command(stop);
		ssd1306_send_command(1);
		ssd1306_send_command(SSD1306_CMD_ACTIVATE_SCROLL);
		break;
	}
}

void ssd1306_set_brightness(uint8_t brightness)
{
	ssd1306_send_command(SSD1306_CMD_SETCONTRAST);
	ssd1306_send_command(brightness);
}

void ssd1306_dim(ssd1306_dim_level_t level)
{
	uint8_t brightness = 0;

	switch (level) {
	default:
	case SSD1306_DIM_BRIGHT:
		brightness = 0xCF;
		break;

	case SSD1306_DIM_NORMAL:
		brightness = 0x60;
		break;

	case SSD1306_DIM_DARK:
		brightness = 0x20;
		break;

	case SSD1306_DIM_DARKEST:
		brightness = 0;
		break;
	}

	ssd1306_set_brightness(brightness);
}

void ssd1306_clear()
{
	ssd1306_send_command(SSD1306_CMD_MEMORYMODE);
	ssd1306_send_command(SSD1306_MEM_MODE_HORIZONTAL_ADDRESSING);
	
	ssd1306_send_command(SSD1306_CMD_COLUMNADDR);
	ssd1306_send_command(0); // Column start address (0 = reset)
	ssd1306_send_command(SSD1306_LCDWIDTH - 1); // Column end address (127 = reset)

	ssd1306_send_command(SSD1306_CMD_PAGEADDR);
	ssd1306_send_command(0); // Page start address (0 = reset)

	// Page end address
#ifdef SSD1306_128_64
	ssd1306_send_command(7);
#elif defined SSD1306_128_32
	ssd1306_send_command(3);
#elif defined SSD1306_96_16
	ssd1306_send_command(1);
#endif

	for (uint16_t i = 0; i < ssd1306_screen_buffer_size; i += 16) {
#ifdef SSD1306_DEBUG
		printf("ssd1306_clear: sending data chunk. i = %u", i, buffer);
#endif

		uint8_t data[17];
		data[0] = SSD1306_I2C_DC_FLAG;

		for (uint8_t j = 1; j < 17; ++j)
			data[j] = 0;
		
		i2c_send(data, sizeof(data));

#ifdef SSD1306_DEBUG
		printf("ssd1306_update: data chunk written\r\n");
#endif
	}

#ifdef SSD1306_DEBUG
	printf("ssd1306_update: finished\r\n");
#endif
}

void ssd1306_send_command(ssd1306_cmd_t cmd)
{
	uint8_t data[2] = {0};
	data[1] = (uint8_t) cmd;
	
	i2c_send(data, sizeof(data));

#ifdef SSD1306_DEBUG
	printf("ssd1306_send_command: %02x. I2C status: %d\r\n", cmd, status);
#endif
}

void ssd1306_set_column_address(uint8_t start, uint8_t end)
{
	if (start > 127 || end > 127) {
#ifdef SSD1306_VERBOSE_ERRORS
		printf("ssd1306_set_column_address: invalid range: %u-%u\r\n", start, end);
#endif
		return;
	}

	uint8_t data[3];
	data[0] = SSD1306_CMD_COLUMNADDR;
	data[1] = start;
	data[2] = end;
	
	i2c_send(data, sizeof(data));
}

void ssd1306_set_page_address(uint8_t start, uint8_t end)
{
	if (start > 7 || end > 7) {
#ifdef SSD1306_VERBOSE_ERRORS
		printf("ssd1306_set_page_address: invalid range: %u-%u\r\n", start, end);
#endif
		return;
	}

	uint8_t data[3];
	data[0] = SSD1306_CMD_PAGEADDR;
	data[1] = start;
	data[2] = end;
	
	i2c_send(data, sizeof(data));
}

void ssd1306_set_page(uint8_t page)
{
	if (page > 7) {
#ifdef SSD1306_VERBOSE_ERRORS
		printf("ssd1306_set_page: invalid page: %u\r\n", page);
#endif
		return;
	}

	ssd1306_send_command(SSD1306_CMD_PAGESTARTADDR | page);
}

void ssd1306_set_start_column(uint8_t address)
{
	if (address > 127) {
#ifdef SSD1306_VERBOSE_ERRORS
		printf("ssd1306_set_start_column: invalid address: %u\r\n", address);
#endif
		return;
	}

	// Set lower nibble
	ssd1306_send_command(SSD1306_CMD_SETLOWCOLUMN | (address & 0xF));
	// Set upper nibble
	ssd1306_send_command(SSD1306_CMD_SETHIGHCOLUMN | ((address >> 4) & 0xF));
}

void ssd1306_send_data(const uint8_t* data, uint8_t length, uint8_t bit_shift)
{
	if (bit_shift > 7)
		return;
	
	uint8_t buffer[17];
	uint8_t bytes_remaining = length;
	uint8_t data_index = 0;
	static const uint8_t chunk_size = 16;

	while (bytes_remaining > 0) {
		uint8_t count = bytes_remaining >= chunk_size ? chunk_size : bytes_remaining;
		bytes_remaining -= count;

		buffer[0] = SSD1306_I2C_DC_FLAG;
		for (uint8_t i = 1; i <= count; ++i)
			buffer[i] = data[data_index++] << bit_shift;
		
		i2c_send(buffer, count + 1);
	}
}

void ssd1306_page_addressing()
{
	ssd1306_send_command(SSD1306_CMD_MEMORYMODE);
	ssd1306_send_command(SSD1306_MEM_MODE_PAGE_ADDRESSING);
}

void ssd1306_fill_area(uint8_t x, uint8_t start_page, uint8_t width, uint8_t pages, uint8_t color)
{
	if (width == 0 || x >= SSD1306_LCDWIDTH)
		return;
	
	ssd1306_page_addressing();
	
	uint8_t data[2];
	data[0] = SSD1306_I2C_DC_FLAG;
	data[1] = color > 0 ? 0xFF : 0;
	
	for (uint8_t i = 0; i < pages; ++i) {
		uint8_t page = start_page + i;
		if (page >= 8)
			return;
		
		ssd1306_set_page(page);
		ssd1306_set_start_column(x);
		
		for (uint8_t j = 0; j < width && x + j < SSD1306_LCDWIDTH; ++j) {
			i2c_send(data, sizeof(data));
		}
	}
}

void ssd1306_set_display_enabled(uint8_t enabled)
{
	if (enabled) {
		ssd1306_send_command(SSD1306_CMD_DISPLAYON);
		ssd1306_display_on = 1;
	} else {
		ssd1306_send_command(SSD1306_CMD_DISPLAYOFF);
		ssd1306_display_on = 0;
	}
}

uint8_t ssd1306_is_display_enabled()
{
	return ssd1306_display_on;
}
