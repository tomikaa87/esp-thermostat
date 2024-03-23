#include "HeatingZone.h"

#include "Extras.h"
#include "HomeAssistant.h"
#include "PrivateConfig.h"

#include <CoreApplication.h>

#include <string>
#include <sstream>

namespace HA = HomeAssistant;

namespace
{
    std::string zoneDependentName(
        const std::string_view& name,
        const unsigned index
    )
    {
        return
            std::string{ name }
            + Extras::fromPstr(PSTR(" (Zone "))
            + std::to_string(index)
            + ")";
    }

    std::string zoneDependentUniqueId(
        const std::string_view& uniqueId,
        const unsigned index
    )
    {
        return
            std::string{ uniqueId }
            + Extras::fromPstr(PSTR("_zone_"))
            + std::to_string(index);
    }

    std::string appendIndex(
        const std::string_view& s,
        const unsigned index
    )
    {
        return std::string{ s } + '_' + std::to_string(index);
    }
}

namespace Devices::Climate
{
    auto uniqueId() { return PSTR("climate"); };
}

namespace Devices::BoostRemainingSensor
{
    auto uniqueId() { return PSTR("boost_remaining"); }
    auto stateTopic() { return PSTR("/boost/remaining"); }
}

namespace Devices::BoostActivateButton
{
    auto uniqueId() { return PSTR("boost_activate"); }
}

namespace Devices::BoostDeactivateButton
{
    auto uniqueId() { return PSTR("boost_deactivate"); }
}

namespace Devices::RemoteTemperatureNumber
{
    auto uniqueId() { return PSTR("remote_temperature"); }
}

namespace Devices::RemoteWindowSensor
{
    auto uniqueId() { return PSTR("remote_window_sensor"); }
}

namespace Topics::BoostActive
{
    auto state() { return PSTR("/boost/active"); }
    auto command() { return PSTR("/boost/active/set"); }
}

HeatingZone::HeatingZone(
    const unsigned index,
    CoreApplication& app
)
    : _index{ index }
    , _app{ app }
    , _log{ appendIndex(Extras::fromPstr(PSTR("HeatingZone")), _index) }
    , _stateSetting{ _app.settings().registerSetting<HeatingZoneController::State>() }
    , _controller{ _controllerConfig, _controllerSchedule }
    , _topicPrefix{
        HA::makeUniqueId()
            + appendIndex(Extras::fromPstr(PSTR("/zone")), _index)
    }
    , _mode{ _topicPrefix, HA::Topics::Mode::state(), HA::Topics::Mode::command(), app.mqttClient() }
    , _targetTemperature{ _topicPrefix, HA::Topics::Temperature::Active::state(), HA::Topics::Temperature::Active::command(), app.mqttClient() }
    , _lowTargetTemperature{ _topicPrefix, HA::Topics::Temperature::Low::state(), HA::Topics::Temperature::Low::command(), app.mqttClient() }
    , _highTargetTemperature{ _topicPrefix, HA::Topics::Temperature::High::state(), HA::Topics::Temperature::High::command(), app.mqttClient() }
    , _remoteTemperature{ _topicPrefix, HA::Topics::Temperature::Remote::state(), HA::Topics::Temperature::Remote::command(), app.mqttClient() }
    , _action{ _topicPrefix, HA::Topics::Action::state(), app.mqttClient() }
    , _presetMode{ _topicPrefix, HA::Topics::Preset::state(), HA::Topics::Preset::command(), app.mqttClient() }
    , _boostRemainingSeconds{
        _topicPrefix,
        Devices::BoostRemainingSensor::stateTopic(),
        app.mqttClient()
    }
    , _boostActive{
        _topicPrefix,
        Topics::BoostActive::state(),
        Topics::BoostActive::command(),
        app.mqttClient()
    }
    , _windowState{
        _topicPrefix,
        HA::Topics::RemoteWindowSensor::state(),
        HA::Topics::RemoteWindowSensor::command(),
        app.mqttClient()
    }
{
    setupMqttComponentConfigs();
    setupMqttChangeHandlers();

    if (_stateSetting.load()) {
        _log.info_P(PSTR("controller state loaded"));

        _log.debug_P(
            PSTR("mode=%u, high=%d, low=%d"),
            _stateSetting.value().mode,
            _stateSetting.value().highTargetTemperature,
            _stateSetting.value().lowTargetTemperature
        );

        _controller.loadState(_stateSetting.value());
    } else {
        _log.warning_P(PSTR("failed to load controller state, resetting to default"));

        _controller.loadState(HeatingZoneController::State{});

        if (!_stateSetting.save()) {
            _log.warning_P(PSTR("failed to save the default state"));
        }
    }

    updateMqtt();
}

