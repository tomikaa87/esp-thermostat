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

#include "Settings.h"
#include "HeatingController.h"

#include <iomanip>
#include <sstream>

Settings::Settings(ISettingsHandler& handler)
    : _handler(handler)
{
    _handler.setDefaultsLoader([this](const ISettingsHandler::DefaultsLoadReason reason) {
        _log.warning_p(PSTR("defaults load requested from settings handler: reason=%d"), reason);
        loadDefaults();
    });

    _handler.registerSetting(data);

    load();
}

bool Settings::load()
{
    const auto ok = _handler.load();

    _log.info_p(PSTR("loading settings: ok=%d"), ok);

    dumpData();

    if (!check()) {
        _log.warning_p(PSTR("loaded settings corrected"));
        save();
    }

    return ok;
}

bool Settings::save()
{
    if (!check()) {
        _log.warning_p(PSTR("settings corrected before saving"));
    }

    dumpData();

    const auto ok = _handler.save();

    _log.info_p(PSTR("saving settings: ok=%d"), ok);

    return ok;
}

void Settings::loadDefaults()
{
    _log.info_p(PSTR("loading defaults"));

    data = {};

    if (!check()) {
        _log.warning_p(PSTR("loaded defaults corrected"));
    }
}

bool Settings::check()
{
    bool modified = false;

    // Reset Heat Control mode if it's corrupted
    if (data.HeatingController.Mode > static_cast<uint8_t>(HeatingController::Mode::Off)) {
        data.HeatingController.Mode = static_cast<uint8_t>(HeatingController::Mode::Off);
        modified = true;
    }

    // If daytime temp is out of range, reset to default
    if (data.HeatingController.DaytimeTemp > Limits::HeatingController::DaytimeTempMax || data.HeatingController.DaytimeTemp < Limits::HeatingController::DaytimeTempMin) {
        data.HeatingController.DaytimeTemp = DefaultSettings::HeatingController::DaytimeTemp;
        modified = true;
    }

    // If nighttime temp is out of range, reset to default
    if (data.HeatingController.NightTimeTemp > Limits::HeatingController::NightTimeTempMax || data.HeatingController.NightTimeTemp < Limits::HeatingController::NightTimeTempMin) {
        data.HeatingController.NightTimeTemp = DefaultSettings::HeatingController::NightTimeTemp;
        modified = true;
    }

    // If temperature overshoot is out of range, reset to default
    if (data.HeatingController.Overshoot > Limits::HeatingController::TempOvershootMax || data.HeatingController.NightTimeTemp < Limits::HeatingController::TempOvershootMin) {
        data.HeatingController.Overshoot = DefaultSettings::HeatingController::TempOvershoot;
        modified = true;
    }

    // If temperature undershoot is out of range, reset to default
    if (data.HeatingController.Undershoot > Limits::HeatingController::TempUndershootMax || data.HeatingController.Undershoot < Limits::HeatingController::TempUndershootMin) {
        data.HeatingController.Undershoot = DefaultSettings::HeatingController::TempUndershoot;
        modified = true;
    }

    // If temperature correction is out of range, reset to default
    if (data.HeatingController.TempCorrection > Limits::HeatingController::TempCorrectionMax || data.HeatingController.TempCorrection < Limits::HeatingController::TempCorrectionMin) {
        data.HeatingController.TempCorrection = DefaultSettings::HeatingController::TempCorrection;
        modified = true;
    }

    // If BOOST interval is out of range, reset to default
    if (data.HeatingController.BoostIntervalMins > Limits::HeatingController::BoostIntervalMax || data.HeatingController.BoostIntervalMins < Limits::HeatingController::BoostIntervalMin) {
        data.HeatingController.BoostIntervalMins = DefaultSettings::HeatingController::BoostInterval;
        modified = true;
    }

    // If Custom Temperature Timeout is out of range, reset to default
    if (data.HeatingController.CustomTempTimeoutMins > Limits::HeatingController::CustomTempTimeoutMax || data.HeatingController.CustomTempTimeoutMins < Limits::HeatingController::CustomTempTimeoutMin) {
            data.HeatingController.CustomTempTimeoutMins = DefaultSettings::HeatingController::CustomTempTimeout;
            modified = true;
    }

    // If there was a correction, assume that the settings data is
    // corrupted, so reset the brightness of the display to default.
    // This check is necessary since all possible values (0-255) are valid
    // for backlight level thus we cannot decide if it's corrupted or not.
    // At last, save the corrected values.
    if (modified) {
        data.Display.Brightness = DefaultSettings::Display::Brightness;
        data.Display.TimeoutSecs = DefaultSettings::Display::TimeoutSecs;
    }

    return !modified;
}

void Settings::dumpData() const
{
    _log.debug_p(PSTR("Display{ Brightness=%u, TimeoutSecs=%u }"),
        data.Display.Brightness,
        data.Display.TimeoutSecs
    );

    _log.debug_p(PSTR("HeatingController{ Mode=%u, DaytimeTemp=%d, NightTimeTemp=%d, TargetTemp=%d, TargetTempSetTimestamp=%ld, Overshoot=%u, Undershoot=%u, TempCorrection=%d, BoostIntervalMins=%u, CustomTempTimeputMins=%u }"),
        data.HeatingController.Mode,
        data.HeatingController.DaytimeTemp,
        data.HeatingController.NightTimeTemp,
        data.HeatingController.TargetTemp,
        data.HeatingController.TargetTempSetTimestamp,
        data.HeatingController.Overshoot,
        data.HeatingController.Undershoot,
        data.HeatingController.TempCorrection,
        data.HeatingController.BoostIntervalMins,
        data.HeatingController.CustomTempTimeoutMins
    );

    std::stringstream schDays;
    for (auto i = 0; i < 7; ++i) {
        schDays << std::to_string(i) << "=";
        schDays << std::hex << std::setw(2) << std::setfill('0');
        for (auto j = 0; j < 6; ++j) {
             schDays << static_cast<int>(data.Scheduler.DayData[i][j]);
        }
        schDays << std::resetiosflags << 'h';
        if (i < 6) {
            schDays << ", ";
        }
    }

    _log.debug_p(PSTR("Scheduler{ Enabled=%u, Days=[ %s ] }"),
        data.Scheduler.Enabled,
        schDays.str().c_str()
    );
}