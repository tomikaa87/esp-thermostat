#include "HeatingZone.h"
#include "Extras.h"

#include <CoreApplication.h>

#include <string>
#include <sstream>

namespace
{
    void addTopicField(
        std::stringstream& ss,
        PGM_P name,
        const std::string_view& prefix,
        PGM_P topic
    )
    {
        ss
            << ",\""
            << Extras::pgmToStdString(name)
            << "\":\""
            << prefix
            << Extras::pgmToStdString(topic)
            << "\"";
    }
}

HeatingZone::HeatingZone(
    const unsigned index,
    CoreApplication& app
)
    : _app{ app }
    , _controller{ HeatingZoneController::Configuration{} }
    , _topicPrefix{ std::string{ "furnace_" } + std::to_string(index) }
    , _mode{ _topicPrefix, PSTR("/mode"), PSTR("/mode/set"), app.mqttClient() }
    , _targetTemperature{ _topicPrefix, PSTR("/temp/active"), PSTR("/temp/active/set"), app.mqttClient() }
    , _lowTargetTemperature{ _topicPrefix, PSTR("/temp/low"), PSTR("/temp/low/set"), app.mqttClient() }
    , _highTargetTemperature{ _topicPrefix, PSTR("/temp/high"), PSTR("/temp/high/set"), app.mqttClient() }
    , _remoteTemperature{ _topicPrefix, PSTR("/temp/remote"), PSTR("/temp/remote/set"), app.mqttClient() }
    , _action{ _topicPrefix, PSTR("/action"), app.mqttClient() }
    , _presetMode{ _topicPrefix, PSTR("/preset"), PSTR("/preset/set"), app.mqttClient() }
    , _boostRemainingSeconds{ _topicPrefix, PSTR("/boost/remaining"), app.mqttClient() }
    , _boostActive{ _topicPrefix, PSTR("/boost/active"), app.mqttClient() }
{
    setupMqttComponentConfigs();

    _app.setMqttUpdateHandler(
        [this] {
            updateMqtt();
        }
    );
}

void HeatingZone::task(const uint32_t systemClockDeltaMs)
{
    _controller.task(systemClockDeltaMs);
}

bool HeatingZone::callingForHeating() const
{
    return _controller.callingForHeating();
}

void HeatingZone::setupMqttComponentConfigs()
{
    using namespace Extras;

    // Climate config
    _app.mqttClient().publish(
        [this] {
            return makeTopic(PSTR("homeassistant/climate/"), PSTR("/config"));
        },
        [this] {
            return makeClimateConfig();
        }
    );

    // "Boost" activate button config
    _app.mqttClient().publish(
        [this] {
            return makeTopic(PSTR("homeassistant/button/"), PSTR("_boost_activate/config"));
        },
        [this] {
            return makeButtonConfig(
                PSTR("mdi:radiator-on"),
                PSTR("Activate Boost"),
                PSTR("boost_deactivate"),
                PSTR("/boost/active/set"),
                PSTR("1")
            );
        }
    );

    // "Boost" deactivate button config
    _app.mqttClient().publish(
        [this] {
            return makeTopic(PSTR("homeassistant/button/"), PSTR("_boost_deactivate/config"));
        },
        [this] {
            return makeButtonConfig(
                PSTR("mdi:radiator-off"),
                PSTR("Deactivate Boost"),
                PSTR("boost_deactivate"),
                PSTR("/boost/active/set"),
                PSTR("0")
            );
        }
    );

    // "Boost" remaining seconds sensor config
    _app.mqttClient().publish(
        [this] {
            return makeTopic(PSTR("homeassistant/sensor/"), PSTR("_boost_remaining/config"));
        },
        [this] {
            return makeSensorConfig(
                PSTR("mdi:timer"),
                PSTR("Boost Remaining"),
                PSTR("boost_remaining"),
                PSTR("/boost/remaining"),
                PSTR("s")
            );
        }
    );

    // Remote temperature sensor config
    _app.mqttClient().publish(
        [this] {
            return makeTopic(PSTR("homeassistant/number/"), PSTR("_remote_temp/config"));
        },
        [this] {
            return makeRemoteTemperatureSensorConfig(
                PSTR("Remote Temperature Input"),
                PSTR("remote_temperature"),
                PSTR("/temp/remote/set"),
                PSTR("/temp/remote")
            );
        }
    );
}

