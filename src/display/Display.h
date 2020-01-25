#pragma once

#include "../config.h"

#include <cstdint>

template <typename DriverImpl>
class DisplayImpl
{
public:
    using Driver = DriverImpl;

    DisplayImpl() = delete;

    static void init()
    {
        powerOff();
        Driver::init();
        clear();
        powerOn();
    }

    static void clear()
    {
        fill(0);
    }

    static void fill(const uint8_t pattern)
    {
        Driver::fill(pattern);
    }

    static void fillArea(
        const uint8_t column,
        const uint8_t startLine,
        const uint8_t width,
        const uint8_t height,
        const uint8_t pattern
    )
    {
        if (width == 0 || column >= Driver::Width)
            return;

        for (uint8_t i = 0; i < height; ++i) {
            uint8_t line = startLine + i;
            if (line >= 8)
                return;

            Driver::setLine(line);
            Driver::setColumn(column);

            for (uint8_t j = 0; j < width && column + j < Driver::Width; ++j) {
                Driver::sendData(pattern);
            }
        }
    }

    static void powerOff()
    {
        Driver::setPowerOn(false);
    }

    static void powerOn()
    {
        Driver::setPowerOn(true);
    }

    static void sendData(uint8_t data, uint8_t bitShift = 0, bool invert = false)
    {
        Driver::sendData(data, bitShift, invert);
    }

    static void sendData(const uint8_t* data, uint8_t length, uint8_t bitShift = 0, bool invert = false)
    {
        Driver::sendData(data, length, bitShift, invert);
    }

    static void setContrast(const uint8_t value)
    {
        Driver::setContrast(value);
    }

    static void setColumn(const uint8_t column)
    {
        Driver::setColumn(column);
    }

    static void setLine(const uint8_t line)
    {
        Driver::setLine(line);
    }
};

// Select default display driver implementation
#ifdef CONFIG_USE_OLED_SH1106
#include "Driver_SH1106.h"
using Display = DisplayImpl<Driver::SH1106>;
#else
#include "Driver_SSD1306.h"
using Display = DisplayImpl<Driver::SSD1306>;
#endif