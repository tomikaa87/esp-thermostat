#pragma once

#include "Extras.h"

#include <functional>
#include <sstream>
#include <string>
#include <string_view>

#include <Arduino.h>

namespace HomeAssistant
{
    namespace Topics
    {
        namespace Temperature
        {
            namespace Remote
            {
                PGM_P command();
                PGM_P state();
            }

            namespace Active
            {
                PGM_P command();
                PGM_P state();
            }

            namespace High
            {
                PGM_P command();
                PGM_P state();
            }

            namespace Low
            {
                PGM_P command();
                PGM_P state();
            }
        }

        namespace Preset
        {
            PGM_P command();
            PGM_P state();
        }

        namespace Mode
        {
            PGM_P command();
            PGM_P state();
        }

        namespace Action
        {
            PGM_P state();
        }
    }

    using ConfigAppender = std::function<void (std::stringstream&)>;

    void addDeviceConfig(
        std::stringstream& config,
        const std::string_view& firmwareVersion
    );

    /**
     * @brief Makes a unique ID: furnace_controller_<WiFi MAC>_<ID>
     *
     * @param id
     * @return std::string
     */
    std::string makeUniqueId(const std::string_view& id = {});

    /**
     * @brief Makes a config topic: /homeassistant/<device type>/<unique ID with device name>/config
     *
     * @param deviceType
     * @param deviceName
     * @return std::string
     */
    std::string makeConfigTopic(
        const std::string_view& deviceType,
        const std::string_view& deviceName
    );

    std::string makeClimateConfig(
        const std::string_view& name,
        const std::string_view& uniqueId,
        const std::string_view& topicPrefix,
        const ConfigAppender& appender = {}
    );

    std::string makeSwitchConfig(
        const std::string_view& icon,
        const std::string_view& name,
        const std::string_view& uniqueId,
        const std::string_view& topicPrefix,
        const std::string_view& commandTopic,
        const std::string_view& stateTopic,
        const ConfigAppender& appender = {}
    );

    std::string makeSensorConfig(
        const std::string_view& icon,
        const std::string_view& name,
        const std::string_view& uniqueId,
        const std::string_view& topicPrefix,
        const std::string_view& stateTopic,
        const std::string_view& unit,
        const ConfigAppender& appender = {}
    );

    std::string makeButtonConfig(
        const std::string_view& icon,
        const std::string_view& name,
        const std::string_view& uniqueId,
        const std::string_view& topicPrefix,
        const std::string_view& commandTopic,
        const std::string_view& pressPayload,
        const ConfigAppender& appender = {}
    );

    std::string makeNumberConfig(
        const std::string_view& icon,
        const std::string_view& name,
        const std::string_view& uniqueId,
        const std::string_view& topicPrefix,
        const std::string_view& commandTopic,
        const std::string_view& stateTopic,
        const std::string_view& unit,
        const ConfigAppender& appender = {}
    );
}