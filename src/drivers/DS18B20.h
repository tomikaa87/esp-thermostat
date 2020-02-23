#pragma once

#include "BusConfig.h"
#include "OneWire.h"

#include <cstdint>

namespace Drivers
{

class DS18B20
{
public:
    static constexpr auto ResolutionBits = 12;

    DS18B20() = delete;

    static void update();
    static int16_t lastReading();

private:
    using Bus = Peripherals::Bus::MainTemperatureOneWire;

    static int16_t _lastReading;

    static void startConversion();
    static int16_t readSensor();
};

}