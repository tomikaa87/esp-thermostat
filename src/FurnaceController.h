#pragma once

#include "HeatingZone.h"
#include "Settings.h"

#include <CoreApplication.h>
#include <Logger.h>

#include <array>
#include <cstdint>

class ApplicationConfig;

class FurnaceController
{
public:
    static constexpr auto ZoneCount = 5;

    explicit FurnaceController(const ApplicationConfig& appConfig);

    void task();

private:
    const ApplicationConfig& _appConfig;
    CoreApplication _app;
    Logger _log{ "FurnaceController" };
    std::array<HeatingZone, ZoneCount> _zones;
    uint32_t _lastTaskMillis{};
    bool _relayOutputActive{ false };

    void setupRelayOutput() const;
    void setRelayOutputActive(bool active);
};