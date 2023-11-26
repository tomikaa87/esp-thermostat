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

    void addDeviceConfig(
        std::stringstream& config,
        const std::string_view& firmwareVersion
    )
    {
        using namespace Extras;

        if (config.tellp() > 0) {
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

    std::string makeConfigTopic(
        const std::string_view& deviceType,
        const std::string_view& deviceName
    )
    {
        using namespace Extras;

        std::stringstream topic;

        auto uniqueId{ WiFi.macAddress() };
        uniqueId.replace(":", "");

        topic << fromPstr(PSTR("homeassistant/"));
        topic << deviceType;
        topic << fromPstr(PSTR("/furnace_controller_"));
        topic << uniqueId.c_str();
        topic << '_' << deviceName;
        topic << fromPstr(PSTR("/config"));

        return topic.str();
    }

    std::string makeSwitchConfig(
        const std::string_view& icon,
        const std::string_view& name,
        const std::string_view& uniqueId,
        const std::string_view& commandTopic,
        const std::string_view& stateTopic,
        const std::function<void (std::stringstream&)>& appender
    )
    {
        using namespace Extras;

        std::stringstream config;

        config << '{';

        config << fromPstr(PSTR(R"("icon":)"));
        quoteValue(config, icon);

        config << fromPstr(PSTR(R"(,"name":)"));
        quoteValue(config, name);

        config << fromPstr(PSTR(R"(,"unique_id":)"));
        quoteValue(config, uniqueId);

        config << fromPstr(PSTR(R"(,"command_topic":)"));
        quoteValue(config, commandTopic);

        config << fromPstr(PSTR(R"(,"state_topic":)"));
        quoteValue(config, stateTopic);

        if (appender) {
            appender(config);
        }

        config << '}';

        return config.str();
    }
}