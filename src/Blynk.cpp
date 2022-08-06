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

#ifdef IOT_ENABLE_BLYNK

#include "Blynk.h"
#include "HeatingController.h"
#include "Settings.h"

#include <IBlynkHandler.h>

#ifdef DEBUG_BLYNK_KEYPAD
#warning "Debug-mode Blynk keypad is enabled"

#include "Keypad.h"
#include "ui/Ui.h"
#endif // DEBUG_BLYNK_KEYPAD

namespace VirtualPins
{
    namespace Buttons
    {
        static constexpr auto IncrementTemp = 1;
        static constexpr auto DecrementTemp = 2;
        static constexpr auto ActivateBoost = 3;
        static constexpr auto DeactivateBoost = 4;
    }

    namespace Labels
    {
        static constexpr auto NextSwitchLabel = 119;
        static constexpr auto NextSwitchTimeLabel = 118;
    }

    static constexpr auto ModeSelector = 120;
    static constexpr auto BoostRemainingSeconds = 121;
    static constexpr auto HeatingState = 122;
    static constexpr auto CurrentTemperature = 123;
    static constexpr auto BoostState = 124;
    static constexpr auto NightTimeTemperature = 125;
    static constexpr auto DaytimeTemperature = 126;
    static constexpr auto TargetTemperature = 127;

    static constexpr auto Terminal = 64;

#ifdef DEBUG_BLYNK_KEYPAD
    namespace KeypadButtons
    {
        static constexpr auto Left = 110;
        static constexpr auto Right = 111;
        static constexpr auto Menu = 112;
        static constexpr auto Boost = 113;
        static constexpr auto Plus = 114;
        static constexpr auto Minus = 115;
    }
#endif // DEBUG_BLYNK_KEYPAD
}

Blynk::Blynk(
    IBlynkHandler& blynkHandler,
    HeatingController& heatingController,
    Ui& ui,
    Settings& settings
)
    : _blynkHandler(blynkHandler)
    , _heatingController(heatingController)
    , _ui(ui)
    , _settings(settings)
{
    if (!_settings.data.Scheduler.DisableBlynk) {
        setupHandlers();
    }
}

void Blynk::task()
{
    processValueUpdates();
    processButtonCallbackRequests();

#ifdef DEBUG_BLYNK_KEYPAD
    processKeypadButtonPresses();
#endif // DEBUG_BLYNK_KEYPAD
}

