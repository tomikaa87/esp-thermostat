#include "HeatingZoneController.h"

HeatingZoneController::HeatingZoneController(Configuration config)
{
}

void HeatingZoneController::setMode(Mode mode)
{
}

HeatingZoneController::Mode HeatingZoneController::mode() const
{
    return _mode;
}

void HeatingZoneController::startOrExtendBoost()
{
}

void HeatingZoneController::stopBoost()
{
}

bool HeatingZoneController::boostActive() const
{
    return false;
}

uint32_t HeatingZoneController::boostRemainingSeconds() const
{
    return 0;
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
    return false;
}

void HeatingZoneController::task(uint32_t systemClockMillis)
{
}
