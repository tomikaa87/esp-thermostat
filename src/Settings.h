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
        constexpr auto Brightness = 20;
        constexpr auto TimeoutSecs = 15;
    }
}

class Settings
{
public:
    static constexpr uint8_t DataVersion = 1;

    explicit Settings(ISettingsHandler& handler);

    DECLARE_SETTINGS_STRUCT(DisplaySettings)
    {
        uint8_t Brightness = DefaultSettings::Display::Brightness;
        uint8_t TimeoutSecs = DefaultSettings::Display::TimeoutSecs;
    };

    DECLARE_SETTINGS_STRUCT(HeatingZoneSettings)
    {
        HeatingZoneController::Configuration config{};
        HeatingZoneController::Schedule schedule{};
        HeatingZoneController::State state{};
    };

    DECLARE_SETTINGS_STRUCT(SystemSettings)
    {
        HeatingZoneController::DeciDegrees internalSensorOffset{ 0 };
    };

    DECLARE_SETTINGS_STRUCT(Data)
    {
        DisplaySettings display;
        SystemSettings system;
        std::array<HeatingZoneSettings, 5> heatingZones;
    };

    Data data;

    bool load();
    bool save();

    void loadDefaults();

private:
    Logger _log{ "Settings" };
    ISettingsHandler& _handler;

    bool check();

    void dumpData() const;
};
