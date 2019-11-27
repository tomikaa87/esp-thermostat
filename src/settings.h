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
    Created on 2017-01-09
*/

#ifndef SETTINGS_H
#define	SETTINGS_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef uint8_t schedule_day_data[6];

struct persistent_settings {
	struct {
		uint8_t enabled: 1;
		uint8_t: 0;

		schedule_day_data days[7];
	} schedule;

	struct {
		// Value between 0 and 255
		uint8_t brightness;
		uint8_t timeout_secs;
	} display;

	struct _settings_heatctl {
		uint8_t mode;

		// Temperature values in 0.1 Celsius
		int16_t day_temp;
		int16_t night_temp;

		// Values for histeresis in 0.1 Celsius
		uint8_t overshoot;
		uint8_t undershoot;
		int8_t temp_correction;

		// Values in minutes
		uint8_t boost_intval;
        uint16_t custom_temp_timeout;
	} heatctl;
} __attribute__((packed));

#define SETTINGS_TEMP_MIN				100
#define SETTINGS_TEMP_MAX				300

#define SETTINGS_LIMIT_HEATCTL_DAY_TEMP_MAX		SETTINGS_TEMP_MAX
#define SETTINGS_LIMIT_HEATCTL_DAY_TEMP_MIN		SETTINGS_TEMP_MIN
#define SETTINGS_LIMIT_HEATCTL_NIGHT_TEMP_MAX		SETTINGS_TEMP_MAX
#define SETTINGS_LIMIT_HEATCTL_NIGHT_TEMP_MIN		SETTINGS_TEMP_MIN
#define SETTINGS_LIMIT_HEATCTL_OVERSHOOT_MAX		10
#define SETTINGS_LIMIT_HEATCTL_OVERSHOOT_MIN		1
#define SETTINGS_LIMIT_HEATCTL_UNDERSHOOT_MAX		10
#define SETTINGS_LIMIT_HEATCTL_UNDERSHOOT_MIN		1
#define SETTINGS_LIMIT_HEATCTL_BOOST_INTVAL_MAX		60
#define SETTINGS_LIMIT_HEATCTL_BOOST_INTVAL_MIN		5
#define SETTINGS_LIMIT_HEATCTL_TEMP_CORR_MAX		100
#define SETTINGS_LIMIT_HEATCTL_TEMP_CORR_MIN		-100
#define SETTINGS_LIMIT_HEATCTL_CUSTOM_TEMP_TIMEOUT_MIN  0
#define SETTINGS_LIMIT_HEATCTL_CUSTOM_TEMP_TIMEOUT_MAX  1440

#define SETTINGS_DEFAULT_HEATCTL_DAY_TEMP		220
#define SETTINGS_DEFAULT_HEATCTL_NIGHT_TEMP		200
#define SETTINGS_DEFAULT_HEATCTL_OVERSHOOT		5
#define SETTINGS_DEFAULT_HEATCTL_UNDERSHOOT		5
#define SETTINGS_DEFAULT_HEATCTL_BOOST_INTVAL		10
#define SETTINGS_DEFAULT_HEATCTL_CUSTOM_TEMP_TIMEOUT    240
#define SETTINGS_DEFAULT_HEATCTL_TEMP_CORR		0
#define SETTINGS_DEFAULT_DISPLAY_BRIGHTNESS		20
#define SETTINGS_DEFAULT_DISPLAY_TIMEOUT_SECS		15

extern struct persistent_settings settings;

typedef uint8_t (* settings_read_func_t)(uint8_t address);
typedef void (* settings_write_func_t)(uint8_t address, uint8_t data);

void settings_init(settings_read_func_t read_func, settings_write_func_t write_func);
void settings_load();
void settings_save();
void settings_save_heatctl();

#ifdef	__cplusplus
}
#endif

#endif	/* SETTINGS_H */

