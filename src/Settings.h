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

#include "Logger.h"

#include <ISettingsHandler.h>

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
    namespace HeatingController
    {
        constexpr auto Mode = 0;
        constexpr auto DaytimeTemp = 220;
        constexpr auto NightTimeTemp = 200;
        constexpr auto TargetTemp = NightTimeTemp;
        constexpr auto TempOvershoot = 5;
        constexpr auto TempUndershoot = 5;
        constexpr auto BoostInterval = 10;
        constexpr auto CustomTempTimeout = 120;
        constexpr auto TempCorrection = 0;
    }

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

    using SchedulerDayData = uint8_t[6];

    DECLARE_SETTINGS_STRUCT(SchedulerSettings)
    {
        uint8_t Enabled: 1;
        uint8_t: 0;
        SchedulerDayData DayData[7];
    };

    DECLARE_SETTINGS_STRUCT(DisplaySettings)
    {
        uint8_t Brightness = DefaultSettings::Display::Brightness;
        uint8_t TimeoutSecs = DefaultSettings::Display::TimeoutSecs;
    };

    DECLARE_SETTINGS_STRUCT(HeatingControllerSettings)
    {
        uint8_t Mode = DefaultSettings::HeatingController::Mode;

        // Temperature values in 0.1 Celsius
        int16_t DaytimeTemp = DefaultSettings::HeatingController::DaytimeTemp;
        int16_t NightTimeTemp = DefaultSettings::HeatingController::NightTimeTemp;

        // Target temperature settings
        int16_t TargetTemp = DefaultSettings::HeatingController::TargetTemp;
        std::time_t TargetTempSetTimestamp = 0;

        // Values for histeresis in 0.1 Celsius
        uint8_t Overshoot = DefaultSettings::HeatingController::TempOvershoot;
        uint8_t Undershoot = DefaultSettings::HeatingController::TempUndershoot;
        int8_t TempCorrection = DefaultSettings::HeatingController::TempCorrection;

        // Values in minutes
        uint8_t BoostIntervalMins = DefaultSettings::HeatingController::BoostInterval;
        uint16_t CustomTempTimeoutMins = DefaultSettings::HeatingController::CustomTempTimeout;
    };

    DECLARE_SETTINGS_STRUCT(Data)
    {
        uint32_t Crc32;
        uint8_t Version;

        SchedulerSettings Scheduler;
        DisplaySettings Display;
        HeatingControllerSettings HeatingController;
    };

    Data data;

    bool load();
    bool save();

    void loadDefaults();

private:
    ISettingsHandler& _handler;

    void check();
};
