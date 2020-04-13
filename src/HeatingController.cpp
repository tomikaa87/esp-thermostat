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
    Created on 2017-01-04
*/

#include "HeatingController.h"
#include "clock.h"
#include "config.h"
#include "extras.h"
#include "settings.h"

#include <Arduino.h>

#include "Peripherals.h"

#define TEMPERATURE_STEP	5

HeatingController::HeatingController(const Clock& clock)
    : _clock{ clock }
{
    _log.info("initializing");

    // Setup relay control pin
    digitalWrite(D8, LOW);
    pinMode(D8, OUTPUT);
}

void HeatingController::task()
{
    // Only Heat Control settings are auto-saved
    if (isModeSaveNeeded()) {
        _settingsChanged = false;
        settings_save_heatctl();
    }

    // Read temperature sensor and store it in tenths of degrees
    _sensorTemp = Peripherals::Sensors::MainTemperature::lastReading() / 10;

#ifdef HEATCL_DEBUG
    printf("heatctl: s_tmp=%d\r\n", sensor_temp);
    printf("heatctl: bst_act=%u\r\n", heatctl.boost_active);
    printf("heatctl: bst_end=%lu\r\n", heatctl.boost_end);
    printf("heatctl: clk_epch=%lu\r\n", clock_epoch);
    printf("heatctl: day_ovr=%u\r\n", heatctl.day_override);
    printf("heatctl: nig_ovr=%u\r\n", heatctl.night_override);
    printf("heatctl: heat_act=%u\r\n", heatctl.heating_active);
#endif

    if (_boostActive && _clock.utcTime() >= _boostEnd) {
        _boostActive = false;
        _boostDeactivated = true;

        _log.info("boost ended");
    }

    if (mode() == Mode::Normal) {
        // On schedule change, update target temperature
        const auto hasDtSched = hasDaytimeSchedule();

        if (hasDtSched != _usingDaytimeSchedule || _targetTemp == 0 || isCustomTempResetNeeded()) {
            _usingDaytimeSchedule = hasDtSched;

            if (_customTempSet) {
                _log.info("resetting custom temperature");
                _customTempSet = false;
            }

            if (_usingDaytimeSchedule) {
                _log.info("setting daytime temp as target");
                _targetTemp = settings.heatctl.day_temp;
            } else {
                _log.info("setting night time temp as target");
                _targetTemp = settings.heatctl.night_temp;
            }
        }
    }

    // Heating control works in the following way:
    // Start heating if it's inactive and:
    //	- boost is active
    //		OR
    //	- current temp <= target temp + undershoot
    //
    // Stop heating if it's active and:
    //	- boost was deactivated in the current iteration and the
    //	  current temperature >= target temperature
    //		OR
    //	- current temp >= target temp + overshoot
    //
    // Undershoot and overshoot are the values that give the system
    // hysteresis which avoids switching the heat on and off in a short
    // period of time.
    //
    // BOOST function turns on heating instantly for a given interval
    // without altering the set temperature values. After deactivating
    // BOOST the heating should be switched off when the current temperature
    // is higher than the target temperature, without adding the overshoot
    // value to it. This avoids unnecessary heating when the temperature
    // is already high enough.

    do {
        if (_heatingActive) {
            if (_boostActive) {
                break;
            }

            if (mode() == Mode::Off) {
                _log.info("stopping heating because Off mode");
                stopHeating();
                break;
            }

            if (_sensorTemp >= _targetTemp && _boostDeactivated) {
                _log.info("stopping heating because boost ended");
                stopHeating();
                break;
            }

            if (_sensorTemp >= _targetTemp + settings.heatctl.overshoot) {
                _log.info("stopping heating because of high temp");
                stopHeating();
            }
        } else {
            if (_boostActive) {
                _log.info("starting heating because boost started");
                startHeating();
                break;
            }

            if (mode() == Mode::Off) {
                break;
            }

            if (_sensorTemp <= _targetTemp - settings.heatctl.undershoot) {
                _log.info("starting heating because of low temp");
                startHeating();
            }
        }
    } while (false);

    _boostDeactivated = false;
}

