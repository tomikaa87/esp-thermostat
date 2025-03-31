#include "HomeAssistant.h"

#include <ESP8266WiFi.h>

namespace HomeAssistant
{
    template <typename T>
    void quoteValue(
        std::stringstream& config,
        T&& value
    )
    {
        config << '"' << value << '"';
    }

    template <typename... Ts>
    void quoteValuesConcat(
        std::stringstream& config,
        Ts&&... values
    )
    {
        config << '"';
        (config << ... << values);
        config << '"';
    }

    template <typename... Ts>
    void quotedValueList(
        std::stringstream& config,
        Ts&&... values
    )
    {
        auto i{ sizeof...(values) };

        config << '[';

        (
            [&] {
                config << '"' << values << '"';
                if (i > 1) {
                    --i;
                    config << ',';
                }
            }()
            , ...
        );

        config << ']';
    }

    template <typename NameType, typename... ValueTypes>
    void addFieldWithValuesConcat(
        std::stringstream& config,
        NameType&& name,
        ValueTypes&&... values
    )
    {
        // Add ',' if the config is empty or just has a '{'
        if (config.tellp() > 1) {
            config << ',';
        }

        config << '"' << name << "\":";
        quoteValuesConcat(config, std::forward<ValueTypes>(values)...);
    }

    void addCommonDeviceConfig(
        std::stringstream& config,
        const std::string_view& firmwareVersion
    )
    {
        using namespace Extras;

        if (config.tellp() > 1) {
            config << ',';
        }

        config << fromPstr(PSTR(R"("device":{"sw_version":)"));
        quoteValue(config, firmwareVersion);

        config << fromPstr(PSTR(R"(,"model":"ESP Furnace Controller")"));
        config << fromPstr(PSTR(R"(,"manufacturer":"ToMikaa")"));
    }

