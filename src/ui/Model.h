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
            int currentTemperature{};
            int targetTemperature{};

            enum class Status {
                Idle,
                Heating,
                Holiday,
                Boost,
                WindowOpen
            } status{ Status::Idle };
        };

        std::array<Zone, Config::ZoneCount> zones;

        bool heating{};
        bool energySaverEnabled{};
    };
}