void HeatingZone::task(const uint32_t systemClockDeltaMs)
{
    _controller.task(systemClockDeltaMs);

    _mqttUpdateTimer += systemClockDeltaMs;
    if (_mqttUpdateTimer >= 1000) {
        _mqttUpdateTimer = 0;
        updateMqtt();
    }

    if (_controller.stateChanged()) {
        _log.debug_P(PSTR("controller state changed, saving"));

        _stateSetting.value() = _controller.saveState();

        if (!_stateSetting.save()) {
            _log.warning_P(PSTR("failed to save controller state"));
        }
    }
}

bool HeatingZone::callingForHeating()
{
    return _controller.callingForHeating();
}

void HeatingZone::loadDefaultSettings()
{
    _log.debug_P(PSTR("%s"), __func__);

    _controllerConfig = {};
    _controllerSchedule = {};
    // _controllerState = HeatingZoneController::State{};

    // _controller.loadState(_controllerState);
}

void HeatingZone::handleFurnaceHeatingChanged(const bool heating)
{
    _controller.handleFurnaceHeatingChanged(heating);
}

void HeatingZone::setupMqttComponentConfigs()
{
    using namespace Extras;

    // Climate config
    _app.mqttClient().publish(
        [this] {
            return HA::makeConfigTopic(
                fromPstr("climate"),
                zoneDependentUniqueId(
                    fromPstr(Devices::Climate::uniqueId()),
                    _index
                )
            );
        },
        [this] {
            return HA::makeClimateConfig(
                fromPstr(PSTR("Zone")) + ' ' + std::to_string(_index),
                zoneDependentUniqueId(
                    fromPstr(Devices::Climate::uniqueId()),
                    _index
                ),
                _topicPrefix,
                [this](auto& config) {
                    HA::addDeviceConfig(
                        config,
                        _app.config().firmwareVersion.toString()
                    );
                }
            );
        }
    );

    // "Boost" activate button config
    _app.mqttClient().publish(
        [this] {
            return HA::makeConfigTopic(
                fromPstr("button"),
                zoneDependentUniqueId(
                    fromPstr(Devices::BoostActivateButton::uniqueId()),
                    _index
                )
            );
        },
        [this] {
            return HA::makeButtonConfig(
                fromPstr(PSTR("mdi:radiator")),
                zoneDependentName(fromPstr(PSTR("Activate Boost")), _index),
                zoneDependentUniqueId(
                    fromPstr(Devices::BoostActivateButton::uniqueId()),
                    _index
                ),
                _topicPrefix,
                fromPstr(Topics::BoostActive::command()),
                "1",
                [this](auto& config) {
                    HA::addDeviceConfig(
                        config,
                        _app.config().firmwareVersion.toString()
                    );
                }
            );
        }
    );

    // "Boost" deactivate button config
    _app.mqttClient().publish(
        [this] {
            return HA::makeConfigTopic(
                fromPstr("button"),
                zoneDependentUniqueId(
                    fromPstr(Devices::BoostDeactivateButton::uniqueId()),
                    _index
                )
            );
        },
        [this] {
            return HA::makeButtonConfig(
                fromPstr(PSTR("mdi:radiator-off")),
                zoneDependentName(fromPstr(PSTR("Deactivate Boost")), _index),
                zoneDependentUniqueId(
                    fromPstr(Devices::BoostDeactivateButton::uniqueId()),
                    _index
                ),
                _topicPrefix,
                fromPstr(Topics::BoostActive::command()),
                "0",
                [this](auto& config) {
                    HA::addDeviceConfig(
                        config,
                        _app.config().firmwareVersion.toString()
                    );
                }
            );
        }
    );

    // "Boost" remaining seconds sensor config
    _app.mqttClient().publish(
        [this] {
            return HA::makeConfigTopic(
                fromPstr("sensor"),
                zoneDependentUniqueId(
                    fromPstr(Devices::BoostRemainingSensor::uniqueId()),
                    _index
                )
            );
        },
        [this] {
            return HA::makeSensorConfig(
                fromPstr(PSTR("mdi:timer")),
                zoneDependentName(fromPstr("Boost Remaining"), _index),
                zoneDependentUniqueId(
                    fromPstr(Devices::BoostRemainingSensor::uniqueId()),
                    _index
                ),
                _topicPrefix,
                fromPstr(Devices::BoostRemainingSensor::stateTopic()),
                "s",
                [this](auto& config) {
                    HA::addDeviceConfig(
                        config,
                        _app.config().firmwareVersion.toString()
                    );
                }
            );
        }
    );

    // Remote temperature sensor config
    _app.mqttClient().publish(
        [this] {
            return HA::makeConfigTopic(
                fromPstr("number"),
                zoneDependentUniqueId(
                    fromPstr(Devices::RemoteTemperatureNumber::uniqueId()),
                    _index
                )
            );
        },
        [this] {
            return HA::makeNumberConfig(
                fromPstr(PSTR("mdi:thermometer")),
                zoneDependentName(fromPstr("Remote Temperature"), _index),
                zoneDependentUniqueId(
                    fromPstr(Devices::RemoteTemperatureNumber::uniqueId()),
                    _index
                ),
                _topicPrefix,
                fromPstr(HA::Topics::Temperature::Remote::command()),
                fromPstr(HA::Topics::Temperature::Remote::state()),
                "C",
                [this](auto& config) {
                    HA::addDeviceConfig(
                        config,
                        _app.config().firmwareVersion.toString()
                    );
                }
            );
        }
    );

    // Remote window state sensor config
    _app.mqttClient().publish(
        [this] {
            return HA::makeConfigTopic(
                fromPstr("switch"),
                zoneDependentUniqueId(
                    fromPstr(Devices::RemoteWindowSensor::uniqueId()),
                    _index
                )
            );
        },
        [this] {
            return HA::makeSwitchConfig(
                fromPstr(PSTR("mdi:window-closed")),
                zoneDependentName(fromPstr("Remote Window Open"), _index),
                zoneDependentUniqueId(
                    fromPstr(Devices::RemoteWindowSensor::uniqueId()),
                    _index
                ),
                _topicPrefix,
                fromPstr(HA::Topics::RemoteWindowSensor::command()),
                fromPstr(HA::Topics::RemoteWindowSensor::state()),
                [this](auto& config) {
                    HA::addDeviceConfig(
                        config,
                        _app.config().firmwareVersion.toString()
                    );
                }
            );
        }
    );
}

