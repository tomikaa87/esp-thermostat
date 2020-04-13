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

#include "settings.h"
#include "HeatingController.h"
#include "Peripherals.h"

#include <stdbool.h>
#include <stddef.h>
#include <string.h>

struct persistent_settings settings;

static settings_read_func_t read_f;
static settings_write_func_t write_f;

static void check_settings();

void settings_init(settings_read_func_t read_func, settings_write_func_t write_func)
{
    read_f = read_func;
    write_f = write_func;
}

void settings_load()
{
    uint8_t* data = (uint8_t*)&settings;

    for (uint8_t i = 0; i < sizeof(struct persistent_settings); ++i, ++data) {
        *data = read_f(i);
    }

    check_settings();
}

void settings_save()
{
    const uint8_t* data = (uint8_t*)&settings;

    for (uint8_t i = 0; i < sizeof(struct persistent_settings); ++i, ++data) {
        write_f(i, *data);
    }

    Peripherals::Storage::EERAM::setAseEnabled(true);
}

void settings_save_heatctl()
{
    // Currently we save settings to the RTC SRAM, no need to do partial writes
    // to save EEPROM program cycles.
    settings_save();
}

static void check_settings()
{
    bool modified = false;

    // Reset Heat Control mode if it's corrupted
    if (settings.heatctl.mode > static_cast<uint8_t>(HeatingController::Mode::Off)) {
        settings.heatctl.mode = static_cast<uint8_t>(HeatingController::Mode::Off);
        modified = true;
    }

    // If daytime temp is out of range, reset to default
    if (settings.heatctl.day_temp > SETTINGS_LIMIT_HEATCTL_DAY_TEMP_MAX || settings.heatctl.day_temp < SETTINGS_LIMIT_HEATCTL_DAY_TEMP_MIN) {
        settings.heatctl.day_temp = SETTINGS_DEFAULT_HEATCTL_DAY_TEMP;
        modified = true;
    }

    // If nighttime temp is out of range, reset to default
    if (settings.heatctl.night_temp > SETTINGS_LIMIT_HEATCTL_NIGHT_TEMP_MAX || settings.heatctl.night_temp < SETTINGS_LIMIT_HEATCTL_NIGHT_TEMP_MIN) {
        settings.heatctl.night_temp = SETTINGS_DEFAULT_HEATCTL_NIGHT_TEMP;
        modified = true;
    }

    // If temperature overshoot is out of range, reset to default
    if (settings.heatctl.overshoot > SETTINGS_LIMIT_HEATCTL_OVERSHOOT_MAX || settings.heatctl.night_temp < SETTINGS_LIMIT_HEATCTL_OVERSHOOT_MIN) {
        settings.heatctl.overshoot = SETTINGS_DEFAULT_HEATCTL_OVERSHOOT;
        modified = true;
    }

    // If temperature undershoot is out of range, reset to default
    if (settings.heatctl.undershoot > SETTINGS_LIMIT_HEATCTL_UNDERSHOOT_MAX || settings.heatctl.undershoot < SETTINGS_LIMIT_HEATCTL_UNDERSHOOT_MIN) {
        settings.heatctl.undershoot = SETTINGS_DEFAULT_HEATCTL_UNDERSHOOT;
        modified = true;
    }

    // If temperature correction is out of range, reset to default
    if (settings.heatctl.temp_correction > SETTINGS_LIMIT_HEATCTL_TEMP_CORR_MAX || settings.heatctl.temp_correction < SETTINGS_LIMIT_HEATCTL_TEMP_CORR_MIN) {
        settings.heatctl.undershoot = SETTINGS_DEFAULT_HEATCTL_TEMP_CORR;
        modified = true;
    }

    // If BOOST interval is out of range, reset to default
    if (settings.heatctl.boost_intval > SETTINGS_LIMIT_HEATCTL_BOOST_INTVAL_MAX || settings.heatctl.boost_intval < SETTINGS_LIMIT_HEATCTL_BOOST_INTVAL_MIN) {
        settings.heatctl.boost_intval = SETTINGS_DEFAULT_HEATCTL_BOOST_INTVAL;
        modified = true;
    }

    // If Custom Temperature Timeout is out of range, reset to default
    if (settings.heatctl.custom_temp_timeout > SETTINGS_LIMIT_HEATCTL_CUSTOM_TEMP_TIMEOUT_MAX || settings.heatctl.custom_temp_timeout < SETTINGS_LIMIT_HEATCTL_CUSTOM_TEMP_TIMEOUT_MIN) {
            settings.heatctl.custom_temp_timeout = SETTINGS_DEFAULT_HEATCTL_CUSTOM_TEMP_TIMEOUT;
            modified = true;
    }

    // If there was a correction, assume that the settings data is
    // corrupted, so reset the brightness of the display to default.
    // This check is necessary since all possible values (0-255) are valid
    // for backlight level thus we cannot decide if it's corrupted or not.
    // At last, save the corrected values.
    if (modified) {
        settings.display.brightness = SETTINGS_DEFAULT_DISPLAY_BRIGHTNESS;
        settings.display.timeout_secs = SETTINGS_DEFAULT_DISPLAY_TIMEOUT_SECS;
        settings_save();
    }
}