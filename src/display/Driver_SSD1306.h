#pragma once

#include <cstdint>

namespace Driver
{

class SSD1306
{
public:
    enum class MemoryMode : uint8_t
    {
        HorizontalAddressing,
        VerticalAddressing,
        PageAddressing
    };

    static constexpr uint8_t Height = 64;
    static constexpr uint8_t Width = 128;
    static constexpr uint8_t Lines = 8;

    static void init();

    static void fill(uint8_t pattern);

    static void sendCommand(uint8_t code);
    static void sendCommand(uint8_t code, uint8_t arg);

    static void sendData(uint8_t data, uint8_t bitShift = 0, bool invert = false);
    static void sendData(const uint8_t* data, uint8_t length, uint8_t bitShift = 0, bool invert = false);

    static void setPowerOn(bool on);
    static void setComPadsAltHwConfig(uint8_t value);
    static void setComScanInverted(bool inverted);
    static void setContrast(uint8_t value);
    static void setDcDcConvOn(bool on);
    static void setDischargePrechargePeriod(uint8_t precharge, uint8_t discharge);
    static void setDisplayAllOn(bool on);
    static void setDisplayClockDivRatio(uint8_t div, uint8_t ratio);
    static void setDisplayStartLine(uint8_t value);
    static void setDisplayOffset(uint8_t value);
    static void setInvertedDisplay(bool inverted);
    static void setMemoryMode(MemoryMode mode);
    static void setMultiplexRatio(uint8_t value);
    static void setSegmentRemap(bool reverse);
    static void setVcomDeselectLevel(uint8_t value);

    static void setColumn(uint8_t column);
    static void setLine(uint8_t line);
};

}