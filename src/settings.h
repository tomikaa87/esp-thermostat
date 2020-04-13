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

#include <cstdint>
#include <ctime>

#define SETTINGS_PACKED __attribute__((__packed__))

namespace PersistentData
{
    using SchedulerDayData = uint8_t[6];

    struct SchedulerSettings
    {
        uint8_t Enabled: 1;
        uint8_t: 0;
        SchedulerDayData DayData[7];
    } SETTINGS_PACKED;

    struct DisplaySettings
    {
        uint8_t Brightness;
        uint8_t TimeoutSecs;
    } SETTINGS_PACKED;

    struct HeatingControllerSettings
    {
        uint8_t Mode;

        // Temperature values in 0.1 Celsius
        int16_t DaytimeTemp;
        int16_t NightTimeTemp;

        // Target temperature settings
        int16_t TargetTemp;
        std::time_t TargetTempSetTimestamp;

        // Values for histeresis in 0.1 Celsius
        uint8_t Overshoot;
        uint8_t Undershoot;
        int8_t TempCorrection;

        // Values in minutes
        uint8_t BoostIntervalMins;
        uint16_t CustomTempTimeoutMins;
    } SETTINGS_PACKED;

    struct Settings
    {
        uint32_t Crc32;
        uint8_t Version;

        SchedulerSettings Scheduler;
        DisplaySettings Display;
        HeatingControllerSettings HeatingController;
    } SETTINGS_PACKED;
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
        constexpr auto CustomTempTimeout = 240;
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

    Settings();

    void load();
    void loadDefaults();
    void save();
    void saveHeatingControllerSettings();

    PersistentData::Settings Data;

private:
    Logger _log{ "Settings" };
    bool _aseEnabled = false;

    void check();
};
