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

#pragma once

#include "Config.h"

#include <HeatingZoneController.h>
#include <ISettingsHandler.h>
#include <Logger.h>

#include <array>
#include <cstdint>
#include <ctime>

namespace PersistentData
{

}

namespace Limits
{
    constexpr auto MinimumTemperature = 100;
    constexpr auto MaximumTemperature = 300;

    namespace HeatingController
    {
        constexpr auto DaytimeTempMax = MaximumTemperature;
        constexpr auto DaytimeTempMin = MinimumTemperature;
        constexpr auto NightTimeTempMax = MaximumTemperature;
        constexpr auto NightTimeTempMin = MinimumTemperature;
        constexpr auto TempOvershootMax = 10;
        constexpr auto TempOvershootMin = 1;
        constexpr auto TempUndershootMax = 10;
        constexpr auto TempUndershootMin = 1;
        constexpr auto BoostIntervalMax = 60;
        constexpr auto BoostIntervalMin = 5;
        constexpr auto TempCorrectionMax = 100;
        constexpr auto TempCorrectionMin = -100;
        constexpr auto CustomTempTimeoutMin = 0;
        constexpr auto CustomTempTimeoutMax = 1440;
    }
}

namespace DefaultSettings
{
    namespace Display
    {
        constexpr auto Brightness = 10;
        constexpr auto TimeoutSecs = 15;
    }
}

class Settings
{
public:
    explicit Settings(ISettingsHandler& handler);

    struct Heating
    {
        struct ZoneControllerSettings
        {
            HeatingZoneController::Configuration config{};
            HeatingZoneController::Schedule schedule{};
            HeatingZoneController::State state{};
        };

        std::array<ZoneControllerSettings, Config::ZoneCount> zones{{}};
    } heating;

    DECLARE_SETTINGS_STRUCT(System)
    {
        DECLARE_SETTINGS_STRUCT(Display)
        {
            uint8_t brightness = DefaultSettings::Display::Brightness;
            uint8_t timeoutSecs = DefaultSettings::Display::TimeoutSecs;
        };

        uint8_t maximumLogLevel{ static_cast<uint8_t>(Log::Severity::Info) };

        bool masterEnable{ false };
        bool energyOptimizerEnabled{ true };

        HeatingZoneController::DeciDegrees internalSensorOffset{ 0 };

        Display display;
    } system;

    bool load();
    bool save();

    void loadDefaults();

private:
    Logger _log{ "Settings" };
    ISettingsHandler& _handler;

    bool check();

    void dumpData() const;

    void registerHeatingSettings();
};
