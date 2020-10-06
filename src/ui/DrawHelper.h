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

#ifndef DRAW_HELPER_H
#define	DRAW_HELPER_H

#include <stdbool.h>
#include <stdint.h>

#include "Settings.h"

typedef enum {
	DH_NO_INDICATOR,
	DH_MODE_HEATING,
	DH_MODE_OFF
} mode_indicator_t;

void draw_weekday(uint8_t x, uint8_t wday);
void draw_mode_indicator(mode_indicator_t indicator);
void draw_schedule_bar(Settings::SchedulerDayData sday);
void draw_schedule_indicator(uint8_t sch_intval_idx);
void draw_temperature_value(uint8_t x, int8_t int_part, int8_t frac_part);

#endif	/* DRAW_HELPER_H */

