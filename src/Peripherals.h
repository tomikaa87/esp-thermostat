#pragma once

#include "drivers/DS18B20.h"
#include "drivers/EERAM.h"

#include <Arduino.h>

namespace Peripherals
{
    namespace Sensors
    {
        using MainTemperature = Drivers::DS18B20;
    }

    namespace Clock
    {

    }

    namespace Relay
    {

    }

    namespace Storage
    {
        using EERAM = Drivers::EERAM;
    }
}
