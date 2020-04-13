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
    Created on 2016-12-30
*/

#pragma once

#include "Logger.h"

#include <ctime>

class SystemClock
{
public:
    static constexpr auto NtpSyncIntervalSec = 1800;
    static constexpr auto RtcSyncIntervalSec = 60;

    SystemClock();

    void task();
    void timerIsr();

    std::time_t localTime() const;
    std::time_t utcTime() const;

    void setUtcTime(std::time_t t);

private:
    Logger _log{ "SystemClock" };
    std::time_t _epoch = 0;
    std::time_t _lastRtcSync = 0;
    bool _ntpSyncing = false;
    bool _rtcSynced = false;
    int _isrCounter = 0;
    int _localTimeOffsetMinutes = 60;
    int _localTimeDstOffsetMinutes = 60;

    void updateFromRtc();
    void updateRtc();

    static bool isDst(std::time_t t);
};