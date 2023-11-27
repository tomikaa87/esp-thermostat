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

    void addDeviceConfig(
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

        config << fromPstr(PSTR(R"(,"name":"ESP Furnace Controller")"));
        config << fromPstr(PSTR(R"(,"model":"MK1")"));
        config << fromPstr(PSTR(R"(,"manufacturer":"ToMikaa")"));

        config << fromPstr(PSTR(R"(,"identifiers":)"));
        quotedValueList(
            config,
            fromPstr(PSTR("ESP_Furnace_Controller")),
            WiFi.macAddress().c_str()
        );

        config << "}";
    }

    std::string makeUniqueId(const std::string_view& id)
    {
        using namespace Extras;

        auto mac{ WiFi.macAddress() };
        mac.replace(":", "");

        std::stringstream ss;

        ss
            << fromPstr(PSTR("furnace_controller_"))
#ifdef TEST_BUILD
            << "test_"
#endif
            << mac.c_str();

        if (!id.empty()) {
            ss << '_' << id;
        }

        return ss.str();
    }

    std::string makeConfigTopic(
        const std::string_view& deviceType,
        const std::string_view& deviceName
    )
    {
        using namespace Extras;

        std::stringstream topic;

        topic
            << fromPstr(PSTR("homeassistant/"))
            << deviceType
            << '/'
            << makeUniqueId(deviceName)
            << fromPstr(PSTR("/config"));

        return topic.str();
    }

    std::string makeClimateConfig(
        const std::string_view& topicPrefix,
        const ConfigAppender& appender
    )
    {
        using namespace Extras;

        std::stringstream config;

        config << '{';

        config << fromPstr(PSTR(R"("icon":"mdi:sun-thermometer")"));
        config << fromPstr(PSTR(R"(,"name":"Furnace Zone")"));

        addFieldWithValuesConcat(config, fromPstr("unique_id"), makeUniqueId(topicPrefix));

        config << fromPstr(PSTR(R"(,"max_temp":30)"));
        config << fromPstr(PSTR(R"(,"min_temp":10)"));
        config << fromPstr(PSTR(R"(,"modes":["heat","off"])"));
        config << fromPstr(PSTR(R"(,"preset_modes":["away"])"));
        config << fromPstr(PSTR(R"(,"precision":0.1)"));
        config << fromPstr(PSTR(R"(,"temperature_unit":"C")"));
        config << fromPstr(PSTR(R"(,"temp_step":0.5)"));

        addFieldWithValuesConcat(config, fromPstr(PSTR("mode_command_topic")), topicPrefix, fromPstr(PSTR("/mode/set")));
        addFieldWithValuesConcat(config, fromPstr(PSTR("mode_state_topic")), topicPrefix, fromPstr(PSTR("/mode")));
        addFieldWithValuesConcat(config, fromPstr(PSTR("current_temperature_topic")), topicPrefix, fromPstr(PSTR("/temp/remote")));
        addFieldWithValuesConcat(config, fromPstr(PSTR("temperature_command_topic")), topicPrefix, fromPstr(PSTR("/temp/active/set")));
        addFieldWithValuesConcat(config, fromPstr(PSTR("temperature_state_topic")), topicPrefix, fromPstr(PSTR("/temp/active")));
        addFieldWithValuesConcat(config, fromPstr(PSTR("temperature_high_command_topic")), topicPrefix, fromPstr(PSTR("/temp/high/set")));
        addFieldWithValuesConcat(config, fromPstr(PSTR("temperature_high_state_topic")), topicPrefix, fromPstr(PSTR("/temp/high")));
        addFieldWithValuesConcat(config, fromPstr(PSTR("temperature_low_command_topic")), topicPrefix, fromPstr(PSTR("/temp/low/set")));
        addFieldWithValuesConcat(config, fromPstr(PSTR("temperature_low_state_topic")), topicPrefix, fromPstr(PSTR("/temp/low")));
        addFieldWithValuesConcat(config, fromPstr(PSTR("action_topic")), topicPrefix, fromPstr(PSTR("/action")));
        addFieldWithValuesConcat(config, fromPstr(PSTR("preset_mode_command_topic")), topicPrefix, fromPstr(PSTR("/preset/set")));
        addFieldWithValuesConcat(config, fromPstr(PSTR("preset_mode_state_topic")), topicPrefix, fromPstr(PSTR("/preset")));

        if (appender) {
            appender(config);
        }

        config << '}';

        return config.str();
    }

    std::string makeSwitchConfig(
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

        std::stringstream config;

        config << '{';

        addFieldWithValuesConcat(config, fromPstr(PSTR("icon")), icon);
        addFieldWithValuesConcat(config, fromPstr(PSTR("name")), name);
        addFieldWithValuesConcat(config, fromPstr(PSTR("unique_id")), makeUniqueId(uniqueId));
        addFieldWithValuesConcat(config, fromPstr(PSTR("command_topic")), topicPrefix, commandTopic);
        addFieldWithValuesConcat(config, fromPstr(PSTR("state_topic")), topicPrefix, stateTopic);
        addFieldWithValuesConcat(config, fromPstr(PSTR("payload_off")), '0');
        addFieldWithValuesConcat(config, fromPstr(PSTR("payload_on")), '1');

        if (appender) {
            appender(config);
        }

        config << '}';

        return config.str();
    }

    std::string makeSensorConfig(
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

        std::stringstream config;

        config << '{';

        addFieldWithValuesConcat(config, fromPstr(PSTR("icon")), icon);
        addFieldWithValuesConcat(config, fromPstr(PSTR("name")), name);
        addFieldWithValuesConcat(config, fromPstr(PSTR("unique_id")), makeUniqueId(uniqueId));
        addFieldWithValuesConcat(config, fromPstr(PSTR("state_topic")), topicPrefix, stateTopic);
        addFieldWithValuesConcat(config, fromPstr(PSTR("unit_of_measurement")), unit);

        if (appender) {
            appender(config);
        }

        config << '}';

        return config.str();
    }
}