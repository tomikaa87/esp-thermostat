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

#include <iomanip>
#include <sstream>

namespace Layout
{
    constexpr auto BaseAddress{ ISettingsHandler::ReservedAreaSize };

    namespace System
    {
        constexpr auto BaseAddress{ Layout::BaseAddress };
        constexpr auto AddressStep{ 32u };

        static_assert(BaseAddress >= Layout::BaseAddress);
        static_assert(AddressStep > sizeof(Settings::System));
    }

    namespace Heating
    {
        constexpr auto BaseAddress{ System::BaseAddress + System::AddressStep };
        constexpr auto AddressStep{ 128u };
        constexpr auto NextBaseAddress{ BaseAddress + AddressStep * Config::ZoneCount };

        static_assert(BaseAddress >= System::BaseAddress + System::AddressStep);
        static_assert(AddressStep > sizeof(Settings::Heating::ZoneControllerSettings));

        namespace Configuration
        {
            constexpr auto AddressStep{ 32u };
            static_assert(AddressStep > sizeof(HeatingZoneController::Configuration));
        }

        namespace Schedule
        {
            constexpr auto AddressStep{ 64u };
            static_assert(AddressStep > sizeof(HeatingZoneController::Schedule));
        }

        namespace State
        {
            constexpr auto AddressStep{ 32u };
            static_assert(AddressStep > sizeof(HeatingZoneController::State));
        }
    }
}

namespace
{
    template <typename T>
    [[nodiscard]] bool registerSetting(
        ISettingsHandler& handler,
        T& setting,
        const std::size_t address,
        Logger& log,
        const char* name
    )
    {
        auto ok = handler.registerSetting(setting, address);

        log.log_P(
            ok ? Log::Severity::Debug : Log::Severity::Error,
            PSTR("setting registration '%s': %s ref=%p, address=%u, size=%u"),
            name,
            ok ? "succeeded" : "FAILED",
            &setting,
            address,
            sizeof(T)
        );

        return ok;
    }
}

Settings::Settings(ISettingsHandler& handler)
    : _handler(handler)
{
    _handler.setDefaultsLoader([this](const ISettingsHandler::DefaultsLoadReason reason) {
        _log.warning_P(PSTR("defaults load requested from settings handler: reason=%d"), reason);
        loadDefaults();
    });

    if (!registerSetting(_handler, system, Layout::System::BaseAddress, _log, "System")) {
        abort();
    }

    registerHeatingSettings();

    load();
}

bool Settings::load()
{
    const auto ok = _handler.load();

    _log.info_P(PSTR("loading settings: ok=%d"), ok);

    dumpData();

    if (!check()) {
        _log.warning_P(PSTR("loaded settings corrected"));
        save();
    }

    return ok;
}

bool Settings::save()
{
    // if (!check()) {
    //     _log.warning_P(PSTR("settings corrected before saving"));
    // }

    dumpData();

    const auto ok = _handler.save() != ISettingsHandler::SaveResult::Error;
    _log.info_P(PSTR("saving settings: ok=%d"), ok);

    return ok;
}

void Settings::loadDefaults()
{
    _log.info_P(PSTR("loading defaults"));

    const auto ok = _handler.save(true) != ISettingsHandler::SaveResult::Error;
    _log.info_P(PSTR("saving default settings: ok=%d"), ok);

    // if (!check()) {
    //     _log.warning_P(PSTR("loaded defaults corrected"));
    // }
}

