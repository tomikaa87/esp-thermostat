#pragma once

#include "drivers/OneWire.h"

#include <Arduino.h>

namespace Peripherals
{
    namespace Bus
    {
        using MainTemperatureOneWire = Drivers::OneWire<D7>;
    }
}