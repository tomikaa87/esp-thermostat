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

namespace Devices::EnergyOptimizerSwitch
{
    auto uniqueId() { return PSTR("energy_optimizer_switch"); }
    auto commandTopic() { return PSTR("/energy_optimizer_switch/set"); }
    auto stateTopic() { return PSTR("/energy_optimizer_switch"); }
}

namespace Devices::CallingForHeatingSensor
{
    auto uniqueId() { return PSTR("calling_for_heating_sensor"); }
    auto stateTopic() { return PSTR("/calling_for_heating"); }
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
    , _energyOptimizerSwitch{
        _topicPrefix,
        Devices::EnergyOptimizerSwitch::stateTopic(),
        Devices::EnergyOptimizerSwitch::commandTopic(),
        _app.mqttClient()
    }
    , _callingForHeatingState{
        _topicPrefix,
        Devices::CallingForHeatingSensor::stateTopic(),
        _app.mqttClient()
    }
{
    if (!_settings.load()) {
        _log.warning_P("failed to load settings, restoring defaults");
    }

    setupRelayOutput();
    setupMqttComponentConfigs();
    setupMqttChangeHandlers();
    updateMqtt();
}

void FurnaceController::task()
{
    const uint32_t currentMillis = millis();
    const uint32_t deltaMillis = currentMillis - _lastTaskMillis;
    _lastTaskMillis = currentMillis;

    _app.task();

    _mqttUpdateTimer += deltaMillis;
    if (_mqttUpdateTimer >= 1000) {
        _mqttUpdateTimer = 0;
        updateMqtt();
    }

    bool callingForHeating{ false };

    for (auto& zone : _zones) {
        zone.task(deltaMillis);

        if (_settings.value().masterEnable) {
            if (zone.callingForHeating()) {
                callingForHeating = true;
            }
        }
    }

    _callingForHeatingState = callingForHeating ? 1 : 0;

    if (_settings.value().energyOptimizerEnabled) {
        for (auto& zone : _zones) {
            zone.handleFurnaceHeatingChanged(callingForHeating);
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

    _app.mqttClient().publish(
        [] {
            return HomeAssistant::makeConfigTopic(
                fromPstr("switch"),
                fromPstr(Devices::EnergyOptimizerSwitch::uniqueId())
            );
        },
        [&] {
            return HomeAssistant::makeSwitchConfig(
                fromPstr(PSTR("mdi:leaf")),
                fromPstr(PSTR("Energy Optimizer")),
                fromPstr(Devices::EnergyOptimizerSwitch::uniqueId()),
                _topicPrefix,
                fromPstr(Devices::EnergyOptimizerSwitch::commandTopic()),
                fromPstr(Devices::EnergyOptimizerSwitch::stateTopic()),
                [&](auto& config) {
                    HomeAssistant::addDeviceConfig(
                        config,
                        _app.config().firmwareVersion.toString()
                    );
                }
            );
        }
    );

    _app.mqttClient().publish(
        [] {
            return HomeAssistant::makeConfigTopic(
                fromPstr("sensor"),
                fromPstr(Devices::CallingForHeatingSensor::uniqueId())
            );
        },
        [&] {
            return HomeAssistant::makeSensorConfig(
                fromPstr(PSTR("mdi:radiator")),
                fromPstr(PSTR("Calling for heating")),
                fromPstr(Devices::CallingForHeatingSensor::uniqueId()),
                _topicPrefix,
                fromPstr(Devices::CallingForHeatingSensor::stateTopic()),
                "",
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
            _log.debug_P(PSTR("masterSwitch=%d"), value);
            _settings.value().masterEnable = value != 0;
            _settings.save();
        }
    );

    _energyOptimizerSwitch.setChangedHandler(
        [this](const auto value) {
            _log.debug_P(PSTR("energyOptimizerEnabled=%d"), value);
            _settings.value().energyOptimizerEnabled = value != 0;
            _settings.save();
        }
    );
}

void FurnaceController::updateMqtt()
{
    _masterSwitch = _settings.value().masterEnable;
    _energyOptimizerSwitch = _settings.value().energyOptimizerEnabled;
}
