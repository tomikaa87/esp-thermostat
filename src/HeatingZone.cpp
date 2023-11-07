#include "HeatingZone.h"
#include "Extras.h"

#include <CoreApplication.h>

#include <string>
#include <sstream>

namespace
{
    std::string makeTopicPrefix(const unsigned index)
    {
        return std::string{ "furnace_" } + std::to_string(index);
    }

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
    , _controller{
        HeatingZoneController::Configuration{}
    }
    , _topicPrefix{ makeTopicPrefix(index) }
    , _index{ index }
    , _mode{ _topicPrefix, PSTR("/mode"), PSTR("/mode/set"), app.mqttClient() }
    , _targetTemperature{_topicPrefix, PSTR("/temp/active"), PSTR("/temp/active/set"), app.mqttClient() }
    , _currentTemperature{_topicPrefix, PSTR("/temp/current"), app.mqttClient() }
    , _lowTargetTemperature{ _topicPrefix, PSTR("/temp/low"), PSTR("/temp/low/set"), app.mqttClient() }
    , _highTargetTemperature{ _topicPrefix, PSTR("/temp/high"), PSTR("/temp/high/set"), app.mqttClient() }
    , _action{ _topicPrefix, PSTR("/action"), app.mqttClient() }
    , _presetMode{ _topicPrefix, PSTR("/preset"), PSTR("/preset/set"), app.mqttClient() }
{
    setupMqtt();
}

void HeatingZone::setupMqtt()
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
        config << pgmToStdString(PSTR(R"(,"object_id":")")) << _topicPrefix << '"' ;
        config << pgmToStdString(PSTR(R"(,"unique_id":")")) << _topicPrefix << '"';
        config << pgmToStdString(PSTR(R"(,"max_temp":30)"));
        config << pgmToStdString(PSTR(R"(,"min_temp":10)"));
        config << pgmToStdString(PSTR(R"(,"modes":["heat","off"])"));
        config << pgmToStdString(PSTR(R"(,"preset_modes":["away"])"));
        config << pgmToStdString(PSTR(R"(,"precision":0.1)"));
        config << pgmToStdString(PSTR(R"(,"temperature_unit":"C")"));
        config << pgmToStdString(PSTR(R"(,"temp_step":0.5)"));

        addTopicField(config, PSTR("mode_command_topic"), _topicPrefix, PSTR("/mode/set"));
        addTopicField(config, PSTR("mode_state_topic"), _topicPrefix, PSTR("/mode"));
        addTopicField(config, PSTR("current_temperature_topic"), _topicPrefix, PSTR("/temp/current"));
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

        config << '}';

        return config.str();
}
