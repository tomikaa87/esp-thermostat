#pragma once

#include "PrivateConfig.h"

#include <array>
#include <cstdint>

namespace UI
{
    struct Model {
        struct Clock {
            uint8_t hours{};
            uint8_t minutes{};
            uint8_t dayOfWeek{};
        } clock;

        struct Zone {
            int zoneNumber{ 1 };
            int currentTemperature{ 220 };
            int targetTemperature{ 230 };

            enum class Status {
                Idle,
                Heating,
                Holiday,
                Boost,
                WindowOpen
            } status{ Status::Idle };
        };

        std::array<Zone, Config::ZoneCount> zones;

        bool heating{ true };
        bool energySaverEnabled{ true };

        int16_t internalTemperature{ 258 };
    };
}