void HeatingZone::setupMqttChangeHandlers()
{
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
}

void HeatingZone::updateMqtt()
{
    if (_controller.targetTemperature().has_value()) {
        _targetTemperature = std::to_string(_controller.targetTemperature().value() / 10.f);
    } else {
        _targetTemperature = "None";
    }

    if (_controller.callingForHeating()) {
        _action = "heating";
    } else if (_controller.mode() != HeatingZoneController::Mode::Off) {
        _action = "idle";
    } else {
        _action = "off";
    }

    if (_controller.mode() == HeatingZoneController::Mode::Holiday) {
        _presetMode = "away";
    } else {
        _presetMode = "None";
    }

    _boostRemainingSeconds = _controller.boostRemainingSeconds();
    _boostActive = _controller.boostActive() ? 1 : 0;
}

void HeatingZone::onTargetTemperatureChanged(const float value)
{
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
    if (value == "away" && _controller.mode() == HeatingZoneController::Mode::Auto) {
        _controller.setMode(HeatingZoneController::Mode::Holiday);
    } else if (value == "none" && _controller.mode() == HeatingZoneController::Mode::Holiday) {
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

std::string HeatingZone::makeTopic(PGM_P prependToPrefix, PGM_P appendToPrefix) const
{
    return Extras::pgmToStdString(prependToPrefix)
        + _topicPrefix
        + Extras::pgmToStdString(appendToPrefix);
}

std::string HeatingZone::makeClimateConfig() const
{
    using namespace Extras;

        std::stringstream config;

        config << '{';

        config << pgmToStdString(PSTR(R"("icon":"mdi:sun-thermometer")"));
        config << pgmToStdString(PSTR(R"(,"name":"Furnace Zone")"));
        config << pgmToStdString(PSTR(R"(,"object_id":")")) << _topicPrefix;
        config << pgmToStdString(PSTR(R"(","unique_id":")")) << _topicPrefix;
        config << pgmToStdString(PSTR(R"(","max_temp":30)"));
        config << pgmToStdString(PSTR(R"(,"min_temp":10)"));
        config << pgmToStdString(PSTR(R"(,"modes":["heat","off"])"));
        config << pgmToStdString(PSTR(R"(,"preset_modes":["away"])"));
        config << pgmToStdString(PSTR(R"(,"precision":0.1)"));
        config << pgmToStdString(PSTR(R"(,"temperature_unit":"C")"));
        config << pgmToStdString(PSTR(R"(,"temp_step":0.5)"));

        addTopicField(config, PSTR("mode_command_topic"), _topicPrefix, PSTR("/mode/set"));
        addTopicField(config, PSTR("mode_state_topic"), _topicPrefix, PSTR("/mode"));
        // Since we don't have a local sensor, we use the remote temperature for the current value
        addTopicField(config, PSTR("current_temperature_topic"), _topicPrefix, PSTR("/temp/remote"));
        addTopicField(config, PSTR("temperature_command_topic"), _topicPrefix, PSTR("/temp/active/set"));
        addTopicField(config, PSTR("temperature_state_topic"), _topicPrefix, PSTR("/temp/active"));
        addTopicField(config, PSTR("temperature_high_command_topic"), _topicPrefix, PSTR("/temp/high/set"));
        addTopicField(config, PSTR("temperature_high_state_topic"), _topicPrefix, PSTR("/temp/high"));
        addTopicField(config, PSTR("temperature_low_command_topic"), _topicPrefix, PSTR("/temp/low/set"));
        addTopicField(config, PSTR("temperature_low_state_topic"), _topicPrefix, PSTR("/temp/low"));
        addTopicField(config, PSTR("action_topic"), _topicPrefix, PSTR("/action"));
        addTopicField(config, PSTR("preset_mode_command_topic"), _topicPrefix, PSTR("/preset/set"));
        addTopicField(config, PSTR("preset_mode_state_topic"), _topicPrefix, PSTR("/preset"));

        config << '}';

        return config.str();
}

std::string HeatingZone::makeButtonConfig(
    PGM_P icon,
    PGM_P name,
    PGM_P id,
    PGM_P commandTopic,
    PGM_P pressPayload
) const
{
    using namespace Extras;

        std::stringstream config;

        config << '{';

        config << pgmToStdString(PSTR(R"("icon":")")) << pgmToStdString(icon);
        config << pgmToStdString(PSTR(R"(","name":")")) << pgmToStdString(name);
        config << pgmToStdString(PSTR(R"(","object_id":")")) << _topicPrefix << '_' << pgmToStdString(id);
        config << pgmToStdString(PSTR(R"(","unique_id":")")) << _topicPrefix << '_' << pgmToStdString(id);
        config << pgmToStdString(PSTR(R"(","command_topic":")")) << _topicPrefix << pgmToStdString(commandTopic);
        config << pgmToStdString(PSTR(R"(","payload_press":")")) << pgmToStdString(pressPayload);
        config << '"';

        config << '}';

        return config.str();
}

std::string HeatingZone::makeSensorConfig(
    PGM_P icon,
    PGM_P name,
    PGM_P id,
    PGM_P stateTopic,
    PGM_P unit
) const
{
    using namespace Extras;

    std::stringstream config;

    config << '{';

    config << pgmToStdString(PSTR(R"("icon":")")) << pgmToStdString(icon);
    config << pgmToStdString(PSTR(R"(","name":")")) << pgmToStdString(name);
    config << pgmToStdString(PSTR(R"(","object_id":")")) << _topicPrefix << '_' << pgmToStdString(id);
    config << pgmToStdString(PSTR(R"(","unique_id":")")) << _topicPrefix << '_' << pgmToStdString(id);
    config << pgmToStdString(PSTR(R"(","state_topic":")")) << _topicPrefix << '_' << pgmToStdString(stateTopic);
    config << pgmToStdString(PSTR(R"(","unit_of_measurement":")")) << pgmToStdString(unit);
    config << '"';

    config << '}';

    return config.str();
}

std::string HeatingZone::makeRemoteTemperatureSensorConfig(
    PGM_P name,
    PGM_P id,
    PGM_P commandTopic,
    PGM_P stateTopic
) const
{
    using namespace Extras;

    std::stringstream config;

    config << '{';

    config << pgmToStdString(PSTR(R"("icon":"mdi:thermometer")"));
    config << pgmToStdString(PSTR(R"(,"name":")")) << pgmToStdString(name);
    config << pgmToStdString(PSTR(R"(","object_id":")")) << _topicPrefix << '_' << pgmToStdString(id);
    config << pgmToStdString(PSTR(R"(","unique_id":")")) << _topicPrefix << '_' << pgmToStdString(id);
    config << pgmToStdString(PSTR(R"(","command_topic":")")) << _topicPrefix << '_' << pgmToStdString(commandTopic);
    config << pgmToStdString(PSTR(R"(","state_topic":")")) << _topicPrefix << '_' << pgmToStdString(stateTopic);
    config << pgmToStdString(PSTR(R"(","unit_of_measurement":"C")"));
    config << pgmToStdString(PSTR(R"(,"mode":"slider")"));
    config << pgmToStdString(PSTR(R"(,"max":30)"));
    config << pgmToStdString(PSTR(R"(,"min":10)"));
    config << pgmToStdString(PSTR(R"(,"step":0.1)"));
    config << pgmToStdString(PSTR(R"(,"unit_of_measurement":"C")"));
    config << '"';

    config << '}';

    return config.str();
}
