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
    Created on 2017-01-04
*/

#ifndef HEAT_CTL_H
#define	HEAT_CTL_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>
#include <time.h>

typedef enum
{
	HC_MODE_NORMAL,
	HC_MODE_BOOST,
	HC_MODE_OFF
} heatctl_mode_t;

typedef enum
{
	HC_STATE_HEATING_OFF,
	HC_STATE_HEATING_ON
} heatctl_state_t;

struct heatctl_next_switch {
	heatctl_state_t state;
	uint8_t weekday;
	uint8_t hour;
	uint8_t minute;
};

typedef int16_t tenths_of_degrees_t;

void heatctl_init();
void heatctl_task();

heatctl_mode_t heatctl_mode();
void heatctl_set_mode(heatctl_mode_t mode);

void heatctl_inc_target_temp();
void heatctl_dec_target_temp();

bool heatctl_is_active();
bool heatctl_is_boost_active();
time_t heatctl_boost_remaining_secs();

tenths_of_degrees_t heatctl_target_temp();
void heatctl_set_target_temp(tenths_of_degrees_t value);

tenths_of_degrees_t heatctl_daytime_temp();
void heatctl_set_daytime_temp(tenths_of_degrees_t value);

tenths_of_degrees_t heatctl_night_time_temp();
void heatctl_set_night_time_temp(tenths_of_degrees_t value);

tenths_of_degrees_t heatctl_current_temp();

void heatctl_activate_boost();
void heatctl_deactivate_boost();
void heatctl_extend_boost();

bool heatctl_has_daytime_schedule();
struct heatctl_next_switch heatctl_next_state();

#ifdef	__cplusplus
}
#endif

#endif	/* HEAT_CTL_H */

