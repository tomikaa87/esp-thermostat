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

#include "SystemClock.h"
#include "drivers/MCP7940N.h"
#include "Peripherals.h"

using rtc = Peripherals::Clock::Rtc;

SystemClock::SystemClock()
{
    _log.info("initializing");

    if (rtc::getPowerFailFlag()) {
        _log.warning("power failure detected");
        rtc::clearPowerFailFlag();
    }

    rtc::setBatteryEnabled(true);
    rtc::setOscillatorEnabled(true);

    // TODO this check is bogus
    if (!rtc::isOscillatorRunning()) {
        _log.warning("RTC oscillator has stopped");
    }
}

void SystemClock::task()
{
    if (_lastRtcSync == 0 || _epoch - _lastRtcSync > RtcSyncIntervalSec) {
        _log.info("automatic update from RTC triggered");
        updateFromRtc();
    }
}

void ICACHE_RAM_ATTR SystemClock::timerIsr()
{
    if (++_isrCounter == 3125) {
        _isrCounter = 0;
        ++_epoch;
    }
}

std::time_t SystemClock::localTime() const
{
    auto localTime = _epoch + _localTimeOffsetMinutes * 60;
    if (isDst(localTime)) {
        localTime += _localTimeDstOffsetMinutes * 60;
    }

    return localTime;
}

std::time_t SystemClock::utcTime() const
{
    return _epoch;
}

void SystemClock::setUtcTime(const std::time_t t)
{
    _log.info("setting UTC time: %ld", t);

    _epoch = t;
    updateRtc();
}

void SystemClock::updateFromRtc()
{
    const auto dt = rtc::getDateTime();

    struct tm rtcTm;
    rtcTm.tm_hour = dt.hours;
    rtcTm.tm_min = dt.minutes;
    rtcTm.tm_sec = dt.seconds;
    rtcTm.tm_mday = dt.date;
    rtcTm.tm_mon = dt.month - 1;     // RTC: 1-12, C: 0-11
    rtcTm.tm_year = dt.year + 100;   // RTC: 0-99, C: 1900 + value

    _log.info("time updated from RTC (UTC): %d-%02d-%02d %d:%02d:%02d",
        rtcTm.tm_year + 1900,
        rtcTm.tm_mon + 1,
        rtcTm.tm_mday,
        rtcTm.tm_hour,
        rtcTm.tm_min,
        rtcTm.tm_sec
    );

    _epoch = mktime(&rtcTm);
    _lastRtcSync = _epoch;

    _log.info("epoch after updating: %ld", _epoch);
}

void SystemClock::updateRtc()
{
    _log.info("updating RTC, epoch: %ld", _epoch);

    const auto tm = gmtime(&_epoch);

    const rtc::DateTime dt{
        static_cast<uint8_t>(tm->tm_year - 100),
        static_cast<uint8_t>(tm->tm_mon + 1),
        static_cast<uint8_t>(tm->tm_mday),
        static_cast<uint8_t>(tm->tm_wday + 1),
        static_cast<uint8_t>(tm->tm_hour),
        static_cast<uint8_t>(tm->tm_min),
        static_cast<uint8_t>(tm->tm_sec)
    };

    rtc::setDateTime(dt);

    _log.info("RTC update finished, current time (UTC): %d-%02d-%02d %d:%02d:%02d",
        tm->tm_year + 1900,
        tm->tm_mon + 1,
        tm->tm_mday,
        tm->tm_hour,
        tm->tm_min,
        tm->tm_sec
    );
}

bool SystemClock::isDst(const std::time_t t)
{
    const auto tm = gmtime(&t);

    // Before March or after October
    if (tm->tm_mon < 2 || tm->tm_mon > 9)
        return false;

    // After March and before October
    if (tm->tm_mon > 2 && tm->tm_mon < 9)
        return true;

    const auto previousSunday = tm->tm_mday - tm->tm_wday;

    // Sunday after March 25th
    if (tm->tm_mon == 2)
        return previousSunday >= 25;

    // Sunday before October 25th
    if (tm->tm_mon == 9)
        return previousSunday < 25;

    // This should never happen
    return false;
}