bool Settings::check()
{
    bool modified = false;

    // // Reset Heat Control mode if it's corrupted
    // if (data.HeatingController.Mode > static_cast<uint8_t>(HeatingController::Mode::Off)) {
    //     data.HeatingController.Mode = static_cast<uint8_t>(HeatingController::Mode::Off);
    //     modified = true;
    // }

    // // If daytime temp is out of range, reset to default
    // if (data.HeatingController.DaytimeTemp > Limits::HeatingController::DaytimeTempMax || data.HeatingController.DaytimeTemp < Limits::HeatingController::DaytimeTempMin) {
    //     data.HeatingController.DaytimeTemp = DefaultSettings::HeatingController::DaytimeTemp;
    //     modified = true;
    // }

    // // If nighttime temp is out of range, reset to default
    // if (data.HeatingController.NightTimeTemp > Limits::HeatingController::NightTimeTempMax || data.HeatingController.NightTimeTemp < Limits::HeatingController::NightTimeTempMin) {
    //     data.HeatingController.NightTimeTemp = DefaultSettings::HeatingController::NightTimeTemp;
    //     modified = true;
    // }

    // // If temperature overshoot is out of range, reset to default
    // if (data.HeatingController.Overshoot > Limits::HeatingController::TempOvershootMax || data.HeatingController.NightTimeTemp < Limits::HeatingController::TempOvershootMin) {
    //     data.HeatingController.Overshoot = DefaultSettings::HeatingController::TempOvershoot;
    //     modified = true;
    // }

    // // If temperature undershoot is out of range, reset to default
    // if (data.HeatingController.Undershoot > Limits::HeatingController::TempUndershootMax || data.HeatingController.Undershoot < Limits::HeatingController::TempUndershootMin) {
    //     data.HeatingController.Undershoot = DefaultSettings::HeatingController::TempUndershoot;
    //     modified = true;
    // }

    // // If temperature correction is out of range, reset to default
    // if (data.HeatingController.TempCorrection > Limits::HeatingController::TempCorrectionMax || data.HeatingController.TempCorrection < Limits::HeatingController::TempCorrectionMin) {
    //     data.HeatingController.TempCorrection = DefaultSettings::HeatingController::TempCorrection;
    //     modified = true;
    // }

    // // If BOOST interval is out of range, reset to default
    // if (data.HeatingController.BoostIntervalMins > Limits::HeatingController::BoostIntervalMax || data.HeatingController.BoostIntervalMins < Limits::HeatingController::BoostIntervalMin) {
    //     data.HeatingController.BoostIntervalMins = DefaultSettings::HeatingController::BoostInterval;
    //     modified = true;
    // }

    // // If Custom Temperature Timeout is out of range, reset to default
    // if (data.HeatingController.CustomTempTimeoutMins > Limits::HeatingController::CustomTempTimeoutMax || data.HeatingController.CustomTempTimeoutMins < Limits::HeatingController::CustomTempTimeoutMin) {
    //         data.HeatingController.CustomTempTimeoutMins = DefaultSettings::HeatingController::CustomTempTimeout;
    //         modified = true;
    // }

    // // If there was a correction, assume that the settings data is
    // // corrupted, so reset the brightness of the display to default.
    // // This check is necessary since all possible values (0-255) are valid
    // // for backlight level thus we cannot decide if it's corrupted or not.
    // // At last, save the corrected values.
    // if (modified) {
    //     data.Display.brightness = DefaultSettings::Display::Brightness;
    //     data.Display.timeoutSecs = DefaultSettings::Display::timeoutSecs;
    // }

    return !modified;
}

void Settings::dumpData() const
{
    _log.info_P(
        PSTR("System{ masterEnable=%u, energyOptimizerEnabled=%u }"),
        system.masterEnable,
        system.energyOptimizerEnabled
    );

    _log.info_P(
        PSTR("System.Display{ brightness=%u, timeoutSecs=%u }"),
        system.display.brightness,
        system.display.timeoutSecs
    );

    auto i = 0u;
    for (const auto& zone : heating.zones) {
        _log.info(
            "System.Heating.Zones[%u].Configuration{ overrideTimeoutSeconds=%u, boostInitialDurationSeconds=%u, boostExtensionDurationSeconds=%u, heatingStartDelaySeconds=%u, heatingOvershoot=%u, heatingUndershoot=%u, holidayModeTemperature=%u }",
            i,
            zone.config.overrideTimeoutSeconds,
            zone.config.boostInitialDurationSeconds,
            zone.config.boostExtensionDurationSeconds,
            zone.config.heatingStartDelaySeconds,
            zone.config.heatingOvershoot,
            zone.config.heatingUndershoot,
            zone.config.holidayModeTemperature
        );

        _log.info(
            "System.Heating.Zones[%u].State{ mode=%u, highTargetTemperature=%u, lowTargetTemperature=%u }",
            i,
            zone.state.mode,
            zone.state.highTargetTemperature,
            zone.state.lowTargetTemperature
        );

        static_assert(
            std::is_same_v<HeatingZoneController::Schedule, std::array<uint8_t, 6 * 7>>,
            "Dumping code must be adjusted to the Schedule type"
        );

        for (auto day = 0u; day < 7; ++day) {
            char bits[49]{}; // 48 + \0

            const auto dayOffset = day * 6u;
            for (auto byteIdx = 0u; byteIdx < 6u; ++byteIdx) {
                const auto b = zone.schedule[dayOffset + byteIdx];
                for (auto bitIdx = 0; bitIdx < 8; ++bitIdx) {
                    bits[byteIdx * 8 + bitIdx] = (b & (1 << (7 - bitIdx)) ? '1' : '0');
                }
            }

            _log.info(
                "System.Heating.Zones[%u].Schedule[%u]{ %s }",
                i,
                day,
                bits
            );
        }

        ++i;
    }
}

void Settings::registerHeatingSettings()
{
    auto nextBaseAddress{ Layout::Heating::BaseAddress };

    auto i = 0;
    for (auto& s : heating.zones) {
        _log.debug_P(
            PSTR("registering heating settings, index=%d, baseAddress=%u"),
            i++,
            nextBaseAddress
        );

        if (
            !registerSetting(
                _handler,
                s.config,
                nextBaseAddress,
                _log,
                "ZoneControllerSettings::Configuration"
            )
        ) {
            abort();
        }
        if (
            !registerSetting(
                _handler,
                s.schedule,
                nextBaseAddress + Layout::Heating::Configuration::AddressStep,
                _log,
                "ZoneControllerSettings::Schedule"
            )
        ) {
            abort();
        }

        if (
            !registerSetting(
                _handler,
                s.state,
                nextBaseAddress
                    + Layout::Heating::Configuration::AddressStep
                    + Layout::Heating::Schedule::AddressStep,
                _log,
                "ZoneControllerSettings::State"
            )
        ) {
            abort();
        }

        nextBaseAddress += Layout::Heating::AddressStep;
    }
}