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

#include "heat_ctl.h"
#include "clock.h"
#include "config.h"
#include "extras.h"
#include "settings.h"

#include <Arduino.h>

#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "Peripherals.h"

//#define HEATCL_DEBUG

#define TEMPERATURE_STEP	5

static struct {
	unsigned boost_active: 1;
	unsigned boost_deactivated: 1;
	unsigned heating_active: 1;
	unsigned using_daytime_schedule: 1;
	unsigned settings_changed: 1;
        unsigned custom_temp_set: 1;
	unsigned: 0;

	time_t boost_end;
	tenths_of_degrees_t target_temp;
	tenths_of_degrees_t sensor_temp;

	time_t last_settings_change_time;
        time_t last_set_temp_change_time;
} heatctl;

static void start_heating();
static void stop_heating();
static void clamp_target_temp();
static bool is_mode_save_needed();
static void mark_settings_changed();
static bool is_custom_temp_reset_needed();
static void mark_custom_temp_set();

void heatctl_init()
{
	memset(&heatctl, 0, sizeof(heatctl));
	heatctl.target_temp = SETTINGS_TEMP_MIN;

	digitalWrite(D8, LOW);
	pinMode(D8, OUTPUT);
}

void heatctl_task()
{
	// Only Heat Control settings are auto-saved
	if (is_mode_save_needed()) {
		heatctl.settings_changed = 0;
		settings_save_heatctl();
	}

	// Read temperature sensor and store it in tenths of degrees
	heatctl.sensor_temp = Peripherals::Sensors::MainTemperature::lastReading() / 10;

#ifdef HEATCL_DEBUG
	printf("heatctl: s_tmp=%d\r\n", sensor_temp);
	printf("heatctl: bst_act=%u\r\n", heatctl.boost_active);
	printf("heatctl: bst_end=%lu\r\n", heatctl.boost_end);
	printf("heatctl: clk_epch=%lu\r\n", clock_epoch);
	printf("heatctl: day_ovr=%u\r\n", heatctl.day_override);
	printf("heatctl: nig_ovr=%u\r\n", heatctl.night_override);
	printf("heatctl: heat_act=%u\r\n", heatctl.heating_active);
#endif

	if (heatctl.boost_active && clock_epoch >= heatctl.boost_end) {
		heatctl.boost_active = 0;
		heatctl.boost_deactivated = 1;

//		cli_log("BOOST end");

#ifdef HEATCL_DEBUG
		printf("heatctl: bst deact\r\n");
#endif
	}

	if (settings.heatctl.mode == HC_MODE_NORMAL) {
		// On schedule change, update target temperature
		bool has_daytime_schedule = heatctl_has_daytime_schedule();

		if (has_daytime_schedule != heatctl.using_daytime_schedule
                        || heatctl.target_temp == 0
                        || is_custom_temp_reset_needed()) {

			heatctl.using_daytime_schedule = has_daytime_schedule;

                        if (heatctl.custom_temp_set) {
//                                cli_log("Custom Temp reset");
                                heatctl.custom_temp_set = 0;
                        }

			if (heatctl.using_daytime_schedule) {
//				cli_log("Target = Daytime");
				heatctl.target_temp = settings.heatctl.day_temp;
			} else {
//				cli_log("Target = Night time");
				heatctl.target_temp = settings.heatctl.night_temp;
			}
		}
	}

#ifdef HEATCL_DEBUG
	printf("heatctl: tgt_tmp=%u\r\n", heatctl.target_temp);
#endif

	// Heating control works in the following way:
	// Start heating if it's inactive and:
	//	- boost is active
	//		OR
	//	- current temp <= target temp + undershoot
	//
	// Stop heating if it's active and:
	//	- boost was deactivated in the current iteration and the
	//	  current temperature >= target temperature
	//		OR
	//	- current temp >= target temp + overshoot
	//
	// Undershoot and overshoot are the values that give the system
	// hysteresis which avoids switching the heat on and off in a short
	// period of time.
	//
	// BOOST function turns on heating instantly for a given interval
	// without altering the set temperature values. After deactivating
	// BOOST the heating should be switched off when the current temperature
	// is higher than the target temperature, without adding the overshoot
	// value to it. This avoids unnecessary heating when the temperature
	// is already high enough.

	do {
		if (heatctl.heating_active) {
			if (heatctl.boost_active) {
				break;
			}

			if (settings.heatctl.mode == HC_MODE_OFF) {
//				cli_log("Stop: OFF mode");
				stop_heating();
				break;
			}

			if (heatctl.sensor_temp >= heatctl.target_temp && heatctl.boost_deactivated) {
//				cli_log("Stop: BOOST");
				stop_heating();
				break;
			}

			if (heatctl.sensor_temp >= heatctl.target_temp + settings.heatctl.overshoot) {
//				cli_log("Stop: hi temp");
				stop_heating();
			}
		} else {
			if (heatctl.boost_active) {
//				cli_log("Start: BOOST");
				start_heating();
				break;
			}

			if (settings.heatctl.mode == HC_MODE_OFF) {
				break;
			}

			if (heatctl.sensor_temp <= heatctl.target_temp - settings.heatctl.undershoot) {
//				cli_log("Start: low temp");
				start_heating();
			}
		}
	} while (false);

	heatctl.boost_deactivated = 0;
}