HeatingController::Mode HeatingController::mode() const
{
    if (isBoostActive())
        return Mode::Boost;

    return static_cast<Mode>(settings.heatctl.mode);
}

void HeatingController::setMode(Mode mode)
{
    _log.infof("setting mode to %d", static_cast<int>(mode));

    if (mode == Mode::Boost) {
        if (!isBoostActive()) {
            activateBoost();
        }
    } else {
        if (isBoostActive()) {
            deactivateBoost();
        }

        settings.heatctl.mode = static_cast<uint8_t>(mode);

        markSettingsChanged();
    }
}

bool HeatingController::isActive() const
{
    return _heatingActive;
}

bool HeatingController::isBoostActive() const
{
    return _boostActive;
}

void HeatingController::activateBoost()
{
    _log.info("activating boost");

    if (isBoostActive()) {
        _log.warning("boost already active");
        return;
    }

    _boostEnd = _clock.utcTime() + settings.heatctl.boost_intval * 60;
    _boostActive = true;
}

void HeatingController::deactivateBoost()
{
    _log.info("deactivating boost");

    if (!isBoostActive()) {
        _log.warning("boost already deactivated");
        return;
    }

    _boostActive = false;
    _boostDeactivated = true;
}

void HeatingController::extendBoost()
{
    _boostEnd += settings.heatctl.boost_intval * 60;
    _log.infof("extending boost, end: %ld", _boostEnd);

    // TODO load max value from settings
    if (boostRemaining() > 4 * 3600) {
        _log.warningf("maximum boost time reached: %ld", 4 * 3600);
        _boostEnd = _clock.utcTime() + 4 * 3600;
    }
}

std::time_t HeatingController::boostRemaining() const
{
    if (!isBoostActive()) {
        return 0;
    }

    return _boostEnd - _clock.utcTime();
}

HeatingController::TenthsOfDegrees HeatingController::currentTemp() const
{
    return _sensorTemp;
}

HeatingController::TenthsOfDegrees HeatingController::targetTemp() const
{
    return _targetTemp;
}

void HeatingController::setTargetTemp(TenthsOfDegrees temp)
{
    _log.infof("setting target temp: %d", temp);

    _targetTemp = temp;
    clampTargetTemp();
    markCustomTempSet();
}

void HeatingController::incTargetTemp()
{
    _log.info("increasing target temp");

    if (_targetTemp >= SETTINGS_TEMP_MAX) {
        return;
    }

    _targetTemp += TEMPERATURE_STEP;
    clampTargetTemp();
    markCustomTempSet();
}

void HeatingController::decTargetTemp()
{
    _log.info("decreasing target temp");

    if (_targetTemp <= SETTINGS_TEMP_MIN) {
        return;
    }

    _targetTemp -= TEMPERATURE_STEP;
    clampTargetTemp();
    markCustomTempSet();
}

HeatingController::TenthsOfDegrees HeatingController::daytimeTemp() const
{
    return settings.heatctl.day_temp;
}

void HeatingController::setDaytimeTemp(TenthsOfDegrees temp)
{
    _log.infof("setting daytime temp: %d", temp);

    settings.heatctl.day_temp = Extras::clampValue(
        temp,
        SETTINGS_LIMIT_HEATCTL_DAY_TEMP_MIN,
        SETTINGS_LIMIT_HEATCTL_DAY_TEMP_MAX
    );
}

HeatingController::TenthsOfDegrees HeatingController::nightTimeTemp() const
{
    return settings.heatctl.night_temp;
}

