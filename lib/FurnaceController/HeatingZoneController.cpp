#include "HeatingZoneController.h"

namespace
{
    constexpr HeatingZoneController::DeciDegrees FailSafeLowTarget{ 100 };
    constexpr HeatingZoneController::DeciDegrees FailSafeHighTarget{ 300 };
    constexpr uint32_t OpenWindowLockoutDurationMs{ 10 * 60 * 1000 };
}

HeatingZoneController::HeatingZoneController(
    Configuration& config,
    Schedule& schedule
)
    : _config{ config }
    , _schedule{ schedule }
    , _lastInputTemperature{ FailSafeHighTarget }
{
}

void HeatingZoneController::updateDateTime(
    const int dayOfWeek,
    const int hour,
    const int minute
)
{
    if (hour > 23 || minute > 59 || dayOfWeek > 6) {
        return;
    }

    const auto intervalIndex = (hour & 0b11111) << 1 | (minute >= 30 ? 1 : 0);

    _scheduleDataDay = dayOfWeek;
    _scheduleDataByte = intervalIndex >> 3;
    _scheduleDataMask = 1 << (intervalIndex & 0b111);
}

void HeatingZoneController::setMode(const Mode mode)
{
    _mode = mode;

    if (_mode == Mode::Off) {
        resetTargetTemperature();
    }

    _stateChanged = true;
}

HeatingZoneController::Mode HeatingZoneController::mode() const
{
    return _mode;
}

void HeatingZoneController::startOrExtendBoost()
{
    if (boostActive()) {
        _requestedBoostTimeMs += _config.boostExtensionDurationSeconds * 1000;
    } else {
        _requestedBoostTimeMs = _config.boostInitialDurationSeconds * 1000;
    }
}

void HeatingZoneController::stopBoost()
{
    _requestedBoostTimeMs = 0;
}

bool HeatingZoneController::boostActive() const
{
    return _requestedBoostTimeMs > 0;
}

uint32_t HeatingZoneController::boostRemainingSeconds() const
{
    return _requestedBoostTimeMs / 1000;
}

void HeatingZoneController::inputTemperature(const DeciDegrees value)
{
    _lastInputTemperature = value;
}

void HeatingZoneController::setHighTargetTemperature(const DeciDegrees value)
{
    _highTargetTemperature = value;
    _stateChanged = true;
}

HeatingZoneController::DeciDegrees HeatingZoneController::highTargetTemperature() const
{
    return _highTargetTemperature;
}

void HeatingZoneController::setLowTargetTemperature(const DeciDegrees value)
{
    _lowTargetTemperature = value;
    _stateChanged = true;
}

HeatingZoneController::DeciDegrees HeatingZoneController::lowTargetTemperature() const
{
    return _lowTargetTemperature;
}

void HeatingZoneController::overrideTargetTemperature(const DeciDegrees value)
{
    if (_mode == Mode::Off) {
        return;
    }

    _overrideRemainingMs = _config.overrideTimeoutSeconds * 1000;
    _overrideTemperature = value;
}

void HeatingZoneController::resetTargetTemperature()
{
    _overrideRemainingMs = 0;
}

bool HeatingZoneController::targetTemperatureOverrideActive() const
{
    return _overrideRemainingMs > 0;
}

uint32_t HeatingZoneController::targetTemperatureOverrideRemainingSeconds() const
{
    return _overrideRemainingMs / 1000;
}

std::optional<HeatingZoneController::DeciDegrees> HeatingZoneController::targetTemperature() const
{
    if (targetTemperatureOverrideActive()) {
        return _overrideTemperature;
    }

    switch (_mode) {
        case Mode::Off:
            break;
        case Mode::Auto:
            return targetTemperatureBySchedule();
        case Mode::Holiday:
            return _config.holidayModeTemperature;
    }

    return {};
}

void HeatingZoneController::setWindowOpened(const bool open)
{
    if (!open && _windowOpen) {
        _openWindowLockoutRemainingMs = OpenWindowLockoutDurationMs;
    } else if (open) {
        _openWindowLockoutRemainingMs = 0;
    }

    _windowOpen = open;
}