heatctl_mode_t heatctl_mode()
{
	if (heatctl_is_boost_active()) {
		return HC_MODE_BOOST;
	}

	return static_cast<heatctl_mode_t>(settings.heatctl.mode);
}

void heatctl_set_mode(heatctl_mode_t mode)
{
	if (mode == HC_MODE_BOOST) {
		if (!heatctl_is_boost_active()) {
			heatctl_activate_boost();
		}
	} else {
		if (heatctl_is_boost_active()) {
			heatctl_deactivate_boost();
		}

		settings.heatctl.mode = mode;

		mark_settings_changed();
	}
}

void heatctl_inc_target_temp()
{
	if (heatctl.target_temp >= SETTINGS_TEMP_MAX)
		return;

	heatctl.target_temp += TEMPERATURE_STEP;
	clamp_target_temp();

        mark_custom_temp_set();
}

void heatctl_dec_target_temp()
{
	if (heatctl.target_temp <= SETTINGS_TEMP_MIN)
		return;

	heatctl.target_temp -= TEMPERATURE_STEP;
	clamp_target_temp();

        mark_custom_temp_set();
}

bool heatctl_is_active()
{
	return heatctl.heating_active;
}

bool heatctl_is_boost_active()
{
	return heatctl.boost_active;
}

time_t heatctl_boost_remaining_secs()
{
	if (!heatctl.boost_active)
		return 0ul;

	return heatctl.boost_end - clock_epoch;
}

tenths_of_degrees_t heatctl_target_temp()
{
	return heatctl.target_temp;
}

void heatctl_set_target_temp(tenths_of_degrees_t value)
{
	heatctl.target_temp = value;
	clamp_target_temp();

        mark_custom_temp_set();
}

tenths_of_degrees_t heatctl_daytime_temp()
{
	return settings.heatctl.day_temp;
}

void heatctl_set_daytime_temp(tenths_of_degrees_t value)
{
	settings.heatctl.day_temp = value;

	CLAMP_VALUE(settings.heatctl.day_temp,
		SETTINGS_LIMIT_HEATCTL_DAY_TEMP_MIN,
		SETTINGS_LIMIT_HEATCTL_DAY_TEMP_MAX);

	mark_settings_changed();
}

tenths_of_degrees_t heatctl_night_time_temp()
{
	return settings.heatctl.night_temp;
}

void heatctl_set_night_time_temp(tenths_of_degrees_t value)
{
	settings.heatctl.night_temp = value;

	CLAMP_VALUE(settings.heatctl.night_temp,
		SETTINGS_LIMIT_HEATCTL_NIGHT_TEMP_MIN,
		SETTINGS_LIMIT_HEATCTL_NIGHT_TEMP_MAX);

	mark_settings_changed();
}

tenths_of_degrees_t heatctl_current_temp()
{
	return heatctl.sensor_temp;
}

void heatctl_activate_boost()
{
	if (heatctl.boost_active)
		return;

#ifdef HEATCL_DEBUG
	printf("heatctl: bst_act\r\n");
#endif
//	cli_log("BOOST activated");

	heatctl.boost_end = clock_epoch + settings.heatctl.boost_intval * 60;
	heatctl.boost_active = 1;
}

void heatctl_deactivate_boost()
{
#ifdef HEATCL_DEBUG
	printf("heatctl: bst_deact\r\n");
#endif

//	cli_log("BOOST deactivated");

	heatctl.boost_active = 0;
	heatctl.boost_deactivated = 1;
}

