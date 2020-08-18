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

#include "Logger.h"
#include "TrackedVariable.h"

#include <Variant.h>

#include <functional>
#include <stdint.h>

class HeatingController;
class IBlynkHandler;

class Blynk
{
public:
    Blynk(IBlynkHandler& blynkHandler, HeatingController& heatingController);

    void task();

    void updateActiveTemperature(float celsius);
    void updateDaytimeTemperature(float celsius);
    void updateNightTimeTemperature(float celsius);
    void updateCurrentTemperature(float celsius);
    void updateIsBoostActive(bool active);
    void updateIsHeatingActive(bool active);
    void updateBoostRemaining(uint32_t secs);
    void updateMode(uint8_t mode);
    void updateNextSwitch(uint8_t state, uint8_t weekday, uint8_t hour, uint8_t minute);

    void terminalPrintln(const char* msg);

private:
    IBlynkHandler& _blynkHandler;
    HeatingController& _heatingController;
    Logger _log{ "Blynk" };

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

    void setupHandlers();

    void onConnected();

    void processButtonCallbackRequests();
    void processValueUpdates();

    template <typename T, int size>
    inline void floatToStr(const float f, T(&buf)[size]);
};