void HeatingZone::setupMqttChangeHandlers()
{
    _mode.setChangedHandler(
        [this](const std::string& value) {
            onModeChanged(value);
        }
    );

    _targetTemperature.setChangedHandler(
        [this](const std::string& value) {
            onTargetTemperatureChanged(std::atof(value.c_str()));
        }
    );

    _lowTargetTemperature.setChangedHandler(
        [this](const float value) {
            onLowTargetTemperatureChanged(value);
        }
    );

    _highTargetTemperature.setChangedHandler(
        [this](const float value) {
            onHighTargetTemperatureChanged(value);
        }
    );

    _remoteTemperature.setChangedHandler(
        [this](const float value) {
            onRemoteTemperatureChanged(value);
        }
    );

    _presetMode.setChangedHandler(
        [this](const std::string& value) {
            onPresetModeChanged(value);
        }
    );

    _boostActive.setChangedHandler(
        [this](const int value) {
            onBoostActiveChanged(value);
        }
    );

    _windowState.setChangedHandler(
        [this](const int value) {
            onWindowStateChanged(value > 0);
        }
    );
}

void HeatingZone::updateMqtt()
{
    switch (_controller.mode()) {
        case HeatingZoneController::Mode::Off:
            _mode = "off";
            _presetMode = "None";
            break;

        case HeatingZoneController::Mode::Auto:
            _mode = "heat";
            _presetMode = "None";
            break;

        case HeatingZoneController::Mode::Holiday:
            _mode = "heat";
            _presetMode = "away";
            break;
    }

    if (_controller.targetTemperature().has_value()) {
        _targetTemperature = std::to_string(_controller.targetTemperature().value() / 10.f);
    } else {
        _targetTemperature = "None";
    }

    _lowTargetTemperature = _controller.lowTargetTemperature() / 10.f;

    _highTargetTemperature = _controller.highTargetTemperature() / 10.f;

    if (_controller.callingForHeating()) {
        _action = "heating";
    } else if (_controller.mode() != HeatingZoneController::Mode::Off) {
        _action = "idle";
    } else {
        _action = "off";
    }

    _boostRemainingSeconds = _controller.boostRemainingSeconds();

    _boostActive = _controller.boostActive() ? 1 : 0;

    _windowState = _controller.windowOpened() ? 1 : 0;

#if defined TEST_BUILD && 0
    _log.debug_P(
        PSTR("%s: m=%s, pm=%s, tt=%s, lt=%0.1f, ht=%0.1f, a=%s, ba=%d, br=%d"),
        __func__,
        static_cast<const std::string&>(_mode).c_str(),
        static_cast<const std::string&>(_presetMode).c_str(),
        static_cast<const std::string&>(_targetTemperature).c_str(),
        static_cast<const float&>(_lowTargetTemperature),
        static_cast<const float&>(_highTargetTemperature),
        static_cast<const std::string&>(_action).c_str(),
        static_cast<const int&>(_boostActive),
        static_cast<const int&>(_boostRemainingSeconds)
    );
#endif
}

