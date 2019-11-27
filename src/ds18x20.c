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
    Created on 2017-01-01
*/

#include "ds18x20.h"
#include "one_wire.h"
#include "settings.h"

#include <stdbool.h>

//#define ENABLE_DEBUG

#ifdef ENABLE_DEBUG
#include <stdio.h>
#endif

int16_t ds18x20_last_reading = 0;

#define TEMP_RESOLUTION 12

static void convert_t();
static int16_t read_sensor();

void ds18x20_update()
{
	static bool convert = true;
	
	if (convert) {
		convert_t();
	} else {
		int16_t t = read_sensor();
#ifdef ENABLE_DEBUG
		printf("##sensor=%d#", t);
#endif
		t += settings.heatctl.temp_correction * 10;
		ds18x20_last_reading = t;
	}
	
	convert = !convert;
}

static void convert_t()
{
	one_wire_reset();
	one_wire_write_byte(0xCC);
	one_wire_write_byte(0x44);
}

static int16_t read_sensor()
{
	one_wire_reset();
	one_wire_write_byte(0xCC);
	one_wire_write_byte(0xBE);

	uint8_t lo_byte = one_wire_read_byte();
	uint8_t hi_byte = one_wire_read_byte();
	uint16_t raw_value = (hi_byte << 8) + lo_byte;

	if (raw_value & 0x8000) {
		raw_value = ~raw_value + 1;
	}
	
#ifdef ENABLE_DEBUG
	printf("##raw=%04X#", raw_value);
#endif
	
	int16_t celsius = (raw_value >> (TEMP_RESOLUTION - 8)) * 100;
	uint16_t frac_part = (raw_value << (4 - (TEMP_RESOLUTION - 8))) & 0xf;
	frac_part *= 625;
	celsius += frac_part / 100;
	
	if (raw_value & 0x8000)
		celsius *= -1;

	return celsius;
}