void Blynk::setupHandlers()
{
    _blynkHandler.setConnectedHandler([this] {
        onConnected();
    });

    //
    // Write handlers
    //

    _blynkHandler.setPinWrittenHandler(
        VirtualPins::ModeSelector,
        [this](const int pin, const Variant& value) {
            m_mode = static_cast<int>(value) - 1;
        }
    );

    _blynkHandler.setPinWrittenHandler(
        VirtualPins::TargetTemperature,
        [this](const int pin, const Variant& value) {
            _log.debug("TargetTemperature written: %f", static_cast<double>(value));
            m_targetTemperature = static_cast<double>(value);
        }
    );

    _blynkHandler.setPinWrittenHandler(
        VirtualPins::NightTimeTemperature,
        [this](const int pin, const Variant& value) {
            _log.debug("NightTimeTemperature written: %f", static_cast<double>(value));
            m_nightTimeTemperature = static_cast<double>(value);
        }
    );

    _blynkHandler.setPinWrittenHandler(
        VirtualPins::DaytimeTemperature,
        [this](const int pin, const Variant& value) {
            _log.debug("DaytimeTemperature written: %f", static_cast<double>(value));
            m_daytimeTemperature = static_cast<double>(value);
        }
    );

    //
    // Button handlers
    //

    _blynkHandler.setPinWrittenHandler(
        VirtualPins::Buttons::ActivateBoost,
        [this](const int pin, const Variant& value) {
            m_callActivateBoostCb = value == 1;
        }
    );

    _blynkHandler.setPinWrittenHandler(
        VirtualPins::Buttons::DeactivateBoost,
        [this](const int pin, const Variant& value) {
            m_callDeactivateBoostCb = value == 1;
        }
    );

    _blynkHandler.setPinWrittenHandler(
        VirtualPins::Buttons::IncrementTemp,
        [this](const int pin, const Variant& value) {
            m_callIncrementTempCb = value == 1;
        }
    );

    _blynkHandler.setPinWrittenHandler(
        VirtualPins::Buttons::DecrementTemp,
        [this](const int pin, const Variant& value) {
            m_callDecrementTempCb = value == 1;
        }
    );

    //
    // Read handlers
    //

    char buf[10] = { 0 };

    _blynkHandler.setPinReadHandler(
        VirtualPins::TargetTemperature,
        [this, &buf](const int) {
            floatToStr(m_targetTemperature, buf);
            return Variant{ buf };
        }
    );

    _blynkHandler.setPinReadHandler(
        VirtualPins::DaytimeTemperature,
        [this, &buf](const int) {
            floatToStr(m_daytimeTemperature, buf);
            return Variant{ buf };
        }
    );

    _blynkHandler.setPinReadHandler(
        VirtualPins::NightTimeTemperature,
        [this, &buf](const int) {
            floatToStr(m_nightTimeTemperature, buf);
            return Variant{ buf };
        }
    );

    _blynkHandler.setPinReadHandler(
        VirtualPins::CurrentTemperature,
        [this, &buf](const int) {
            floatToStr(m_currentTemperature, buf);
            return Variant{ buf };
        }
    );

    _blynkHandler.setPinReadHandler(
        VirtualPins::BoostState,
        [this](const int) {
            return Variant{ m_boostActive ? 255 : 0 };
        }
    );

    _blynkHandler.setPinReadHandler(
        VirtualPins::HeatingState,
        [this](const int) {
            return Variant{ m_heatingActive ? 255 : 0 };
        }
    );

    _blynkHandler.setPinReadHandler(
        VirtualPins::BoostRemainingSeconds,
        [this, &buf](const int) {
            int minutes = m_boostRemainingSecs / 60;
            int secs = m_boostRemainingSecs - minutes * 60;
            snprintf(buf, sizeof(buf), "%d:%02d", minutes, secs);
            return Variant{ buf };
        }
    );

    _blynkHandler.setPinReadHandler(
        VirtualPins::HeatingState,
        [this](const int) {
            return Variant{ m_mode + 1 };
        }
    );

#ifdef DEBUG_BLYNK_KEYPAD

    // Debug-mode keypad handlers

    _blynkHandler.setPinWrittenHandler(
        VirtualPins::KeypadButtons::Left,
        [this](const int, const Variant& value) {
            if (value == 1) {
                _keypadButton = VirtualPins::KeypadButtons::Left;
            }
        }
    );

    _blynkHandler.setPinWrittenHandler(
        VirtualPins::KeypadButtons::Right,
        [this](const int, const Variant& value) {
            if (value == 1) {
                _keypadButton = VirtualPins::KeypadButtons::Right;
            }
        }
    );

    _blynkHandler.setPinWrittenHandler(
        VirtualPins::KeypadButtons::Menu,
        [this](const int, const Variant& value) {
            if (value == 1) {
                _keypadButton = VirtualPins::KeypadButtons::Menu;
            }
        }
    );

    _blynkHandler.setPinWrittenHandler(
        VirtualPins::KeypadButtons::Boost,
        [this](const int, const Variant& value) {
            if (value == 1) {
                _keypadButton = VirtualPins::KeypadButtons::Boost;
            }
        }
    );

    _blynkHandler.setPinWrittenHandler(
        VirtualPins::KeypadButtons::Plus,
        [this](const int, const Variant& value) {
            if (value == 1) {
                _keypadButton = VirtualPins::KeypadButtons::Plus;
            }
        }
    );

    _blynkHandler.setPinWrittenHandler(
        VirtualPins::KeypadButtons::Minus,
        [this](const int, const Variant& value) {
            if (value == 1) {
                _keypadButton = VirtualPins::KeypadButtons::Minus;
            }
        }
    );

#endif // DEBUG_BLYNK_KEYPAD
}

void Blynk::onConnected()
{
    // LEDs must be updated manually
    _blynkHandler.writePin(VirtualPins::BoostState, Variant{ m_boostActive ? 255 : 0 });
    _blynkHandler.writePin(VirtualPins::HeatingState, Variant{ m_heatingActive ? 255 : 0 });
}

void Blynk::updateActiveTemperature(const float celsius)
{
    if (_settings.data.Scheduler.DisableBlynk) {
        return;
    }

    m_targetTemperature = celsius;
    if (m_targetTemperature.changed()) {
        _blynkHandler.writePin(
            VirtualPins::TargetTemperature,
            Variant{ m_targetTemperature }
        );
    }
}

void Blynk::updateDaytimeTemperature(const float celsius)
{
    if (_settings.data.Scheduler.DisableBlynk) {
        return;
    }

    m_daytimeTemperature = celsius;
    if (m_daytimeTemperature.changed()) {
        _blynkHandler.writePin(
            VirtualPins::DaytimeTemperature,
            Variant{ m_daytimeTemperature }
        );
    }
}

void Blynk::updateNightTimeTemperature(const float celsius)
{
    if (_settings.data.Scheduler.DisableBlynk) {
        return;
    }

    m_nightTimeTemperature = celsius;
    if (m_nightTimeTemperature.changed()) {
        _blynkHandler.writePin(
            VirtualPins::NightTimeTemperature,
            Variant{ m_nightTimeTemperature }
        );
    }
}

