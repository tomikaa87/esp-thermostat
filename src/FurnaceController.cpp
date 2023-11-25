#include "FurnaceController.h"

#include <Arduino.h>

namespace
{
    constexpr auto RelayOutputPin{ D8 };
}

FurnaceController::FurnaceController(const ApplicationConfig& appConfig)
    : _appConfig{ appConfig }
    , _app{ _appConfig }
    , _zones{
        HeatingZone{ 0, _app },
#ifndef TEST_BUILD
        HeatingZone{ 1, _app },
        HeatingZone{ 2, _app },
        HeatingZone{ 10, _app },
        HeatingZone{ 11, _app }
#endif
    }
{
    setupRelayOutput();
}

void FurnaceController::task()
{
    const uint32_t currentMillis = millis();
    const uint32_t deltaMillis = currentMillis - _lastTaskMillis;
    _lastTaskMillis = currentMillis;

    _app.task();

    bool callingForHeating{ false };

    for (auto& zone : _zones) {
        zone.task(deltaMillis);

        if (zone.callingForHeating()) {
            callingForHeating = true;
        }
    }

    setRelayOutputActive(callingForHeating);
}

void FurnaceController::setupRelayOutput() const
{
    digitalWrite(RelayOutputPin, LOW);
    pinMode(RelayOutputPin, OUTPUT);
}

void FurnaceController::setRelayOutputActive(const bool active)
{
    if (active == _relayOutputActive) {
        return;
    }

    _relayOutputActive = active;

    _log.info_P(PSTR("%s: active=%d"), __func__, active);

    digitalWrite(RelayOutputPin, active ? HIGH : LOW);
}