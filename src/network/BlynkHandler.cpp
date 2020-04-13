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

#include "BlynkHandler.h"
#include "HeatingController.h"

#define ENABLE_DEBUG

#ifdef ENABLE_DEBUG
#define BLYNK_PRINT     Serial
#endif // ENABLE_DEBUG

#include <BlynkSimpleEsp8266.h>

static BlynkHandler* g_blynkHandler = nullptr;

#define HANDLE_BLYNK_WRITE(__PIN) BLYNK_WRITE(__PIN) { \
    if (g_blynkHandler) \
        g_blynkHandler->onVirtualPinUpdated(__PIN, param); \
}

#define HANDLE_BLYNK_READ(__PIN) BLYNK_READ(__PIN) { \
    if (g_blynkHandler) \
        g_blynkHandler->updateVirtualPin(__PIN); \
}

#define HANDLE_BLYNK_BUTTON_PRESS(__PIN) BLYNK_WRITE(__PIN) { \
    if (g_blynkHandler && param.asInt() == 1) \
        g_blynkHandler->onButtonPressed(__PIN); \
}

BLYNK_CONNECTED()
{
    if (g_blynkHandler)
        g_blynkHandler->onBlynkConnected();
}

#define PIN_INCREMENT_TEMP              V1
#define PIN_DECREMENT_TEMP              V2
#define PIN_ACTIVATE_BOOST              V3
#define PIN_DEACTIVATE_BOOST            V4

#define PIN_TARGET_TEMPERATURE          V127
#define PIN_DAYTIME_TEMPERATURE         V126
#define PIN_NIGHT_TIME_TEMPERATURE      V125
#define PIN_BOOST_STATE                 V124
#define PIN_CURRENT_TEMPERATURE         V123
#define PIN_HEATING_STATE               V122
#define PIN_BOOST_REMAINING_SECS        V121
#define PIN_MODE_SELECTOR               V120

#define PIN_NEXT_SWITCH_LABEL           V119
#define PIN_NEXT_SWITCH_TIME_LABEL      V118

HANDLE_BLYNK_BUTTON_PRESS(PIN_INCREMENT_TEMP)
HANDLE_BLYNK_BUTTON_PRESS(PIN_DECREMENT_TEMP)
HANDLE_BLYNK_BUTTON_PRESS(PIN_ACTIVATE_BOOST)
HANDLE_BLYNK_BUTTON_PRESS(PIN_DEACTIVATE_BOOST)

HANDLE_BLYNK_WRITE(PIN_MODE_SELECTOR)
HANDLE_BLYNK_WRITE(PIN_NIGHT_TIME_TEMPERATURE)
HANDLE_BLYNK_WRITE(PIN_DAYTIME_TEMPERATURE)
HANDLE_BLYNK_WRITE(PIN_TARGET_TEMPERATURE)

static WidgetTerminal gs_terminal{ V64 };

BlynkHandler::BlynkHandler(const char* appToken, HeatingController& heatingController)
    : _heatingController(heatingController)
{
    g_blynkHandler = this;
    Blynk.config(appToken, "blynk-server.home", 8080);
}

BlynkHandler::~BlynkHandler()
{
    g_blynkHandler = nullptr;
}

void BlynkHandler::task()
{
    Blynk.run();
    processValueUpdates();
    processButtonCallbackRequests();
}

void BlynkHandler::onBlynkConnected()
{
    _log.debug("connected");

    // LEDs must be updated manually
    updateVirtualPin(PIN_BOOST_STATE);
    updateVirtualPin(PIN_HEATING_STATE);
}

void BlynkHandler::onVirtualPinUpdated(int pin, const BlynkParam& param)
{
    _log.debug("virtual pin updated, pin: %d", pin);

    // Handle "real" data updates (not button presses)
    // here.

    // This callback should run as short as possible
    // to avoid Blynk disconnect errors

    switch (pin)
    {
        case PIN_MODE_SELECTOR:
            m_mode = param.asInt() - 1;
            break;

        case PIN_TARGET_TEMPERATURE:
            m_targetTemperature = param.asFloat();
            break;

        case PIN_NIGHT_TIME_TEMPERATURE:
            m_nightTimeTemperature = param.asFloat();
            break;

        case PIN_DAYTIME_TEMPERATURE:
            m_daytimeTemperature = param.asFloat();
            break;

        default:
            break;
    }
}

void BlynkHandler::onButtonPressed(int pin)
{
    // This callback should run as short as possible
    // to avoid Blynk disconnect errors

    switch (pin)
    {
        case PIN_INCREMENT_TEMP:
            m_callIncrementTempCb = true;
            break;

        case PIN_DECREMENT_TEMP:
            m_callDecrementTempCb = true;
            break;

        case PIN_ACTIVATE_BOOST:
            m_callActivateBoostCb = true;
            break;

        case PIN_DEACTIVATE_BOOST:
            m_callDeactivateBoostCb = true;
            break;

        default:
            break;
    }
}