    void addDeviceConfig(
        std::stringstream& config,
        const std::string_view& firmwareVersion
    )
    {
        using namespace Extras;

        addCommonDeviceConfig(config, firmwareVersion);

        config << fromPstr(PSTR(R"(,"name":"Furnace")"));

        config << fromPstr(PSTR(R"(,"identifiers":[")"));
        config << fromPstr(PSTR("ESP_Furnace_Controller_"));
        config << WiFi.macAddress().c_str();
        config << "\"]";

        config << "}";
    }

    void addHeatingZoneDeviceConfig(
        std::stringstream& config,
        const std::string_view& firmwareVersion,
        const std::size_t zoneIndex
    )
    {
        using namespace Extras;

        addCommonDeviceConfig(config, firmwareVersion);

        config << fromPstr(PSTR(R"(,"name":"Furnace Zone )")) << zoneIndex << '"';

        config << fromPstr(PSTR(R"(,"identifiers":[")"));
        config << fromPstr(PSTR("ESP_Furnace_Controller_"));
        config << WiFi.macAddress().c_str();
        config << "_Zone_" << zoneIndex;
        config << "\"]";

        config << "}";
    }

    std::string makeUniqueId(const std::string_view& id)
    {
        using namespace Extras;

        auto mac{ WiFi.macAddress() };
        mac.replace(":", "");

        std::string s;
        s.reserve(128);

        s += fromPstr(PSTR("furnace_controller_"));
        #ifdef TEST_BUILD
        s += "test_";
        #endif
        s += mac.c_str();

        if (!id.empty()) {
            s += '_';
            s += id;
        }

        return s;
    }

    void makeConfigTopic(
        std::stringstream& stream,
        const std::string_view& deviceType,
        const std::string_view& deviceName
    )
    {
        using namespace Extras;

        stream
            << fromPstr(PSTR("homeassistant/"))
            << deviceType
            << '/'
            << makeUniqueId(deviceName)
            << fromPstr(PSTR("/config"));
    }

    void makeClimateConfig(
        std::stringstream& stream,
        const std::string_view& name,
        const std::string_view& uniqueId,
        const std::string_view& topicPrefix,
        const ConfigAppender& appender
    )
    {
        using namespace Extras;

        stream << '{';

        stream << fromPstr(PSTR(R"("icon":"mdi:sun-thermometer")"));

        addFieldWithValuesConcat(stream, fromPstr(PSTR("name")), name);
        addFieldWithValuesConcat(stream, fromPstr("unique_id"), makeUniqueId(uniqueId));
        addFieldWithValuesConcat(stream, fromPstr("object_id"), makeUniqueId(uniqueId));

        stream << fromPstr(PSTR(R"(,"max_temp":30)"));
        stream << fromPstr(PSTR(R"(,"min_temp":10)"));
        stream << fromPstr(PSTR(R"(,"modes":["heat","off"])"));
        stream << fromPstr(PSTR(R"(,"preset_modes":["away"])"));
        stream << fromPstr(PSTR(R"(,"precision":0.1)"));
        stream << fromPstr(PSTR(R"(,"temperature_unit":"C")"));
        stream << fromPstr(PSTR(R"(,"temp_step":0.5)"));

        addFieldWithValuesConcat(stream, fromPstr(PSTR("mode_command_topic")), topicPrefix, fromPstr(Topics::Mode::command()));
        addFieldWithValuesConcat(stream, fromPstr(PSTR("mode_state_topic")), topicPrefix, fromPstr(Topics::Mode::state()));
        addFieldWithValuesConcat(stream, fromPstr(PSTR("current_temperature_topic")), topicPrefix, fromPstr(Topics::Temperature::Remote::state()));
        addFieldWithValuesConcat(stream, fromPstr(PSTR("temperature_command_topic")), topicPrefix, fromPstr(Topics::Temperature::Active::command()));
        addFieldWithValuesConcat(stream, fromPstr(PSTR("temperature_state_topic")), topicPrefix, fromPstr(Topics::Temperature::Active::state()));
        addFieldWithValuesConcat(stream, fromPstr(PSTR("temperature_high_command_topic")), topicPrefix, fromPstr(Topics::Temperature::High::command()));
        addFieldWithValuesConcat(stream, fromPstr(PSTR("temperature_high_state_topic")), topicPrefix, fromPstr(Topics::Temperature::High::state()));
        addFieldWithValuesConcat(stream, fromPstr(PSTR("temperature_low_command_topic")), topicPrefix, fromPstr(Topics::Temperature::Low::command()));
        addFieldWithValuesConcat(stream, fromPstr(PSTR("temperature_low_state_topic")), topicPrefix, fromPstr(Topics::Temperature::Low::state()));
        addFieldWithValuesConcat(stream, fromPstr(PSTR("action_topic")), topicPrefix, fromPstr(Topics::Action::state()));
        addFieldWithValuesConcat(stream, fromPstr(PSTR("preset_mode_command_topic")), topicPrefix, fromPstr(Topics::Preset::command()));
        addFieldWithValuesConcat(stream, fromPstr(PSTR("preset_mode_state_topic")), topicPrefix, fromPstr(Topics::Preset::state()));

        if (appender) {
            appender(stream);
        }

        stream << '}';
    }

    void makeSwitchConfig(
        std::stringstream& stream,
        const std::string_view& icon,
        const std::string_view& name,
        const std::string_view& uniqueId,
        const std::string_view& topicPrefix,
        const std::string_view& commandTopic,
        const std::string_view& stateTopic,
        const ConfigAppender& appender
    )
    {
        using namespace Extras;

        stream << '{';

        addFieldWithValuesConcat(stream, fromPstr(PSTR("icon")), icon);
        addFieldWithValuesConcat(stream, fromPstr(PSTR("name")), name);
        addFieldWithValuesConcat(stream, fromPstr(PSTR("unique_id")), makeUniqueId(uniqueId));
        addFieldWithValuesConcat(stream, fromPstr(PSTR("object_id")), makeUniqueId(uniqueId));
        addFieldWithValuesConcat(stream, fromPstr(PSTR("command_topic")), topicPrefix, commandTopic);
        addFieldWithValuesConcat(stream, fromPstr(PSTR("state_topic")), topicPrefix, stateTopic);
        addFieldWithValuesConcat(stream, fromPstr(PSTR("payload_off")), '0');
        addFieldWithValuesConcat(stream, fromPstr(PSTR("payload_on")), '1');

        if (appender) {
            appender(stream);
        }

        stream << '}';
    }

    void makeSensorConfig(
        std::stringstream& stream,
        const std::string_view& icon,
        const std::string_view& name,
        const std::string_view& uniqueId,
        const std::string_view& topicPrefix,
        const std::string_view& stateTopic,
        const std::string_view& unit,
        const ConfigAppender& appender
    )
    {
        using namespace Extras;

        stream << '{';

        addFieldWithValuesConcat(stream, fromPstr(PSTR("icon")), icon);
        addFieldWithValuesConcat(stream, fromPstr(PSTR("name")), name);
        addFieldWithValuesConcat(stream, fromPstr(PSTR("unique_id")), makeUniqueId(uniqueId));
        addFieldWithValuesConcat(stream, fromPstr(PSTR("object_id")), makeUniqueId(uniqueId));
        addFieldWithValuesConcat(stream, fromPstr(PSTR("state_topic")), topicPrefix, stateTopic);
        addFieldWithValuesConcat(stream, fromPstr(PSTR("unit_of_measurement")), unit);

        if (appender) {
            appender(stream);
        }

        stream << '}';
    }

    void makeButtonConfig(
        std::stringstream& stream,
        const std::string_view& icon,
        const std::string_view& name,
        const std::string_view& uniqueId,
        const std::string_view& topicPrefix,
        const std::string_view& commandTopic,
        const std::string_view& pressPayload,
        const ConfigAppender& appender
    )
    {
        using namespace Extras;

        stream << '{';

        addFieldWithValuesConcat(stream, fromPstr(PSTR("icon")), icon);
        addFieldWithValuesConcat(stream, fromPstr(PSTR("name")), name);
        addFieldWithValuesConcat(stream, fromPstr(PSTR("unique_id")), makeUniqueId(uniqueId));
        addFieldWithValuesConcat(stream, fromPstr(PSTR("object_id")), makeUniqueId(uniqueId));
        addFieldWithValuesConcat(stream, fromPstr(PSTR("command_topic")), topicPrefix, commandTopic);
        addFieldWithValuesConcat(stream, fromPstr(PSTR("payload_press")), pressPayload);

        if (appender) {
            appender(stream);
        }

        stream << '}';
    }

    void makeNumberConfig(
        std::stringstream& stream,
        const std::string_view& icon,
        const std::string_view& name,
        const std::string_view& uniqueId,
        const std::string_view& topicPrefix,
        const std::string_view& commandTopic,
        const std::string_view& stateTopic,
        const std::string_view& unit,
        const ConfigAppender& appender
    )
    {
        using namespace Extras;

        stream << '{';

        addFieldWithValuesConcat(stream, fromPstr(PSTR("icon")), icon);
        addFieldWithValuesConcat(stream, fromPstr(PSTR("name")), name);
        addFieldWithValuesConcat(stream, fromPstr(PSTR("unique_id")), makeUniqueId(uniqueId));
        addFieldWithValuesConcat(stream, fromPstr(PSTR("object_id")), makeUniqueId(uniqueId));
        addFieldWithValuesConcat(stream, fromPstr(PSTR("command_topic")), topicPrefix, commandTopic);
        addFieldWithValuesConcat(stream, fromPstr(PSTR("state_topic")), topicPrefix, stateTopic);
        addFieldWithValuesConcat(stream, fromPstr(PSTR("unit_of_measurement")), unit);
        addFieldWithValuesConcat(stream, fromPstr(PSTR("mode")), fromPstr("slider"));
        addFieldWithValuesConcat(stream, fromPstr(PSTR("min")), 10);
        addFieldWithValuesConcat(stream, fromPstr(PSTR("max")), 30);
        addFieldWithValuesConcat(stream, fromPstr(PSTR("step")), "0.1");

        if (appender) {
            appender(stream);
        }

        stream << '}';
    }
}

PGM_P HomeAssistant::Topics::Temperature::Remote::command()
{
    return PSTR("/temp/remote/set");
}

PGM_P HomeAssistant::Topics::Temperature::Remote::state()
{
    return PSTR("/temp/remote");
}

PGM_P HomeAssistant::Topics::Temperature::Active::command()
{
    return PSTR("/temp/active/set");
}

PGM_P HomeAssistant::Topics::Temperature::Active::state()
{
    return PSTR("/temp/active");
}

PGM_P HomeAssistant::Topics::Temperature::High::command()
{
    return PSTR("/temp/high/set");
}

PGM_P HomeAssistant::Topics::Temperature::High::state()
{
    return PSTR("/temp/high");
}

PGM_P HomeAssistant::Topics::Temperature::Low::command()
{
    return PSTR("/temp/low/set");
}

PGM_P HomeAssistant::Topics::Temperature::Low::state()
{
    return PSTR("/temp/low");
}

PGM_P HomeAssistant::Topics::Preset::command()
{
    return PSTR("/preset/set");
}

PGM_P HomeAssistant::Topics::Preset::state()
{
    return PSTR("/preset");
}

PGM_P HomeAssistant::Topics::Mode::command()
{
    return PSTR("/mode/set");
}

PGM_P HomeAssistant::Topics::Mode::state()
{
    return PSTR("/mode");
}

PGM_P HomeAssistant::Topics::Action::state()
{
    return PSTR("/action");
}

PGM_P HomeAssistant::Topics::RemoteWindowSensor::command()
{
    return PSTR("/window_state/set");
}

PGM_P HomeAssistant::Topics::RemoteWindowSensor::state()
{
    return PSTR("/window_state");
}