void heatctl_extend_boost()
{
#ifdef HEATCL_DEBUG
	printf("heatctl: bst_ext\r\n");
#endif
//	cli_log("BOOST extended");

	heatctl.boost_end += settings.heatctl.boost_intval * 60;

	// Max boost time is 4 hours
	if (heatctl_boost_remaining_secs() > 4 * 3600)
		heatctl.boost_end = clock_epoch + 4 * 3600;

}

heatctl_state_t scheduled_state_at(uint8_t wd, uint8_t h, uint8_t m)
{
	uint8_t intval_idx = calculate_schedule_intval_idx(h, m);
	uint8_t bit_idx = intval_idx & 0b111;
	uint8_t byte_idx = intval_idx >> 3;
	uint8_t mask = 1 << bit_idx;

	return ((settings.schedule.days[wd][byte_idx] & mask) > 0)
		? HC_STATE_HEATING_ON
		: HC_STATE_HEATING_OFF;
}

bool heatctl_has_daytime_schedule()
{
	struct tm* t = gmtime(&clock_epoch);
	return scheduled_state_at(t->tm_wday, t->tm_hour, t->tm_min);
}

struct heatctl_next_switch heatctl_next_state()
{
	struct tm* t = gmtime(&clock_epoch);

	uint8_t wd = t->tm_wday;
	uint8_t h = t->tm_hour;
	uint8_t m = t->tm_min;
	heatctl_state_t current_state = scheduled_state_at(wd, h, m);

	if (m >= 30)
		m = 30;
	else
		m = 0;

	uint8_t orig_wd = wd;
	uint8_t orig_h = h;
	uint8_t orig_m = m;

	struct heatctl_next_switch ns;
	memset(&ns, 0xff, sizeof(struct heatctl_next_switch));

	do {
		heatctl_state_t next_state = scheduled_state_at(wd, h, m);

		if (next_state != current_state) {
			ns.state = next_state;
			ns.weekday = wd;
			ns.hour = h;
			ns.minute = m;
			break;
		}

		m += 30;
		if (m == 60) {
			m = 0;
			if (++h == 24) {
				h = 0;
				if (++wd == 7)
					wd = 0;
			}
		}
	} while (wd != orig_wd || h != orig_h || m != orig_m);

	return ns;
}

static void start_heating()
{
#ifdef HEATCL_DEBUG
	printf("heatctl: start\r\n");
#endif

	heatctl.heating_active = 1;
	digitalWrite(D8, HIGH);
}

static void stop_heating()
{
#ifdef HEATCL_DEBUG
	printf("heatctl: stop\r\n");
#endif

	heatctl.heating_active = 0;
	digitalWrite(D8, LOW);
}

static void clamp_target_temp()
{
	if (heatctl.using_daytime_schedule) {
		CLAMP_VALUE(heatctl.target_temp,
			SETTINGS_LIMIT_HEATCTL_DAY_TEMP_MIN,
			SETTINGS_LIMIT_HEATCTL_DAY_TEMP_MAX);
	} else {
		CLAMP_VALUE(heatctl.target_temp,
			SETTINGS_LIMIT_HEATCTL_NIGHT_TEMP_MIN,
			SETTINGS_LIMIT_HEATCTL_NIGHT_TEMP_MAX);
	}
}

static bool is_mode_save_needed()
{
	// Mode change should be saved after a few seconds.
	// This delay could spare EEPROM write cycles if the mode is being
	// changed in rapid successions.

	if (heatctl.settings_changed &&
		(heatctl.last_settings_change_time + 10 <= clock_epoch)) {
		return true;
	}

	return false;
}

static void mark_settings_changed()
{
	heatctl.settings_changed = 1;
	heatctl.last_settings_change_time = clock_epoch;
}

static bool is_custom_temp_reset_needed()
{
        if (!heatctl.custom_temp_set || settings.heatctl.custom_temp_timeout == 0) {
            return false;
        }

        return clock_epoch >= (heatctl.last_set_temp_change_time
                + settings.heatctl.custom_temp_timeout * 60);
}

static void mark_custom_temp_set()
{
        heatctl.custom_temp_set = 1;
        heatctl.last_set_temp_change_time = clock_epoch;
}