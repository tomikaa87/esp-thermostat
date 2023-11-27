#include "FurnaceController.h"

#include "Extras.h"
#include "HomeAssistant.h"

#include <Arduino.h>

namespace
{
    constexpr auto RelayOutputPin{ D8 };
}

namespace Devices::MasterSwitch
{
    auto uniqueId() { return PSTR("master_switch"); }
    auto commandTopic() { return PSTR("/master_switch/set"); }
    auto stateTopic() { return PSTR("/master_switch"); }
}

FurnaceController::FurnaceController(const ApplicationConfig& appConfig)
    : _appConfig{ appConfig }
    , _app{ _appConfig }
    , _settings{ _app.settings().registerSetting<Settings>(32) }
    , _zones{
        HeatingZone{ 0, _app },
        HeatingZone{ 1, _app },
        HeatingZone{ 2, _app },
        HeatingZone{ 10, _app },
        HeatingZone{ 11, _app }
    }
    , _topicPrefix{
        HomeAssistant::makeUniqueId()
    }
    , _masterSwitch{
        _topicPrefix,
        Devices::MasterSwitch::stateTopic(),
        Devices::MasterSwitch::commandTopic(),
        _app.mqttClient()
    }
{
    if (!_settings.load()) {
        _log.warning_P("failed to load settings, restoring defaults");
    }

    setupRelayOutput();
    setupMqttComponentConfigs();
    setupMqttChangeHandlers();
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

void FurnaceController::setupMqttComponentConfigs()
{
    using namespace Extras;

    _app.mqttClient().publish(
        [] {
            return HomeAssistant::makeConfigTopic(
                fromPstr("switch"),
                fromPstr(Devices::MasterSwitch::uniqueId())
            );
        },
        [&] {
            return HomeAssistant::makeSwitchConfig(
                fromPstr(PSTR("mdi:power")),
                fromPstr(PSTR("Master Enable")),
                fromPstr(Devices::MasterSwitch::uniqueId()),
                _topicPrefix,
                fromPstr(Devices::MasterSwitch::commandTopic()),
                fromPstr(Devices::MasterSwitch::stateTopic()),
                [&](auto& config) {
                    HomeAssistant::addDeviceConfig(
                        config,
                        _app.config().firmwareVersion.toString()
                    );
                }
            );
        }
    );
}

void FurnaceController::setupMqttChangeHandlers()
{
    _masterSwitch.setChangedHandler(
        [this](const auto value) {
            _log.debug_P(PSTR("setupMqttChangeHandlers: masterSwitch=%d"), value);
        }
    );
}

void FurnaceController::updateMqtt()
{
}
