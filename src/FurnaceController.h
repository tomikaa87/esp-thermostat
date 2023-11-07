#pragma once

#include "HeatingZone.h"

#include <CoreApplication.h>

#include <array>
#include <cstdint>

class ApplicationConfig;

class FurnaceController
{
public:
    explicit FurnaceController(const ApplicationConfig& appConfig);

    void task();

private:
    const ApplicationConfig& _appConfig;
    CoreApplication _app;
    std::array<HeatingZone, 1> _zones;
    uint32_t _lastTaskMillis{};

    void setupRelayOutput() const;
    void setRelayOutputActive(bool active) const;
};