void HeatingController::setNightTimeTemp(TenthsOfDegrees temp)
{
    _log.infof("setting night time temp: %d", temp);

    settings.heatctl.night_temp = Extras::clampValue(
        temp,
        SETTINGS_LIMIT_HEATCTL_NIGHT_TEMP_MIN,
        SETTINGS_LIMIT_HEATCTL_NIGHT_TEMP_MAX
    );
}

bool HeatingController::hasDaytimeSchedule() const
{
    const auto localTime = _clock.localTime();
    const auto t = gmtime(&localTime);
    return scheduledStateAt(t->tm_wday, t->tm_hour, t->tm_min) == State::On;
}

HeatingController::NextTransition HeatingController::nextTransition() const
{
    const auto localTime = _clock.localTime();
    const auto t = gmtime(&localTime);

    uint8_t wd = t->tm_wday;
    uint8_t h = t->tm_hour;
    uint8_t m = t->tm_min;

    const auto currentState = scheduledStateAt(wd, h, m);

    m = m >= 30 ? 30 : 0;

    uint8_t origWd = wd;
    uint8_t origH = h;
    uint8_t origM = m;

    NextTransition nt;

    do {
        const auto nextState = scheduledStateAt(wd, h, m);

        if (nextState != currentState) {
            nt.state = nextState;
            nt.weekday = wd;
            nt.hour = h;
            nt.minute = m;
            break;
        }

        m += 30;
        if (m == 60) {
            m = 0;
            if (++h == 24) {
                h = 0;
                if (++wd == 7)
                    wd = 0;
            }
        }
    } while (wd != origWd || h != origH || m != origM);

    return nt;
}

HeatingController::State HeatingController::scheduledStateAt(uint8_t weekday, uint8_t hour, uint8_t min) const
{
    const uint8_t intvalIdx = calculate_schedule_intval_idx(hour, min);
    const uint8_t bitIdx = intvalIdx & 0b111;
    const uint8_t byteIdx = intvalIdx >> 3;
    const uint8_t mask = 1 << bitIdx;

    return ((settings.schedule.days[weekday][byteIdx] & mask) > 0)
        ? State::On
        : State::Off;
}

void HeatingController::markSettingsChanged()
{
    _settingsChanged = true;
    _settingsLastChanged = _clock.utcTime();
}

void HeatingController::markCustomTempSet()
{
    _log.debug("custom temp set");

    _customTempSet = true;
    _setTempLastChanged = _clock.utcTime();
}

void HeatingController::clampTargetTemp()
{
    if (_usingDaytimeSchedule) {
        _targetTemp = Extras::clampValue(
            _targetTemp,
            SETTINGS_LIMIT_HEATCTL_DAY_TEMP_MIN,
            SETTINGS_LIMIT_HEATCTL_DAY_TEMP_MAX
        );
    } else {
        _targetTemp = Extras::clampValue(
            _targetTemp,
            SETTINGS_LIMIT_HEATCTL_NIGHT_TEMP_MIN,
            SETTINGS_LIMIT_HEATCTL_NIGHT_TEMP_MAX
        );
    }
}

void HeatingController::startHeating()
{
    _log.info("activating relay");

    _heatingActive = true;
    digitalWrite(D8, HIGH);
}

void HeatingController::stopHeating()
{
    _log.info("deactivating relay");

    _heatingActive = false;
    digitalWrite(D8, LOW);
}

bool HeatingController::isCustomTempResetNeeded() const
{
    if (!_customTempSet || settings.heatctl.custom_temp_timeout == 0) {
        return false;
    }

    return _clock.utcTime() >= (_setTempLastChanged 
        + settings.heatctl.custom_temp_timeout * 60);
}

bool HeatingController::isModeSaveNeeded() const
{
    // Mode change should be saved after a few seconds.
    // This delay could spare EEPROM write cycles if the mode is being
    // changed in rapid successions.

    if (_settingsLastChanged && (_settingsLastChanged + 10 <= _clock.utcTime())) {
        return true;
    }

    return false;
}