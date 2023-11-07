#include "HeatingZoneController.h"

#include <iostream>

namespace
{
    constexpr HeatingZoneController::DeciDegrees FailSafeLowTarget{ 100 };
    constexpr HeatingZoneController::DeciDegrees FailSafeHighTarget{ 300 };
}

HeatingZoneController::HeatingZoneController(Configuration config)
    : _lastInputTemperature{ FailSafeHighTarget }
    , _highTargetTemperature{ FailSafeLowTarget }
    , _lowTargetTemperature{ FailSafeLowTarget }
{
    updateConfig(std::move(config));
}

bool HeatingZoneController::updateConfig(Configuration config)
{
    if (
        config.heatingOvershoot > 20
        || config.heatingUndershoot > 20
        || config.boostInitialDurationSeconds > 3600
        || config.boostExtensionDurationSeconds > 3600
        || config.holidayModeTemperature > FailSafeHighTarget
        || config.overrideTimeoutSeconds > 4 * 3600
    ) {
        return false;
    }

    _config = std::move(config);

    updateCallForHeatByTemperature();

    return true;
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

    updateCallForHeatByTemperature();
}

void HeatingZoneController::setMode(const Mode mode)
{
    _mode = mode;

    if (_mode == Mode::Off) {
        resetTargetTemperature();
    }

    updateCallForHeatByTemperature();
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
    updateCallForHeatByTemperature();
}

void HeatingZoneController::setHighTargetTemperature(const DeciDegrees value)
{
    _highTargetTemperature = value;
}

void HeatingZoneController::setLowTargetTemperature(const DeciDegrees value)
{
    _lowTargetTemperature = value;
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

bool HeatingZoneController::callingForHeating() const
{
    if (boostActive()) {
        return true;
    }

    if (_mode == Mode::Auto || _mode == Mode::Holiday) {
        return _callForHeatingByTemperature;
    }

    return false;
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
}

void HeatingZoneController::updateCallForHeatByTemperature()
{
    if (_mode == Mode::Off) {
        _callForHeatingByTemperature = false;
        return;
    }

    DeciDegrees calculatedTargetTemperature{};
    if (const auto t = targetTemperature(); t.has_value()) {
        calculatedTargetTemperature = t.value();
    }

    std::cout << __func__
        << ": calculatedTargetTemperature=" << calculatedTargetTemperature
        << ", _callForHeatingByTemperature=" << _callForHeatingByTemperature
        << ", _lastInputTemperature=" << _lastInputTemperature
        << ", _config.heatingOvershoot=" << _config.heatingOvershoot
        << ", _config.heatingUndershoot=" << _config.heatingUndershoot
        << '\n';

    if (_callForHeatingByTemperature) {
        const auto target = calculatedTargetTemperature + _config.heatingOvershoot;

        std::cout << __func__
            << ": target=" << target
            << '\n';

        if (
            _lastInputTemperature >= target
            || _lastInputTemperature >= FailSafeHighTarget
        ) {
            _callForHeatingByTemperature = false;
        }
    } else {
        const auto target = calculatedTargetTemperature - _config.heatingUndershoot;

        std::cout << __func__
            << ": target=" << target
            << '\n';

        if (
            _lastInputTemperature <= target
            || _lastInputTemperature <= FailSafeLowTarget
        ) {
            _callForHeatingByTemperature = true;
        }
    }
}

HeatingZoneController::DeciDegrees HeatingZoneController::targetTemperatureBySchedule() const
{
    if (_scheduleDataDay > 6 || _scheduleDataByte > 5) {
        return _lowTargetTemperature;
    }

    if (_config.scheduleData[_scheduleDataDay * 6 + _scheduleDataByte] & _scheduleDataMask) {
        return _highTargetTemperature;
    }

    return _lowTargetTemperature;
}