void Blynk::updateCurrentTemperature(const float celsius)
{
    if (_settings.data.Scheduler.DisableBlynk) {
        return;
    }

    m_currentTemperature = celsius;

    char buf[10] = { 0 };
    floatToStr(m_currentTemperature, buf);

    _blynkHandler.writePin(
        VirtualPins::CurrentTemperature,
        Variant{ buf }
    );
}

void Blynk::updateIsBoostActive(const bool active)
{
    if (_settings.data.Scheduler.DisableBlynk) {
        return;
    }

    if (m_boostActive == active)
        return;

    m_boostActive = active;

    _blynkHandler.writePin(
        VirtualPins::BoostState,
        Variant{ m_boostActive ? 255 : 0}
    );
}

void Blynk::updateIsHeatingActive(const bool active)
{
    if (_settings.data.Scheduler.DisableBlynk) {
        return;
    }

    if (m_heatingActive == active)
        return;

    m_heatingActive = active;

    _blynkHandler.writePin(
        VirtualPins::HeatingState,
        Variant{ m_heatingActive ? 255 : 0 }
    );
}

void Blynk::updateBoostRemaining(const uint32_t secs)
{
    if (_settings.data.Scheduler.DisableBlynk) {
        return;
    }

    if (m_boostRemainingSecs == secs)
        return;

    m_boostRemainingSecs = secs;

    const int m = m_boostRemainingSecs / 60;
    const int s = m_boostRemainingSecs - m * 60;
    char buf[10] = { 0 };
    snprintf(buf, sizeof(buf), "%d:%02d", m, s);

    _blynkHandler.writePin(
        VirtualPins::BoostRemainingSeconds,
        Variant{ buf }
    );
}

void Blynk::updateMode(const uint8_t mode)
{
    if (_settings.data.Scheduler.DisableBlynk) {
        return;
    }

    m_mode = mode;

    if (m_mode.changed()) {
        _blynkHandler.writePin(
            VirtualPins::ModeSelector,
            Variant{ m_mode + 1 }
        );
    }
}

void Blynk::updateNextSwitch(const uint8_t state, const uint8_t weekday, const uint8_t hour, const uint8_t minute)
{
    if (_settings.data.Scheduler.DisableBlynk) {
        return;
    }

    if (state > 1 || weekday > 6 || hour > 23 || minute > 59)
    {
        _blynkHandler.writePin(
            VirtualPins::Labels::NextSwitchLabel,
            Variant{ "--" }
        );

        _blynkHandler.writePin(
            VirtualPins::Labels::NextSwitchTimeLabel,
            Variant{ "--- --:--" }
        );
    }
    else
    {
        char buf[16] = { 0 };
        static const char Weekdays[7][4] = { "SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT" };

        snprintf(buf, sizeof(buf), "%s %2d:%02d", Weekdays[weekday], hour, minute);

        _blynkHandler.writePin(
            VirtualPins::Labels::NextSwitchLabel,
            Variant{ state == 0 ? "OFF" : "ON" }
        );

        _blynkHandler.writePin(
            VirtualPins::Labels::NextSwitchTimeLabel,
            Variant{ buf }
        );
    }
}

void Blynk::terminalPrintln(const char* msg)
{
    if (_settings.data.Scheduler.DisableBlynk) {
        return;
    }

    _blynkHandler.writeTerminal(VirtualPins::Terminal, msg);
}

void Blynk::processButtonCallbackRequests()
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

void Blynk::processValueUpdates()
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

#ifdef DEBUG_BLYNK_KEYPAD
void Blynk::processKeypadButtonPresses()
{
    switch (_keypadButton) {
        case VirtualPins::KeypadButtons::Left:
            _ui.handleKeyPress(Keypad::Keys::Left);
            break;

        case VirtualPins::KeypadButtons::Right:
            _ui.handleKeyPress(Keypad::Keys::Right);
            break;

        case VirtualPins::KeypadButtons::Menu:
            _ui.handleKeyPress(Keypad::Keys::Menu);
            break;

        case VirtualPins::KeypadButtons::Boost:
            _ui.handleKeyPress(Keypad::Keys::Boost);
            break;

        case VirtualPins::KeypadButtons::Plus:
            _ui.handleKeyPress(Keypad::Keys::Plus);
            break;

        case VirtualPins::KeypadButtons::Minus:
            _ui.handleKeyPress(Keypad::Keys::Minus);
            break;

        default:
            break;
    }

    _keypadButton = 0;
}
#endif // DEBUG_BLYNK_KEYPAD

template <typename T, int size>
inline void Blynk::floatToStr(const float f, T(&buf)[size])
{
    static_assert(size >= 4, "Output buffer is too small");
    snprintf(buf, size - 1, "%0.1f", f);
}

#endif