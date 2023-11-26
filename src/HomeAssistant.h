#pragma once

#include "Extras.h"

#include <functional>
#include <sstream>
#include <string>
#include <string_view>

#include <Arduino.h>

namespace HomeAssistant
{
    void addDeviceConfig(
        std::stringstream& config,
        const std::string_view& firmwareVersion
    );

    std::string makeConfigTopic(
        const std::string_view& deviceType,
        const std::string_view& deviceName
    );

    std::string makeSwitchConfig(
        const std::string_view& icon,
        const std::string_view& name,
        const std::string_view& uniqueId,
        const std::string_view& commandTopic,
        const std::string_view& stateTopic,
        const std::function<void (std::stringstream&)>& appender = {}
    );
}