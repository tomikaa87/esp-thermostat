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
    Created on 2017-01-12
*/

#pragma once

#include "TrackedVariable.h"

#include <functional>
#include <stdint.h>

class BlynkParam;

class BlynkHandler
{
public:
    BlynkHandler(const char* appToken/*, const char* wifiSSID, const char* wifiPassword*/);
    ~BlynkHandler();

    void task();

    void onBlynkConnected();
    void onVirtualPinUpdated(int pin, const BlynkParam& param);
    void onButtonPressed(int pin);
    void updateVirtualPin(int pin);

    void updateActiveTemperature(float celsius);
    void updateDaytimeTemperature(float celsius);
    void updateNightTimeTemperature(float celsius);
    void updateCurrentTemperature(float celsius);
    void updateIsBoostActive(bool active);
    void updateIsHeatingActive(bool active);
    void updateBoostRemaining(uint32_t secs);
    void updateMode(uint8_t mode);
    void updateNextSwitch(uint8_t state, uint8_t weekday, uint8_t hour, uint8_t minute);

    using Callback = std::function<void()>;

    void setIncrementTempCallback(Callback&& cb) { m_incrementTempCb = std::move(cb); }
    void setDecrementTempCallback(Callback&& cb) { m_decrementTempCb = std::move(cb); }
    void setActivateBoostCallback(Callback&& cb) { m_activateBoostCb = std::move(cb); }
    void setDeactivateBoostCallback(Callback&& cb) { m_deactivateBoostCb = std::move(cb); }

    void setTargetTemperatureChangedCallback(std::function<void(float)>&& cb) { m_targetTemperatureChangedCb = std::move(cb); }
    void setDaytimeTemperatureChangedCallback(std::function<void(float)>&& cb) { m_daytimeTemperatureChangedCb = std::move(cb); }
    void setNightTimeTemperatureChangedCallback(std::function<void(float)>&& cb) { m_nightTimeTemperatureChangedCb = std::move(cb); }

    void setModeChangedCallback(std::function<void(uint8_t)>&& cb) { m_modeChangedCallback = std::move(cb); }

    void terminalPrintln(const char* msg);

private:
    Callback m_incrementTempCb;
    Callback m_decrementTempCb;
    Callback m_activateBoostCb;
    Callback m_deactivateBoostCb;
    std::function<void(float)> m_targetTemperatureChangedCb;
    std::function<void(float)> m_daytimeTemperatureChangedCb;
    std::function<void(float)> m_nightTimeTemperatureChangedCb;
    std::function<void(uint8_t)> m_modeChangedCallback;

    bool m_callIncrementTempCb = false;
    bool m_callDecrementTempCb = false;
    bool m_callActivateBoostCb = false;
    bool m_callDeactivateBoostCb = false;

    float m_currentTemperature = 0;

    bool m_boostActive = false;
    bool m_heatingActive = false;

    TrackedVariable<uint8_t> m_mode = 0xff;
    TrackedVariable<float> m_targetTemperature = 0;
    TrackedVariable<float> m_daytimeTemperature = 0;
    TrackedVariable<float> m_nightTimeTemperature = 0;

    uint32_t m_boostRemainingSecs = 0xffffffff;

    void processButtonCallbackRequests();
    void processValueUpdates();

    template <typename T, int size>
    inline void floatToStr(const float f, T(&buf)[size]);
};