void BlynkHandler::updateVirtualPin(int pin)
{
    _log.debug("updating virtual pin: %d", pin);

    char buf[10] = { 0 };

    switch (pin)
    {
        case PIN_TARGET_TEMPERATURE:
            floatToStr(m_targetTemperature, buf);
            Blynk.virtualWrite(pin, buf);
            break;

        case PIN_DAYTIME_TEMPERATURE:
            floatToStr(m_daytimeTemperature, buf);
            Blynk.virtualWrite(pin, buf);
            break;

        case PIN_NIGHT_TIME_TEMPERATURE:
            floatToStr(m_nightTimeTemperature, buf);
            Blynk.virtualWrite(pin, buf);
            break;

        case PIN_BOOST_STATE:
            Blynk.virtualWrite(PIN_BOOST_STATE, m_boostActive ? 255 : 0);
            break;

        case PIN_CURRENT_TEMPERATURE:
            floatToStr(m_currentTemperature, buf);
            Blynk.virtualWrite(PIN_CURRENT_TEMPERATURE, buf);
            break;

        case PIN_HEATING_STATE:
            Blynk.virtualWrite(PIN_HEATING_STATE, m_heatingActive ? 255 : 0);
            break;

        case PIN_BOOST_REMAINING_SECS:
        {
            int minutes = m_boostRemainingSecs / 60;
            int secs = m_boostRemainingSecs - minutes * 60;

            snprintf(buf, sizeof(buf), "%d:%02d", minutes, secs);

            Blynk.virtualWrite(PIN_BOOST_REMAINING_SECS, buf);
            break;
        }

        case PIN_MODE_SELECTOR:
            Blynk.virtualWrite(PIN_MODE_SELECTOR, m_mode + 1);
            break;

        default:
            break;
    }
}

void BlynkHandler::updateActiveTemperature(float celsius)
{
    m_targetTemperature = celsius;
    if (m_targetTemperature.changed())
        updateVirtualPin(PIN_TARGET_TEMPERATURE);
}

void BlynkHandler::updateDaytimeTemperature(float celsius)
{
    m_daytimeTemperature = celsius;
    if (m_daytimeTemperature.changed())
        updateVirtualPin(PIN_DAYTIME_TEMPERATURE);
}

void BlynkHandler::updateNightTimeTemperature(float celsius)
{
    m_nightTimeTemperature = celsius;
    if (m_nightTimeTemperature.changed())
        updateVirtualPin(PIN_NIGHT_TIME_TEMPERATURE);
}

void BlynkHandler::updateCurrentTemperature(float celsius)
{
    if (m_currentTemperature == celsius)
        return;

    m_currentTemperature = celsius;
    updateVirtualPin(PIN_CURRENT_TEMPERATURE);
}

void BlynkHandler::updateIsBoostActive(bool active)
{
    if (m_boostActive == active)
        return;

    m_boostActive = active;
    updateVirtualPin(PIN_BOOST_STATE);
}

void BlynkHandler::updateIsHeatingActive(bool active)
{
    if (m_heatingActive == active)
        return;

    m_heatingActive = active;
    updateVirtualPin(PIN_HEATING_STATE);
}

void BlynkHandler::updateBoostRemaining(uint32_t secs)
{
    if (m_boostRemainingSecs == secs)
        return;

    m_boostRemainingSecs = secs;
    updateVirtualPin(PIN_BOOST_REMAINING_SECS);
}

void BlynkHandler::updateMode(uint8_t mode)
{
    m_mode = mode;
    if (m_mode.changed())
        updateVirtualPin(PIN_MODE_SELECTOR);
}

void BlynkHandler::updateNextSwitch(uint8_t state, uint8_t weekday, uint8_t hour, uint8_t minute)
{
    if (state > 1 || weekday > 6 || hour > 23 || minute > 59)
    {
        Blynk.virtualWrite(PIN_NEXT_SWITCH_LABEL, "--");
        Blynk.virtualWrite(PIN_NEXT_SWITCH_TIME_LABEL, "--- --:--");
    }
    else
    {
        char buf[16] = { 0 };
        static const char Weekdays[7][4] = { "SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT" };

        snprintf(buf, sizeof(buf), "%s %2d:%02d", Weekdays[weekday], hour, minute);

        Blynk.virtualWrite(PIN_NEXT_SWITCH_LABEL, state == 0 ? "OFF" : "ON");
        Blynk.virtualWrite(PIN_NEXT_SWITCH_TIME_LABEL, buf);
    }
}

void BlynkHandler::terminalPrintln(const char* msg)
{
    gs_terminal.println(msg);
    gs_terminal.flush();
}

void BlynkHandler::processButtonCallbackRequests()
{
    if (m_callIncrementTempCb) {
        _heatingController.incTargetTemp();
    }

    if (m_callDecrementTempCb) {
        _heatingController.decTargetTemp();
    }

    if (m_callActivateBoostCb) {
        if (!_heatingController.isBoostActive()) {
            _heatingController.activateBoost();
        } else {
            _heatingController.extendBoost();
        }
    }

    if (m_callDeactivateBoostCb && _heatingController.isBoostActive()) {
        _heatingController.deactivateBoost();
    }

    m_callIncrementTempCb = false;
    m_callDecrementTempCb = false;
    m_callActivateBoostCb = false;
    m_callDeactivateBoostCb = false;
}

void BlynkHandler::processValueUpdates()
{
    if (m_mode.changed()) {
        _heatingController.setMode(static_cast<HeatingController::Mode>(m_mode.value()));
    }

    if (m_targetTemperature.changed()) {
        _heatingController.setTargetTemp(m_targetTemperature * 10);
    }

    if (m_nightTimeTemperature.changed()) {
        _heatingController.setNightTimeTemp(m_nightTimeTemperature * 10);
    }

    if (m_daytimeTemperature.changed()) {
        _heatingController.setDaytimeTemp(m_daytimeTemperature * 10);
    }
}

template <typename T, int size>
inline void BlynkHandler::floatToStr(const float f, T(&buf)[size])
{
    static_assert(size >= 4, "Output buffer is too small");
    snprintf(buf, size - 1, "%0.1f", f);
}
