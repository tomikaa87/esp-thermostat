#include "HeatingZoneController.h"

HeatingZoneController::HeatingZoneController(Configuration config)
{
}

HeatingZoneController::Mode HeatingZoneController::mode() const
{
    return _mode;
}

bool HeatingZoneController::boostActive() const
{
    return false;
}

void HeatingZoneController::setHighTargetTemperature(DeciDegrees value)
{
}

void HeatingZoneController::setLowTargetTemperature(DeciDegrees value)
{
}

bool HeatingZoneController::targetTemperatureOverrideActive() const
{
    return _overrideActive;
}

HeatingZoneController::DeciDegrees HeatingZoneController::targetTemperature() const
{
    return {};
}

bool HeatingZoneController::callingForHeating() const
{
    return false;
}
