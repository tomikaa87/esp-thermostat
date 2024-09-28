#include "FurnaceController.h"

#include "Extras.h"
#include "HomeAssistant.h"
#include "TemperatureSensor.h"

#include "ui/Model.h"

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

namespace
{
    namespace Detail
    {
        template <std::size_t... Indices>
        [[nodiscard]] constexpr std::array<HeatingZone, sizeof...(Indices)> createZones(
            const auto zoneIds,
            CoreApplication& app,
            std::array<Settings::Heating::ZoneControllerSettings, sizeof...(Indices)>& settings,
            std::index_sequence<Indices...>
        )
        {
            return std::array{
                HeatingZone{
                    std::get<Indices>(zoneIds),
                    app,
                    HeatingZone::SettingDependencies{
                        .state = settings[Indices].state,
                        .configuration = settings[Indices].config,
                        .schedule = settings[Indices].schedule
                    }
                }...
            };
        }
    }

    template <unsigned... ZoneIds>
    [[nodiscard]] constexpr auto createZones(CoreApplication& app, Settings& settings)
    {
        static_assert(sizeof...(ZoneIds) == Config::ZoneCount, "Zone count mismatch");

        return Detail::createZones(
            std::make_tuple(ZoneIds...),
            app,
            settings.heating.zones,
            std::make_index_sequence<sizeof...(ZoneIds)>()
        );
    }
}

FurnaceController::FurnaceController(
    CoreApplication& application,
    const ApplicationConfig& appConfig,
    Settings& settings,
    UI::Model& uiModel
)
    : _appConfig{ appConfig }
    , _app{ application }
    , _settings{ settings }
    , _uiModel{ uiModel }
    , _zones{ createZones<0, 1, 2, 3, 10, 11>(_app, _settings) }
    , _temperatureSensor{ _settings }
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
    _log.debug_P(PSTR("stack memory usage: %u B"), sizeof(FurnaceController));

    setupRelayOutput();
    setupMqttComponentConfigs();
    setupMqttChangeHandlers();
    updateMqtt();
}

void FurnaceController::task(const uint32_t deltaMillis)
{
    _app.task();

    _mqttUpdateTimer += deltaMillis;
    if (_mqttUpdateTimer >= 1000) {
        _mqttUpdateTimer = 0;
        updateMqtt();
    }

    bool callingForHeating{ false };

    _clockUpdateTimer += deltaMillis;
    if (_clockUpdateTimer >= 1000) {
        _clockUpdateTimer = 0;

        const time_t localTime{ _app.systemClock().localTime() };
        const auto* t{ gmtime(&localTime) };

        for (auto& zone : _zones) {
            zone.controller().updateDateTime(t->tm_wday, t->tm_hour, t->tm_min);
        }
    }

    for (auto& zone : _zones) {
        zone.task(deltaMillis);

        if (_settings.system.masterEnable) {
            if (zone.callingForHeating()) {
                callingForHeating = true;
            }
        }
    }

    _callingForHeatingState = callingForHeating ? 1 : 0;

    if (_settings.system.energyOptimizerEnabled) {
        for (auto& zone : _zones) {
            zone.handleFurnaceHeatingChanged(callingForHeating);
        }
    }

    setRelayOutputActive(callingForHeating);

    _temperatureSensor.task();

    updateUiModel();
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
            _settings.system.masterEnable = value != 0;
        }
    );

    _energyOptimizerSwitch.setChangedHandler(
        [this](const auto value) {
            _log.debug_P(PSTR("energyOptimizerEnabled=%d"), value);
            _settings.system.energyOptimizerEnabled = value != 0;
        }
    );
}

void FurnaceController::updateMqtt()
{
    _masterSwitch = _settings.system.masterEnable;
    _energyOptimizerSwitch = _settings.system.energyOptimizerEnabled;
}

void FurnaceController::updateUiModel()
{
    for (auto i = 0u; i < Config::ZoneCount; ++i) {
        auto& zone = _zones[i];
        auto& zoneModel = _uiModel.zones[i];

        // TODO only for debugging
        // zone.controller().inputTemperature(_temperatureSensor.read() / 10);

        zoneModel.targetTemperature = zone.controller().targetTemperature();
        zoneModel.currentTemperature = zone.controller().lastInputTemperature();
        zoneModel.zoneNumber = zone.index();
        zoneModel.status = [&] {
            using Status = UI::Model::Zone::Status;
            if (zone.controller().boostActive()) {
                return Status::Boost;
            }
            if (zone.callingForHeating()) {
                return Status::Heating;
            }
            switch (zone.controller().mode()) {
                case HeatingZoneController::Mode::Off:
                    return Status::Off;
                case HeatingZoneController::Mode::Auto:
                    break;
                case HeatingZoneController::Mode::Holiday:
                    return Status::Holiday;
            }
            return Status::Idle;
        }();
    }

    _uiModel.heating = static_cast<int>(_callingForHeatingState) == 1;
    _uiModel.internalTemperature = _temperatureSensor.read();
}