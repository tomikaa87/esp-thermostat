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

Settings::Settings()
{
    load();
}

void Settings::load()
{
    _log.info("loading");

    const auto read = Peripherals::Storage::EERAM::read(
        0,
        reinterpret_cast<uint8_t*>(&Data),
        sizeof(Data)
    );

    if (!read) {
        _log.error("EERAM read failed");
    }

    {
        const auto block = _log.debugBlock("read data: ");
        for (auto i = 0u; i < sizeof(Data); ++i) {
            _log.debug("%02xh ", reinterpret_cast<const uint8_t*>(&Data)[i]);
        }
    }

    const auto calculatedCrc32 = crc32(
        reinterpret_cast<const uint8_t*>(&Data) + sizeof(Data.Crc32),
        sizeof(Data) - sizeof(Data.Crc32)
    );

    if (calculatedCrc32 != Data.Crc32) {
        _log.warning("data checksum error, calculated: %08xh, loaded: %08xh",
            calculatedCrc32,
            Data.Crc32
        );

        loadDefaults();
        save();
    }

    check();
}

void Settings::save()
{
    _log.info("saving ");

    Data.Version = DataVersion;
    Data.Crc32 = crc32(
        reinterpret_cast<const uint8_t*>(&Data) + sizeof(Data.Crc32),
        sizeof(Data) - sizeof(Data.Crc32)
    );

    _log.debug("calculated checksum: %08xh", Data.Crc32);

    {
        const auto block = _log.debugBlock("writing data: ");
        for (auto i = 0u; i < sizeof(Data); ++i) {
            _log.debug("%02xh ", reinterpret_cast<const uint8_t*>(&Data)[i]);
        }
    }

    const auto written = Peripherals::Storage::EERAM::write(
        0,
        reinterpret_cast<uint8_t*>(&Data),
        sizeof(Data)
    );

    if (!written) {
        _log.error("EERAM write failed, written %u of %u Bytes");
        return;
    }

    if (!_aseEnabled) {
        _log.debug("settings changed, enabling ASE");

        _aseEnabled = true;
        Peripherals::Storage::EERAM::setAseEnabled(true);
    }
}

void Settings::saveHeatingControllerSettings()
{
    // Currently we save settings to the RTC SRAM, no need to do partial writes
    // to save EEPROM program cycles.
    save();
}

void Settings::check()
{
    bool modified = false;

    // Reset Heat Control mode if it's corrupted
    if (Data.HeatingController.Mode > static_cast<uint8_t>(HeatingController::Mode::Off)) {
        Data.HeatingController.Mode = static_cast<uint8_t>(HeatingController::Mode::Off);
        modified = true;
    }

    // If daytime temp is out of range, reset to default
    if (Data.HeatingController.DaytimeTemp > Limits::HeatingController::DaytimeTempMax || Data.HeatingController.DaytimeTemp < Limits::HeatingController::DaytimeTempMin) {
        Data.HeatingController.DaytimeTemp = DefaultSettings::HeatingController::DaytimeTemp;
        modified = true;
    }

    // If nighttime temp is out of range, reset to default
    if (Data.HeatingController.NightTimeTemp > Limits::HeatingController::NightTimeTempMax || Data.HeatingController.NightTimeTemp < Limits::HeatingController::NightTimeTempMin) {
        Data.HeatingController.NightTimeTemp = DefaultSettings::HeatingController::NightTimeTemp;
        modified = true;
    }

    // If temperature overshoot is out of range, reset to default
    if (Data.HeatingController.Overshoot > Limits::HeatingController::TempOvershootMax || Data.HeatingController.NightTimeTemp < Limits::HeatingController::TempOvershootMin) {
        Data.HeatingController.Overshoot = DefaultSettings::HeatingController::TempOvershoot;
        modified = true;
    }

    // If temperature undershoot is out of range, reset to default
    if (Data.HeatingController.Undershoot > Limits::HeatingController::TempUndershootMax || Data.HeatingController.Undershoot < Limits::HeatingController::TempUndershootMin) {
        Data.HeatingController.Undershoot = DefaultSettings::HeatingController::TempUndershoot;
        modified = true;
    }

    // If temperature correction is out of range, reset to default
    if (Data.HeatingController.TempCorrection > Limits::HeatingController::TempCorrectionMax || Data.HeatingController.TempCorrection < Limits::HeatingController::TempCorrectionMin) {
        Data.HeatingController.TempCorrection = DefaultSettings::HeatingController::TempCorrection;
        modified = true;
    }

    // If BOOST interval is out of range, reset to default
    if (Data.HeatingController.BoostIntervalMins > Limits::HeatingController::BoostIntervalMax || Data.HeatingController.BoostIntervalMins < Limits::HeatingController::BoostIntervalMin) {
        Data.HeatingController.BoostIntervalMins = DefaultSettings::HeatingController::BoostInterval;
        modified = true;
    }

    // If Custom Temperature Timeout is out of range, reset to default
    if (Data.HeatingController.CustomTempTimeoutMins > Limits::HeatingController::CustomTempTimeoutMax || Data.HeatingController.CustomTempTimeoutMins < Limits::HeatingController::CustomTempTimeoutMin) {
            Data.HeatingController.CustomTempTimeoutMins = DefaultSettings::HeatingController::CustomTempTimeout;
            modified = true;
    }

    // If there was a correction, assume that the settings data is
    // corrupted, so reset the brightness of the display to default.
    // This check is necessary since all possible values (0-255) are valid
    // for backlight level thus we cannot decide if it's corrupted or not.
    // At last, save the corrected values.
    if (modified) {
        Data.Display.Brightness = DefaultSettings::Display::Brightness;
        Data.Display.TimeoutSecs = DefaultSettings::Display::TimeoutSecs;
        save();
    }
}

void Settings::loadDefaults()
{
    _log.warning("loading default settings");

    Data.Display.Brightness = DefaultSettings::Display::Brightness;
    Data.Display.TimeoutSecs = DefaultSettings::Display::TimeoutSecs;

    Data.HeatingController.BoostIntervalMins = DefaultSettings::HeatingController::BoostInterval;
    Data.HeatingController.CustomTempTimeoutMins = DefaultSettings::HeatingController::CustomTempTimeout;
    Data.HeatingController.DaytimeTemp = DefaultSettings::HeatingController::DaytimeTemp;
    Data.HeatingController.Mode = DefaultSettings::HeatingController::Mode;
    Data.HeatingController.NightTimeTemp = DefaultSettings::HeatingController::NightTimeTemp;
    Data.HeatingController.Overshoot = DefaultSettings::HeatingController::TempOvershoot;
    Data.HeatingController.TargetTemp = DefaultSettings::HeatingController::TargetTemp;
    Data.HeatingController.TargetTempSetTimestamp = 0;
    Data.HeatingController.TempCorrection = DefaultSettings::HeatingController::TempCorrection;
}