void HeatingZone::onModeChanged(const std::string& mode)
{
    if (mode == "heat") {
        _controller.setMode(HeatingZoneController::Mode::Auto);
    } else if (mode == "off") {
        _controller.setMode(HeatingZoneController::Mode::Off);
    }
}

void HeatingZone::onTargetTemperatureChanged(const float value)
{
    _log.info_P(PSTR("%s: value=%f"), __func__, value);
    _controller.overrideTargetTemperature(value * 10);
}

void HeatingZone::onLowTargetTemperatureChanged(const float value)
{
    _controller.setLowTargetTemperature(value * 10);
}

void HeatingZone::onHighTargetTemperatureChanged(const float value)
{
    _controller.setHighTargetTemperature(value * 10);
}

void HeatingZone::onRemoteTemperatureChanged(const float value)
{
    _controller.inputTemperature(value * 10);
}

void HeatingZone::onPresetModeChanged(const std::string& value)
{
    if (value == "away") {
        _controller.setMode(HeatingZoneController::Mode::Holiday);
    } else {
        _controller.setMode(HeatingZoneController::Mode::Auto);
    }
}

void HeatingZone::onBoostActiveChanged(const int value)
{
    if (value == 0) {
        _controller.stopBoost();
    } else if (value == 1) {
        _controller.startOrExtendBoost();
    }
}

void HeatingZone::onWindowStateChanged(const bool open)
{
    _controller.setWindowOpened(open);
}