bool HeatingZoneController::windowOpened() const
{
    return _windowOpen;
}

bool HeatingZoneController::callingForHeating()
{
    if (_windowOpen) {
        return false;
    }

    if (boostActive()) {
        return true;
    }

    if (openWindowLockoutActive()) {
        return false;
    }

    if (_mode != Mode::Auto && _mode != Mode::Holiday) {
        return false;
    }

    DeciDegrees calculatedTargetTemperature{};
    if (const auto t = targetTemperature(); t.has_value()) {
        calculatedTargetTemperature = t.value();
    }

    if (_callForHeatingByTemperature) {
        const auto target = calculatedTargetTemperature + _config.heatingOvershoot;

        if (
            _lastInputTemperature >= target
            || _lastInputTemperature >= FailSafeHighTarget
        ) {
            _callForHeatingByTemperature = false;
        }
    } else {
        // If the furnace for this zone is already heating, don't wait
        // for the temperature to fall below the undershoot threshold.
        // This should optimize the energy consumption because the system
        // is already warm.
        const auto target =
            _furnaceHeating
                ? calculatedTargetTemperature
                : calculatedTargetTemperature - _config.heatingUndershoot;

        if (
            _lastInputTemperature <= target
            || _lastInputTemperature <= FailSafeLowTarget
        ) {
            _callForHeatingByTemperature = true;
        }
    }

    return _callForHeatingByTemperature;
}

void HeatingZoneController::task(const uint32_t systemClockDeltaMs)
{
    if (boostActive()) {
        if (systemClockDeltaMs <= _requestedBoostTimeMs) {
            _requestedBoostTimeMs -= systemClockDeltaMs;
        } else {
            _requestedBoostTimeMs = 0;
        }
    }

    if (targetTemperatureOverrideActive()) {
        if (systemClockDeltaMs <= _overrideRemainingMs) {
            _overrideRemainingMs -= systemClockDeltaMs;
        } else {
            _overrideRemainingMs = 0;
        }
    }

    if (openWindowLockoutActive()) {
        if (systemClockDeltaMs <= _openWindowLockoutRemainingMs) {
            _openWindowLockoutRemainingMs -= systemClockDeltaMs;
        } else {
            _openWindowLockoutRemainingMs = 0;
        }
    }
}

void HeatingZoneController::loadState(const State& state)
{
    // Decomposition ensures we won't forget about new fields after they've added
    const auto& [
        mode,
        highTargetTemperature,
        lowTargetTemperature
    ] = state;

    _stateChanged = false;

    _mode = mode;
    _highTargetTemperature = highTargetTemperature;
    _lowTargetTemperature = lowTargetTemperature;
}

HeatingZoneController::State HeatingZoneController::saveState()
{
    State state;

    _stateChanged = false;

    // Decomposition ensures we won't forget about new fields after they've added
    auto& [
        mode,
        highTargetTemperature,
        lowTargetTemperature
    ] = state;

    mode = _mode;
    highTargetTemperature = _highTargetTemperature;
    lowTargetTemperature = _lowTargetTemperature;

    return state;
}

bool HeatingZoneController::stateChanged() const
{
    return _stateChanged;
}

void HeatingZoneController::handleFurnaceHeatingChanged(const bool heating)
{
    _furnaceHeating = heating;
}

bool HeatingZoneController::openWindowLockoutActive() const
{
    return _openWindowLockoutRemainingMs > 0;
}

uint32_t HeatingZoneController::openWindowLockoutRemainingMs() const
{
    return _openWindowLockoutRemainingMs;
}

HeatingZoneController::DeciDegrees HeatingZoneController::targetTemperatureBySchedule() const
{
    if (_scheduleDataDay > 6 || _scheduleDataByte > 5) {
        return _lowTargetTemperature;
    }

    if (_schedule[_scheduleDataDay * 6 + _scheduleDataByte] & _scheduleDataMask) {
        return _highTargetTemperature;
    }

    return _lowTargetTemperature;
}
