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
#include "Peripherals.h"

#include <cstring>

#include <coredecls.h> // crc32()

Settings::Settings(ISettingsHandler& handler)
    : _handler(handler)
{
    _handler.setDefaultsLoader([this](ISettingsHandler::DefaultsLoadReason) {
        loadDefaults();
    });

    _handler.registerSetting(data);
}

bool Settings::load()
{
    auto ok = _handler.load();
    check();
    return ok;
}

bool Settings::save()
{
    check();
    return _handler.save();
}

void Settings::loadDefaults()
{
    data = {};
    check();
}

void Settings::check()
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
        save();
    }
}