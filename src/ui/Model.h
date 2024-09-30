#pragma once

#include "Config.h"
#include "Settings.h"

#include <array>
#include <cstdint>
#include <optional>

namespace UI
{
    struct Model {
        Settings& settings;

        struct Navigation {
            int selectedZoneIndex{};
        } navigation;

        struct Clock {
            uint8_t hours{};
            uint8_t minutes{};
            uint8_t dayOfWeek{};
        } clock;

        struct Zone {
            int zoneNumber{};
            int currentTemperature{};
            std::optional<int> targetTemperature{};

            enum class Status {
                Off,
                Idle,
                Heating,
                Holiday,
                Boost,
                WindowOpen,
                WindowLockout
            } status{ Status::Idle };
        };

        std::array<Zone, Config::ZoneCount> zones;

        bool heating{};
        bool masterEnable{};
        bool energyOptimizerEnabled{};

        int16_t internalTemperature{};

        bool wifiConnected{};
        bool mqttConnected{};

        VersionNumber firmwareVersion{};
        VersionNumber baseVersion{};
    };
}