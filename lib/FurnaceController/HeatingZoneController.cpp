#include "HeatingZoneController.h"

HeatingZoneController::HeatingZoneController(Configuration config)
    : _config{ std::move(config) }
{
}

void HeatingZoneController::setMode(const Mode mode)
{
    _mode = mode;
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

void HeatingZoneController::inputTemperature(DeciDegrees value)
{
}

void HeatingZoneController::setHighTargetTemperature(DeciDegrees value)
{
}

void HeatingZoneController::setLowTargetTemperature(DeciDegrees value)
{
}

void HeatingZoneController::overrideTargetTemperature(DeciDegrees value)
{
}

void HeatingZoneController::resetTargetTemperature()
{
}

bool HeatingZoneController::targetTemperatureOverrideActive() const
{
    return _overrideActive;
}

std::optional<HeatingZoneController::DeciDegrees> HeatingZoneController::targetTemperature() const
{
    return {};
}

bool HeatingZoneController::callingForHeating() const
{
    if (boostActive()) {
        return true;
    }

    return false;
}

void HeatingZoneController::task(uint32_t systemClockDeltaMs)
{
    if (boostActive()) {
        if (systemClockDeltaMs <= _requestedBoostTimeMs) {
            _requestedBoostTimeMs -= systemClockDeltaMs;
        } else {
            _requestedBoostTimeMs = 0;
        }
    }
}
